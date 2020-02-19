//----------------------------------------------------------------------------------------------------------------------
//	CGPUTextureManager.cpp			©2019 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CGPUTextureManager.h"

#include "CLock.h"
#include "CLogServices.h"
#include "CWorkItemQueue.h"
#include "TOptional.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SGPUTextureManagerInfo

class CGPUTextureReferenceInternals;

struct SGPUTextureManagerInfo {
	// Lifecycle methods
	SGPUTextureManagerInfo(CGPU& gpu) : mGPU(gpu) {}

	// Properties
	CGPU&										mGPU;
	CWorkItemQueue								mWorkItemQueue;
	TPtrArray<CGPUTextureReferenceInternals*>	mGPUTextureReferenceInternals;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUTextureReferenceInternals

class CGPUTextureReferenceInternals : public TReferenceCountable<CGPUTextureReferenceInternals> {
	public:
												CGPUTextureReferenceInternals(const OR<const CString>& reference,
														const OV<EGPUTextureFormat>& gpuTextureFormat,
														EGPUTextureReferenceOptions gpuTextureReferenceOptions,
														SGPUTextureManagerInfo& gpuTextureManagerInfo) :
													TReferenceCountable(),
															mReference(reference.hasReference() ?
																	*reference : CString::mEmpty),
															mGPUTextureFormat(gpuTextureFormat),
															mGPUTextureReferenceOptions(gpuTextureReferenceOptions),
															mGPUTextureManagerInfo(gpuTextureManagerInfo),
															mWorkItem(nil), mFinishLoadingTriggered(false),
															mGPUTexture(nil)
													{
														// Note reference
														mGPUTextureManagerInfo.mGPUTextureReferenceInternals += this;
													}
												~CGPUTextureReferenceInternals()
													{
														// Remove from references
														mGPUTextureManagerInfo.mGPUTextureReferenceInternals -= this;

														// Cleanup
														DisposeOf(mGPUTexture);
													}

				bool							getIsLoaded() const
													{ return mGPUTextureInfo.isValid(); }
				void							loadOrQueueForLoading()
													{
														// Check options
														if (mGPUTextureReferenceOptions &
																kGPUTextureReferenceOptionsLoadImmediately)
															// Finish loading
															finishLoading();
														else
															// Setup workItem
															mGPUTextureManagerInfo.mWorkItemQueue.add(load, this);
													}
				bool							isLoadingContinuing()
													{
														return mFinishLoadingTriggered ||
																((mWorkItem != nil) && !mWorkItem->isCancelled());
													}
				void							finishLoading()
													{
														// Go all the way
														mFinishLoadingTriggered = true;

														// Setup to load
														mLoadLock.lock();
														if (!mGPUTextureInfo.isValid()) {
															// Need to finish
															if (mGPUTexture == nil)
																// Do full load
																load();

															// Register texture
															registerTexture();
														}
														mLoadLock.unlock();
													}

												// Instance methods for subclasses to call
				void							loadComplete(CGPUTexture* gpuTexture)
													{
														// Store
														mGPUTexture = gpuTexture;
														registerTexture();
													}

												// Private methods
				void							registerTexture()
													{
														// Register texture
														mGPUTextureInfo =
																mGPUTextureManagerInfo.mGPU.registerTexture(
																		*mGPUTexture);
													}

												// Instance methods for subclasses to implement or override
		virtual	void							load() = 0;
		virtual	void							unload()
													{
														// Check for workItem
														if (mWorkItem != nil) {
															// Cancel
															mGPUTextureManagerInfo.mWorkItemQueue.cancel(*mWorkItem);
															mWorkItem = nil;
														}

														// Check for render materia texture
														if (mGPUTexture != nil) {
															// Cleanup
															DisposeOf(mGPUTexture);

															// Unregiste with GPU Render Engine
															mGPUTextureManagerInfo.
																	mGPU.unregisterTexture(
																			mGPUTextureInfo);
														}
													}

												// Class methods
		static	void							load(void* userData, CWorkItem& workItem)
													{
														// Get info
														CGPUTextureReferenceInternals&	internals =
																								*((CGPUTextureReferenceInternals*)
																										userData);

														// Store
														internals.mWorkItem = &workItem;

														// Setup to load
														internals.mLoadLock.lock();
														if (!internals.getIsLoaded())
															// Load
															internals.load();
														internals.mLoadLock.unlock();

														// Finished
														internals.mWorkItem = nil;
													}

