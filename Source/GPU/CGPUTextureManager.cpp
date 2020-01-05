//----------------------------------------------------------------------------------------------------------------------
//	CGPUTextureManager.cpp			©2019 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CGPUTextureManager.h"

#include "CImage.h"
#include "CLogServices.h"
#include "CWorkItemQueue.h"
#include "TOptional.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SGPUTextureManagerInfo

class CGPUTextureReferenceInternals;

struct SGPUTextureManagerInfo {
	// Lifecycle methods
	SGPUTextureManagerInfo(CGPURenderEngine& gpuRenderEngine) : mGPURenderEngine(gpuRenderEngine) {}

	// Properties
	CGPURenderEngine&							mGPURenderEngine;
	CWorkItemQueue								mWorkItemQueue;
	TPtrArray<CGPUTextureReferenceInternals*>	mGPUTextureReferenceInternals;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUTextureReferenceInternals

class CGPUTextureReferenceInternals {
	public:
												CGPUTextureReferenceInternals(const OR<const CString>& reference,
														const OV<EGPUTextureFormat>& gpuTextureFormat,
														EGPUTextureReferenceOptions gpuTextureReferenceOptions,
														SGPUTextureManagerInfo& gpuTextureManagerInfo) :
													mReference(reference), mGPUTextureFormat(gpuTextureFormat),
															mGPUTextureReferenceOptions(gpuTextureReferenceOptions),
															mGPUTextureManagerInfo(gpuTextureManagerInfo),
															mWorkItem(nil), mCompleteLoading(false), mGPUTexture(nil),
															mReferenceCount(0)
													{
														// Note reference
														mGPUTextureManagerInfo.mGPUTextureReferenceInternals += this;
													}
		virtual									~CGPUTextureReferenceInternals()
													{
														// Remove from references
														mGPUTextureManagerInfo.mGPUTextureReferenceInternals -= this;

														// Cleanup
														DisposeOf(mGPUTexture);
													}

				CGPUTextureReferenceInternals*	addReference() { mReferenceCount++; return this; }
				void							removeReference()
													{
														// Decrement and see if we are the last one
														if (--mReferenceCount == 0) {
															// Dispose
															CGPUTextureReferenceInternals*	THIS = this;
															DisposeOf(THIS);
														}
													}

				bool							getIsLoaded() const
													{ return mGPUTexture != nil; }
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
														return mCompleteLoading ||
																((mWorkItem != nil) && !mWorkItem->isCancelled());
													}
				void							finishLoading()
													{
														// Check for workItem
														if (mWorkItem != nil) {
															// Cancel
															mGPUTextureManagerInfo.mWorkItemQueue.cancel(*mWorkItem);
															mWorkItem = nil;
														}

														// Go all the way
														mCompleteLoading = true;

														// Load
														load();
													}

												// Instance methods for subclasses to call
				void							loadComplete(CGPUTexture* gpuTexture)
													{
														// Store
														mGPUTexture = gpuTexture;

														// Register with GPU Render Engine
														mGPURenderEngineTextureInfo =
																mGPUTextureManagerInfo.
																		mGPURenderEngine.registerTexture(*mGPUTexture);
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
																	mGPURenderEngine.unregisterTexture(
																			mGPURenderEngineTextureInfo);
														}
													}

												// Class methods
		static	void							load(void* userData, CWorkItem& workItem)
													{
														// Get info
														CGPUTextureReferenceInternals* internals;
														internals = (CGPUTextureReferenceInternals*) userData;
														internals->mWorkItem = &workItem;

														// Load
														internals->load();

														// Finished
														internals->mWorkItem = nil;
													}

		OR<const CString>				mReference;
		OV<EGPUTextureFormat>			mGPUTextureFormat;
		EGPUTextureReferenceOptions		mGPUTextureReferenceOptions;
		SGPUTextureManagerInfo&			mGPUTextureManagerInfo;

		CWorkItem*						mWorkItem;
		bool							mCompleteLoading;
		CGPUTexture*					mGPUTexture;
		SGPURenderEngineTextureInfo		mGPURenderEngineTextureInfo;

		UInt32							mReferenceCount;
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
// MARK: - CImageGPUTextureReferenceInternals

