//----------------------------------------------------------------------------------------------------------------------
//	CGPUTextureManager.cpp			©2019 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CGPUTextureManager.h"

#include "CLock.h"
#include "CLogServices.h"
#include "CWorkItemQueue.h"
#include "TOptional.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SGPUTextureDataInfo

struct SGPUTextureDataInfo {
	// Lifecycle methods
	SGPUTextureDataInfo(const CData& data, EGPUTextureDataFormat gpuTextureDataFormat, S2DSizeU16 size) :
		mData(data), mGPUTextureDataFormat(gpuTextureDataFormat), mSize(size)
	 {}

	// Properties
	const	CData					mData;
			EGPUTextureDataFormat	mGPUTextureDataFormat;
			S2DSizeU16				mSize;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SGPUTextureManagerInfo

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
						// Lifecycle methods
						CGPUTextureReferenceInternals(const OR<const CString>& reference,
								const OV<EGPUTextureDataFormat>& gpuTextureDataFormat,
								EGPUTextureReferenceOptions gpuTextureReferenceOptions,
								SGPUTextureManagerInfo& gpuTextureManagerInfo) :
							TReferenceCountable(),
									mReference(reference.hasReference() ? *reference : CString::mEmpty),
									mGPUTextureDataFormat(gpuTextureDataFormat),
									mGPUTextureReferenceOptions(gpuTextureReferenceOptions),
									mGPUTextureManagerInfo(gpuTextureManagerInfo), mWorkItem(nil),
									mFinishLoadingTriggered(false), mGPUTextureDataInfo(nil)
							{
								// Note reference
								mGPUTextureManagerInfo.mGPUTextureReferenceInternals += this;
							}
						~CGPUTextureReferenceInternals()
							{
								// Remove from references
								mGPUTextureManagerInfo.mGPUTextureReferenceInternals -= this;

								// Cleanup
								unload();
							}

						// Instance methods
				bool	getIsLoaded() const
							{ return mGPUTextureReference.hasGPUTexture(); }
				void	loadOrQueueForLoading()
							{
								// Check options
								if (mGPUTextureReferenceOptions & kGPUTextureReferenceOptionsLoadImmediately)
									// Finish loading
									finishLoading();
								else
									// Setup workItem
									mGPUTextureManagerInfo.mWorkItemQueue.add(load, this);
							}
				bool	isLoadingContinuing()
							{
								return mFinishLoadingTriggered || ((mWorkItem != nil) && !mWorkItem->isCancelled());
							}
				void	finishLoading()
							{
								// Go all the way
								mFinishLoadingTriggered = true;

								// Setup to load
								mLoadLock.lock();
								if (!mGPUTextureReference.hasGPUTexture()) {
									// Need to finish
									if (mGPUTextureDataInfo == nil)
										// Do full load
										load();
									else
										// Register texture
										registerTexture();
								}
								mLoadLock.unlock();
							}

						// Instance methods for subclasses to call
				void	loadComplete(const CData& data, EGPUTextureDataFormat gpuTextureDataFormat, S2DSizeU16 size)
							{
								// Store
								mGPUTextureDataInfo = new SGPUTextureDataInfo(data, gpuTextureDataFormat, size);
								registerTexture();
							}

						// Private methods
				void	registerTexture()
							{
								// Register texture
								mGPUTextureReference =
										mGPUTextureManagerInfo.mGPU.registerTexture(mGPUTextureDataInfo->mData,
												mGPUTextureDataInfo->mGPUTextureDataFormat, mGPUTextureDataInfo->mSize);
							}

						// Instance methods for subclasses to implement or override
		virtual	void	load() = 0;
		virtual	void	unload()
							{
								// Check for workItem
								if (mWorkItem != nil) {
									// Cancel
									mGPUTextureManagerInfo.mWorkItemQueue.cancel(*mWorkItem);
									mWorkItem = nil;
								}

								// Check for render materia texture
								if (mGPUTextureDataInfo != nil) {
									// Cleanup
									Delete(mGPUTextureDataInfo);

									// Unregister with GPU Render Engine
									mGPUTextureManagerInfo.mGPU.unregisterTexture(mGPUTextureReference);
								}
							}