		CString							mReference;
		OV<EGPUTextureFormat>			mGPUTextureFormat;
		EGPUTextureReferenceOptions		mGPUTextureReferenceOptions;
		SGPUTextureManagerInfo&			mGPUTextureManagerInfo;

		CLock							mLoadLock;
		CWorkItem*						mWorkItem;
		bool							mFinishLoadingTriggered;
		CGPUTexture*					mGPUTexture;
		SGPUTextureInfo					mGPUTextureInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBitmapGPUTextureReferenceInternals

class CBitmapGPUTextureReferenceInternals : public CGPUTextureReferenceInternals {
	public:
									CBitmapGPUTextureReferenceInternals(const CBitmap& bitmap,
											const OR<const CString>& reference,
											const OV<EGPUTextureFormat>& gpuTextureFormat,
											EGPUTextureReferenceOptions gpuTextureReferenceOptions,
											SGPUTextureManagerInfo& gpuTextureManagerInfo) :
										CGPUTextureReferenceInternals(reference, gpuTextureFormat,
												gpuTextureReferenceOptions, gpuTextureManagerInfo),
										mBitmap(new CBitmap(bitmap)), mLoadingBitmap(nil)
										{}
									CBitmapGPUTextureReferenceInternals(const OR<const CString>& reference,
											const OV<EGPUTextureFormat>& gpuTextureFormat,
											EGPUTextureReferenceOptions gpuTextureReferenceOptions,
											SGPUTextureManagerInfo& gpuTextureManagerInfo) :
										CGPUTextureReferenceInternals(reference, gpuTextureFormat,
												gpuTextureReferenceOptions, gpuTextureManagerInfo),
										mBitmap(nil), mLoadingBitmap(nil)
										{}

									~CBitmapGPUTextureReferenceInternals()
										{
											// Cleanup
											DisposeOf(mBitmap);
										}

									// CGPUTextureReferenceInternals methods
				void				load()
										{
											// Setup
											CBitmap*	bitmapUse = nil;
											CBitmap*	convertedBitmap = nil;
											SBitmapSize	bitmapSize;

											// Is loading continuing
											if (isLoadingContinuing()) {
												// Setup
												bitmapUse = (mBitmap != nil) ? mBitmap : mLoadingBitmap;
												EBitmapFormat	bitmapUseFormat = bitmapUse->getFormat();

												if (!mGPUTextureFormat.hasValue())
													// Set value
													mGPUTextureFormat =
															resolvedGPUTextureFormat(bitmapUse->getFormat());

												EBitmapFormat	bitmapUseResolvedBitmapFormat =
																		resolvedBitmapFormat(*mGPUTextureFormat,
																				bitmapUseFormat);
												bitmapSize = bitmapUse->getSize();

												// Check bitmap formats
												if (bitmapUseResolvedBitmapFormat != bitmapUseFormat) {
													// Bitmap formats don't match
													convertedBitmap =
															new CBitmap(*bitmapUse, bitmapUseResolvedBitmapFormat);
													bitmapUse = convertedBitmap;

													// Cleanup
													DisposeOf(mLoadingBitmap);
												}
											}

											if (isLoadingContinuing())
												// Create render material texture
												loadComplete(
														new CGPUTexture(bitmapUse->getPixelData(),
																mGPUTextureFormat.getValue(),
																SGPUTextureSize(bitmapSize.mWidth,
																bitmapSize.mHeight)));

											// Cleanup
											DisposeOf(mLoadingBitmap);
											DisposeOf(convertedBitmap);
										}

									// Class methods
		static	EGPUTextureFormat	resolvedGPUTextureFormat(EBitmapFormat bitmapFormat)
										{
											switch (bitmapFormat) {
												// 16 bit formats
												case kBitmapFormatRGB565:	return kGPUTextureFormatRGB565;
												case kBitmapFormatRGBA4444:	return kGPUTextureFormatRGBA4444;
												case kBitmapFormatRGBA5551:	return kGPUTextureFormatRGBA5551;

												// 24 bit formats
												case kBitmapFormatRGB888:	return kGPUTextureFormatRGBA8888;

												// 32 bit formats
												case kBitmapFormatRGBA8888:	return kGPUTextureFormatRGBA8888;
												case kBitmapFormatARGB8888:	return kGPUTextureFormatRGBA8888;
											}
										}
		static	EBitmapFormat		resolvedBitmapFormat(EGPUTextureFormat gpuTextureFormat,
											EBitmapFormat fallbackBitmapFormat)
										{
											// What is the render material texture format
											switch (gpuTextureFormat) {
												// Convertable formats
												case kGPUTextureFormatRGB565:	return kBitmapFormatRGB565;
												case kGPUTextureFormatRGBA4444:	return kBitmapFormatRGBA4444;
												case kGPUTextureFormatRGBA5551:	return kBitmapFormatRGBA5551;

												case kGPUTextureFormatRGBA8888:	return kBitmapFormatRGBA8888;

												// Everything else does not correspond to a bitmap format
												default:						return fallbackBitmapFormat;
											}
										}