class CImageGPUTextureReferenceInternals : public CBitmapGPUTextureReferenceInternals {
	public:
				CImageGPUTextureReferenceInternals(const CByteParceller& byteParceller,
						const OR<const CString>& reference, const OV<EGPUTextureFormat>& gpuTextureFormat,
						EGPUTextureReferenceOptions gpuTextureReferenceOptions,
						SGPUTextureManagerInfo& gpuTextureManagerInfo) :
					CBitmapGPUTextureReferenceInternals(reference, gpuTextureFormat, gpuTextureReferenceOptions,
							gpuTextureManagerInfo),
					mByteParceller(byteParceller)
					{}

				// CGPUTextureReferenceInternals methods
		void	load()
					{
						// Is loading continuing
						CImage*	image = nil;
						if (isLoadingContinuing())
							// Create image
							image = new CImage(mByteParceller);

						// Is loading continuing
						if (isLoadingContinuing())
							// Create bitmap
							mLoadingBitmap = new CBitmap(image->getBitmap());
						DisposeOf(image);

						// Do super
						CBitmapGPUTextureReferenceInternals::load();
					}

		const	CByteParceller	mByteParceller;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUTextureReference

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureReference::CGPUTextureReference(CGPUTextureReferenceInternals* internals)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = internals->addReference();
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
bool CGPUTextureReference::getIsLoaded() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getIsLoaded();
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
const SGPURenderEngineTextureInfo& CGPUTextureReference::getGPURenderingEngineTextureInfo() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mGPURenderEngineTextureInfo;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUTextureManagerInternals

class CGPUTextureManagerInternals {
	public:
										CGPUTextureManagerInternals(CGPURenderEngine& gpuRenderEngine) :
											mReferenceCount(1), mGPUTextureManagerInfo(gpuRenderEngine) {}
										~CGPUTextureManagerInternals() {}

		CGPUTextureManagerInternals*	addReference()
											{ mReferenceCount++; return this; }
		void							removeReference()
											{
												// Decrement reference count and check if we are the last one
												if (--mReferenceCount == 0) {
													// We going away
													CGPUTextureManagerInternals*	THIS = this;
													DisposeOf(THIS);
												}
											}

		UInt32					mReferenceCount;
		SGPUTextureManagerInfo	mGPUTextureManagerInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUTextureManager

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureManager::CGPUTextureManager(CGPURenderEngine& gpuRenderEngine)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CGPUTextureManagerInternals(gpuRenderEngine);
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
			if (iterator.getValue()->mReference.hasReference() && (*iterator.getValue()->mReference == *reference))
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
			if (iterator.getValue()->mReference.hasReference() && (*iterator.getValue()->mReference == *reference))
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
CGPUTextureReference CGPUTextureManager::gpuTextureReference(const CByteParceller& imageByteParceller,
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
			if (iterator.getValue()->mReference.hasReference() && (*iterator.getValue()->mReference == *reference))
				// Found existing
				return CGPUTextureReference(iterator.getValue());
		}

	// Create new
	CGPUTextureReferenceInternals*	renderMaterialImageReferenceInternals =
											new CImageGPUTextureReferenceInternals(imageByteParceller, reference,
													OV<EGPUTextureFormat>(), gpuTextureReferenceOptions,
													mInternals->mGPUTextureManagerInfo);
	CGPUTextureReference			gpuTextureReference(renderMaterialImageReferenceInternals);

	// Load
	renderMaterialImageReferenceInternals->loadOrQueueForLoading();

	return gpuTextureReference;
}

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureReference CGPUTextureManager::gpuTextureReference(const CByteParceller& imageByteParceller,
		EGPUTextureFormat gpuTextureFormat, OR<const CString> reference,
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
			if (iterator.getValue()->mReference.hasReference() && (*iterator.getValue()->mReference == *reference))
				// Found existing
				return CGPUTextureReference(iterator.getValue());
		}

	// Create new
	CGPUTextureReferenceInternals*	renderMaterialImageReferenceInternals =
											new CImageGPUTextureReferenceInternals(imageByteParceller, reference,
													OV<EGPUTextureFormat>(gpuTextureFormat),
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
			if (iterator.getValue()->mReference.hasReference() && (*iterator.getValue()->mReference == *reference))
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