						// Class methods
		static	void	load(void* userData, CWorkItem& workItem)
							{
								// Get info
								CGPUTextureReferenceInternals&	internals =
																		*((CGPUTextureReferenceInternals*) userData);

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

		CString						mReference;
		OV<EGPUTextureDataFormat>	mGPUTextureDataFormat;
		EGPUTextureReferenceOptions	mGPUTextureReferenceOptions;
		SGPUTextureManagerInfo&		mGPUTextureManagerInfo;

		CLock						mLoadLock;
		CWorkItem*					mWorkItem;
		bool						mFinishLoadingTriggered;
		SGPUTextureDataInfo*		mGPUTextureDataInfo;
		SGPUTextureReference		mGPUTextureReference;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBitmapGPUTextureReferenceInternals

class CBitmapGPUTextureReferenceInternals : public CGPUTextureReferenceInternals {
	public:
										CBitmapGPUTextureReferenceInternals(const CBitmap& bitmap,
												const OR<const CString>& reference,
												const OV<EGPUTextureDataFormat>& gpuTextureDataFormat,
												EGPUTextureReferenceOptions gpuTextureReferenceOptions,
												SGPUTextureManagerInfo& gpuTextureManagerInfo) :
											CGPUTextureReferenceInternals(reference, gpuTextureDataFormat,
													gpuTextureReferenceOptions, gpuTextureManagerInfo),
											mBitmap(new CBitmap(bitmap)), mLoadingBitmap(nil)
											{}
										CBitmapGPUTextureReferenceInternals(const OR<const CString>& reference,
												const OV<EGPUTextureDataFormat>& gpuTextureDataFormat,
												EGPUTextureReferenceOptions gpuTextureReferenceOptions,
												SGPUTextureManagerInfo& gpuTextureManagerInfo) :
											CGPUTextureReferenceInternals(reference, gpuTextureDataFormat,
													gpuTextureReferenceOptions, gpuTextureManagerInfo),
											mBitmap(nil), mLoadingBitmap(nil)
											{}

										~CBitmapGPUTextureReferenceInternals()
											{
												// Cleanup
												Delete(mBitmap);
											}

										// CGPUTextureReferenceInternals methods
				void					load()
											{
												// Setup
												CBitmap*	bitmapUse = nil;
												CBitmap*	convertedBitmap = nil;
												S2DSizeS32	bitmapSize;

												// Is loading continuing
												if (isLoadingContinuing()) {
													// Setup
													bitmapUse = (mBitmap != nil) ? mBitmap : mLoadingBitmap;
													EBitmapFormat	bitmapUseFormat = bitmapUse->getFormat();

													if (!mGPUTextureDataFormat.hasValue())
														// Set value
														mGPUTextureDataFormat =
																resolvedGPUTextureDataFormat(bitmapUse->getFormat());

													EBitmapFormat	bitmapUseResolvedBitmapFormat =
																			resolvedBitmapFormat(*mGPUTextureDataFormat,
																					bitmapUseFormat);
													bitmapSize = bitmapUse->getSize();

													// Check bitmap formats
													if (bitmapUseResolvedBitmapFormat != bitmapUseFormat) {
														// Bitmap formats don't match
														convertedBitmap =
																new CBitmap(*bitmapUse, bitmapUseResolvedBitmapFormat);
														bitmapUse = convertedBitmap;

														// Cleanup
														Delete(mLoadingBitmap);
													}
												}

												if (isLoadingContinuing())
													// Create render material texture
													loadComplete(bitmapUse->getPixelData(),
															mGPUTextureDataFormat.getValue(),
															S2DSizeU16(bitmapSize.mWidth, bitmapSize.mHeight));

												// Cleanup
												Delete(mLoadingBitmap);
												Delete(convertedBitmap);
											}

										// Class methods
		static	EGPUTextureDataFormat	resolvedGPUTextureDataFormat(EBitmapFormat bitmapFormat)
											{
												switch (bitmapFormat) {
													// 16 bit formats
													case kBitmapFormatRGB565:	return kGPUTextureDataFormatRGB565;
													case kBitmapFormatRGBA4444:	return kGPUTextureDataFormatRGBA4444;
													case kBitmapFormatRGBA5551:	return kGPUTextureDataFormatRGBA5551;

													// 24 bit formats
													case kBitmapFormatRGB888:	return kGPUTextureDataFormatRGBA8888;

													// 32 bit formats
													case kBitmapFormatRGBA8888:	return kGPUTextureDataFormatRGBA8888;
													case kBitmapFormatARGB8888:	return kGPUTextureDataFormatRGBA8888;
												}
											}
		static	EBitmapFormat			resolvedBitmapFormat(EGPUTextureDataFormat gpuTextureDataFormat,
												EBitmapFormat fallbackBitmapFormat)
											{
												// What is the render material texture format
												switch (gpuTextureDataFormat) {
													// Convertable formats
													case kGPUTextureDataFormatRGB565:	return kBitmapFormatRGB565;
													case kGPUTextureDataFormatRGBA4444:	return kBitmapFormatRGBA4444;
													case kGPUTextureDataFormatRGBA5551:	return kBitmapFormatRGBA5551;

													case kGPUTextureDataFormatRGBA8888:	return kBitmapFormatRGBA8888;

													// Everything else does not correspond to a bitmap format
													default:							return fallbackBitmapFormat;
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
				// Lifecycle methods
				CDataGPUTextureReferenceInternals(const CByteParceller& byteParceller,
						EGPUTextureDataFormat gpuTextureDataFormat, S2DSizeU16 size,
						const OR<const CString>& reference, EGPUTextureReferenceOptions gpuTextureReferenceOptions,
						SGPUTextureManagerInfo& gpuTextureManagerInfo) :
					CGPUTextureReferenceInternals(reference, gpuTextureDataFormat, gpuTextureReferenceOptions,
							gpuTextureManagerInfo),
					mByteParceller(byteParceller), mSize(size)
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
							loadComplete(data, mGPUTextureDataFormat.getValue(), mSize);
					}

		const	CByteParceller	mByteParceller;
		const	S2DSizeU16		mSize;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBitmapProcGPUTextureReferenceInternals

class CBitmapProcGPUTextureReferenceInternals : public CBitmapGPUTextureReferenceInternals {
	public:
				// Lifecycle methods
				CBitmapProcGPUTextureReferenceInternals(const CByteParceller& byteParceller,
						CGPUTextureManagerBitmapProc bitmapProc, const OR<const CString>& reference,
						const OV<EGPUTextureDataFormat>& gpuTextureDataFormat,
						EGPUTextureReferenceOptions gpuTextureReferenceOptions,
						SGPUTextureManagerInfo& gpuTextureManagerInfo) :
					CBitmapGPUTextureReferenceInternals(reference, gpuTextureDataFormat, gpuTextureReferenceOptions,
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
const CGPUTexture& CGPUTextureReference::getGPUTexture() const
//----------------------------------------------------------------------------------------------------------------------
{
	return *mInternals->mGPUTextureReference.mGPUTexture;
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
CGPUTextureReference CGPUTextureManager::gpuTextureReference(const CBitmap& bitmap, const OR<const CString>& reference,
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
				return CGPUTextureReference(iterator.getValue()->addReference());
		}

	// Create new
	CGPUTextureReferenceInternals*	renderMaterialImageReferenceInternals =
											new CBitmapGPUTextureReferenceInternals(bitmap, reference,
													OV<EGPUTextureDataFormat>(), gpuTextureReferenceOptions,
													mInternals->mGPUTextureManagerInfo);
	CGPUTextureReference			gpuTextureReference(renderMaterialImageReferenceInternals);

	// Load
	renderMaterialImageReferenceInternals->loadOrQueueForLoading();

	return gpuTextureReference;
}

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureReference CGPUTextureManager::gpuTextureReference(const CBitmap& bitmap,
		EGPUTextureDataFormat gpuTextureDataFormat, const OR<const CString>& reference,
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
				return CGPUTextureReference(iterator.getValue()->addReference());
		}

	// Create new
	CGPUTextureReferenceInternals*	renderMaterialImageReferenceInternals =
											new CBitmapGPUTextureReferenceInternals(bitmap, reference,
													OV<EGPUTextureDataFormat>(gpuTextureDataFormat),
													gpuTextureReferenceOptions, mInternals->mGPUTextureManagerInfo);
	CGPUTextureReference			gpuTextureReference(renderMaterialImageReferenceInternals);

	// Load
	renderMaterialImageReferenceInternals->loadOrQueueForLoading();

	return gpuTextureReference;
}

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureReference CGPUTextureManager::gpuTextureReference(const CByteParceller& byteParceller,
		CGPUTextureManagerBitmapProc bitmapProc, const OR<const CString>& reference,
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
				return CGPUTextureReference(iterator.getValue()->addReference());
		}

	// Create new
	CGPUTextureReferenceInternals*	renderMaterialImageReferenceInternals =
											new CBitmapProcGPUTextureReferenceInternals(byteParceller, bitmapProc,
													reference, OV<EGPUTextureDataFormat>(), gpuTextureReferenceOptions,
													mInternals->mGPUTextureManagerInfo);
	CGPUTextureReference			gpuTextureReference(renderMaterialImageReferenceInternals);

	// Load
	renderMaterialImageReferenceInternals->loadOrQueueForLoading();

	return gpuTextureReference;
}

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureReference CGPUTextureManager::gpuTextureReference(const CByteParceller& byteParceller,
		CGPUTextureManagerBitmapProc bitmapProc, EGPUTextureDataFormat gpuTextureDataFormat,
		const OR<const CString>& reference, EGPUTextureReferenceOptions gpuTextureReferenceOptions)
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
				return CGPUTextureReference(iterator.getValue()->addReference());
		}

	// Create new
	CGPUTextureReferenceInternals*	renderMaterialImageReferenceInternals =
											new CBitmapProcGPUTextureReferenceInternals(byteParceller, bitmapProc,
													reference, OV<EGPUTextureDataFormat>(gpuTextureDataFormat),
													gpuTextureReferenceOptions, mInternals->mGPUTextureManagerInfo);
	CGPUTextureReference			gpuTextureReference(renderMaterialImageReferenceInternals);

	// Load
	renderMaterialImageReferenceInternals->loadOrQueueForLoading();

	return gpuTextureReference;
}

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureReference CGPUTextureManager::gpuTextureReference(const CByteParceller& byteParceller,
		EGPUTextureDataFormat gpuTextureDataFormat, S2DSizeU16 size, const OR<const CString>& reference,
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
				return CGPUTextureReference(iterator.getValue()->addReference());
		}

	// Create new
	CGPUTextureReferenceInternals*	renderMaterialImageReferenceInternals =
											new CDataGPUTextureReferenceInternals(byteParceller, gpuTextureDataFormat,
													size, reference, gpuTextureReferenceOptions,
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