		CBitmap*	mBitmap;

		CBitmap*	mLoadingBitmap;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDataGPUTextureReferenceInternals

class CDataGPUTextureReferenceInternals : public CGPUTextureReferenceInternals {
	public:
				CDataGPUTextureReferenceInternals(const CByteParceller& byteParceller,
						const OR<const CString>& reference, const OV<EGPUTextureFormat>& gpuTextureFormat,
						SGPUTextureSize gpuTextureSize, EGPUTextureReferenceOptions gpuTextureReferenceOptions,
						SGPUTextureManagerInfo& gpuTextureManagerInfo) :
					CGPUTextureReferenceInternals(reference, gpuTextureFormat, gpuTextureReferenceOptions,
							gpuTextureManagerInfo),
					mByteParceller(byteParceller), mGPUTextureSize(gpuTextureSize)
					{}

				// CGPUTextureReferenceInternals methods
		void	load()
					{
						// Setup
						CData	data;

						// Is loading continuing
						if (isLoadingContinuing()) {
							// Read data
							UError	error;
							data = mByteParceller.readData(error);
							mByteParceller.reset();
							LogIfErrorAndReturn(error, "reading data from data provider");
						}

						// Is loading continuing
						if (isLoadingContinuing())
							// Create render material texture
							loadComplete(new CGPUTexture(data, mGPUTextureFormat.getValue(), mGPUTextureSize));
					}

		const	CByteParceller	mByteParceller;
		const	SGPUTextureSize	mGPUTextureSize;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBitmapProcGPUTextureReferenceInternals

class CBitmapProcGPUTextureReferenceInternals : public CBitmapGPUTextureReferenceInternals {
	public:
				CBitmapProcGPUTextureReferenceInternals(const CByteParceller& byteParceller,
						CGPUTextureManagerBitmapProc bitmapProc, const OR<const CString>& reference,
						const OV<EGPUTextureFormat>& gpuTextureFormat,
						EGPUTextureReferenceOptions gpuTextureReferenceOptions,
						SGPUTextureManagerInfo& gpuTextureManagerInfo) :
					CBitmapGPUTextureReferenceInternals(reference, gpuTextureFormat, gpuTextureReferenceOptions,
							gpuTextureManagerInfo),
						mByteParceller(byteParceller), mBitmapProc(bitmapProc)
					{}

				// CGPUTextureReferenceInternals methods
		void	load()
					{
						// Is loading continuing
						if (isLoadingContinuing())
							// Create bitmap
							mLoadingBitmap = new CBitmap(mBitmapProc(mByteParceller));

						// Do super
						CBitmapGPUTextureReferenceInternals::load();
					}

		const	CByteParceller					mByteParceller;
				CGPUTextureManagerBitmapProc	mBitmapProc;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUTextureReference

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureReference::CGPUTextureReference(CGPUTextureReferenceInternals* internals)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = internals;
}

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureReference::CGPUTextureReference(const CGPUTextureReference& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureReference::~CGPUTextureReference()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

////----------------------------------------------------------------------------------------------------------------------
//bool CGPUTextureReference::getIsLoaded() const
////----------------------------------------------------------------------------------------------------------------------
//{
//	return mInternals->getIsLoaded();
//}

//----------------------------------------------------------------------------------------------------------------------
void CGPUTextureReference::load() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if loaded
	if (!mInternals->getIsLoaded())
		// Setup for load
		mInternals->loadOrQueueForLoading();
}

//----------------------------------------------------------------------------------------------------------------------
void CGPUTextureReference::finishLoading() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if loaded
	if (!mInternals->getIsLoaded())
		// Setup for load
		mInternals->finishLoading();
}

//----------------------------------------------------------------------------------------------------------------------
const SGPUTextureInfo& CGPUTextureReference::getGPUTextureInfo() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mGPUTextureInfo;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUTextureManagerInternals

class CGPUTextureManagerInternals : public TReferenceCountable<CGPUTextureManagerInternals> {
	public:
		CGPUTextureManagerInternals(CGPU& gpu) : TReferenceCountable(), mGPUTextureManagerInfo(gpu) {}

		SGPUTextureManagerInfo	mGPUTextureManagerInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUTextureManager

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureManager::CGPUTextureManager(CGPU& gpu)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CGPUTextureManagerInternals(gpu);
}

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureManager::CGPUTextureManager(const CGPUTextureManager& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureManager::~CGPUTextureManager()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureReference CGPUTextureManager::gpuTextureReference(const CBitmap& bitmap, OR<const CString> reference,
		EGPUTextureReferenceOptions gpuTextureReferenceOptions)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if we have a reference
	if (reference.hasReference())
		// Look for file in already loaded render material images
		for (TIteratorS<CGPUTextureReferenceInternals*> iterator =
					mInternals->mGPUTextureManagerInfo.mGPUTextureReferenceInternals.getIterator();
				iterator.hasValue(); iterator.advance()) {
			// Check this one
			if (!iterator.getValue()->mReference.isEmpty() && (iterator.getValue()->mReference == *reference))
				// Found existing
				return CGPUTextureReference(iterator.getValue());
		}

	// Create new
	CGPUTextureReferenceInternals*	renderMaterialImageReferenceInternals =
											new CBitmapGPUTextureReferenceInternals(bitmap, reference,
													OV<EGPUTextureFormat>(), gpuTextureReferenceOptions,
													mInternals->mGPUTextureManagerInfo);
	CGPUTextureReference			gpuTextureReference(renderMaterialImageReferenceInternals);

	// Load
	renderMaterialImageReferenceInternals->loadOrQueueForLoading();

	return gpuTextureReference;
}

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureReference CGPUTextureManager::gpuTextureReference(const CBitmap& bitmap, EGPUTextureFormat gpuTextureFormat,
		OR<const CString> reference, EGPUTextureReferenceOptions gpuTextureReferenceOptions)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if we have a reference
	if (reference.hasReference())
		// Look for file in already loaded render material images
		for (TIteratorS<CGPUTextureReferenceInternals*> iterator =
					mInternals->mGPUTextureManagerInfo.mGPUTextureReferenceInternals.getIterator();
				iterator.hasValue(); iterator.advance()) {
			// Check this one
			if (!iterator.getValue()->mReference.isEmpty() && (iterator.getValue()->mReference == *reference))
				// Found existing
				return CGPUTextureReference(iterator.getValue());
		}

	// Create new
	CGPUTextureReferenceInternals*	renderMaterialImageReferenceInternals =
											new CBitmapGPUTextureReferenceInternals(bitmap, reference,
													OV<EGPUTextureFormat>(gpuTextureFormat),
													gpuTextureReferenceOptions, mInternals->mGPUTextureManagerInfo);
	CGPUTextureReference			gpuTextureReference(renderMaterialImageReferenceInternals);

	// Load
	renderMaterialImageReferenceInternals->loadOrQueueForLoading();

	return gpuTextureReference;
}

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureReference CGPUTextureManager::gpuTextureReference(const CByteParceller& byteParceller,
		CGPUTextureManagerBitmapProc bitmapProc, OR<const CString> reference,
		EGPUTextureReferenceOptions gpuTextureReferenceOptions)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if we have a reference
	if (reference.hasReference())
		// Look for file in already loaded render material images
		for (TIteratorS<CGPUTextureReferenceInternals*> iterator =
					mInternals->mGPUTextureManagerInfo.mGPUTextureReferenceInternals.getIterator();
				iterator.hasValue(); iterator.advance()) {
			// Check this one
			if (!iterator.getValue()->mReference.isEmpty() && (iterator.getValue()->mReference == *reference))
				// Found existing
				return CGPUTextureReference(iterator.getValue());
		}

	// Create new
	CGPUTextureReferenceInternals*	renderMaterialImageReferenceInternals =
											new CBitmapProcGPUTextureReferenceInternals(byteParceller, bitmapProc,
													reference, OV<EGPUTextureFormat>(), gpuTextureReferenceOptions,
													mInternals->mGPUTextureManagerInfo);
	CGPUTextureReference			gpuTextureReference(renderMaterialImageReferenceInternals);

	// Load
	renderMaterialImageReferenceInternals->loadOrQueueForLoading();

	return gpuTextureReference;
}

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureReference CGPUTextureManager::gpuTextureReference(const CByteParceller& byteParceller,
		CGPUTextureManagerBitmapProc bitmapProc, EGPUTextureFormat gpuTextureFormat, OR<const CString> reference,
		EGPUTextureReferenceOptions gpuTextureReferenceOptions)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if we have a reference
	if (reference.hasReference())
		// Look for file in already loaded render material images
		for (TIteratorS<CGPUTextureReferenceInternals*> iterator =
					mInternals->mGPUTextureManagerInfo.mGPUTextureReferenceInternals.getIterator();
				iterator.hasValue(); iterator.advance()) {
			// Check this one
			if (!iterator.getValue()->mReference.isEmpty() && (iterator.getValue()->mReference == *reference))
				// Found existing
				return CGPUTextureReference(iterator.getValue());
		}

	// Create new
	CGPUTextureReferenceInternals*	renderMaterialImageReferenceInternals =
											new CBitmapProcGPUTextureReferenceInternals(byteParceller, bitmapProc,
													reference, OV<EGPUTextureFormat>(gpuTextureFormat),
													gpuTextureReferenceOptions, mInternals->mGPUTextureManagerInfo);
	CGPUTextureReference			gpuTextureReference(renderMaterialImageReferenceInternals);

	// Load
	renderMaterialImageReferenceInternals->loadOrQueueForLoading();

	return gpuTextureReference;
}

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureReference CGPUTextureManager::gpuTextureReference(const CByteParceller& textureByteParceller,
		EGPUTextureFormat gpuTextureFormat, SGPUTextureSize gpuTextureSize, OR<const CString> reference,
		EGPUTextureReferenceOptions gpuTextureReferenceOptions)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if we have a reference
	if (reference.hasReference())
		// Look for file in already loaded render material images
		for (TIteratorS<CGPUTextureReferenceInternals*> iterator =
					mInternals->mGPUTextureManagerInfo.mGPUTextureReferenceInternals.getIterator();
				iterator.hasValue(); iterator.advance()) {
			// Check this one
			if (!iterator.getValue()->mReference.isEmpty() && (iterator.getValue()->mReference == *reference))
				// Found existing
				return CGPUTextureReference(iterator.getValue());
		}

	// Create new
	CGPUTextureReferenceInternals*	renderMaterialImageReferenceInternals =
											new CDataGPUTextureReferenceInternals(textureByteParceller, reference,
													gpuTextureFormat, gpuTextureSize,gpuTextureReferenceOptions,
													mInternals->mGPUTextureManagerInfo);
	CGPUTextureReference			gpuTextureReference(renderMaterialImageReferenceInternals);

	// Load
	renderMaterialImageReferenceInternals->loadOrQueueForLoading();

	return gpuTextureReference;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPUTextureManager::loadAll()
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all
	for (TIteratorS<CGPUTextureReferenceInternals*> iterator =
				mInternals->mGPUTextureManagerInfo.mGPUTextureReferenceInternals.getIterator();
			iterator.hasValue(); iterator.advance())
		// Start loading
		iterator.getValue()->loadOrQueueForLoading();
}

//----------------------------------------------------------------------------------------------------------------------
void CGPUTextureManager::pauseLoading()
//----------------------------------------------------------------------------------------------------------------------
{
	// Pause
	mInternals->mGPUTextureManagerInfo.mWorkItemQueue.pause();
}

//----------------------------------------------------------------------------------------------------------------------
void CGPUTextureManager::resumeLoading()
//----------------------------------------------------------------------------------------------------------------------
{
	// Resume
	mInternals->mGPUTextureManagerInfo.mWorkItemQueue.resume();
}

//----------------------------------------------------------------------------------------------------------------------
void CGPUTextureManager::unloadAll()
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all
	for (TIteratorS<CGPUTextureReferenceInternals*> iterator =
				mInternals->mGPUTextureManagerInfo.mGPUTextureReferenceInternals.getIterator();
			iterator.hasValue(); iterator.advance())
		// Unload
		iterator.getValue()->unload();
}
