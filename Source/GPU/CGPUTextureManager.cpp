//----------------------------------------------------------------------------------------------------------------------
//	CGPUTextureManager.cpp			©2019 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CGPUTextureManager.h"

#include "ConcurrencyPrimitives.h"
#include "CLogServices.h"
#include "CWorkItemQueue.h"
#include "TWrappers.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SGPUTextureDataInfo

struct SGPUTextureDataInfo {
	// Lifecycle methods
	SGPUTextureDataInfo(const CData& data, CGPUTexture::DataFormat dataFormat, S2DSizeU16 size) :
		mData(data), mDataFormat(dataFormat), mSize(size)
	 {}

	// Properties
	const	CData					mData;
			CGPUTexture::DataFormat	mDataFormat;
			S2DSizeU16				mSize;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SGPUTextureManagerInfo

class CGPUTextureReferenceInternals;

struct SGPUTextureManagerInfo {
	// Lifecycle methods
	SGPUTextureManagerInfo(CGPU& gpu) : mGPU(gpu) {}

	// Properties
	CGPU&							mGPU;
	CWorkItemQueue					mWorkItemQueue;
	TNArray<CGPUTextureReference>	mGPUTextureReferences;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUTextureReferenceInternals

class CGPUTextureReferenceInternals : public TReferenceCountable<CGPUTextureReferenceInternals> {
	public:
						// Lifecycle methods
						CGPUTextureReferenceInternals(const OR<const CString>& reference,
								SGPUTextureManagerInfo& gpuTextureManagerInfo) :
							TReferenceCountable(),
									mReference(reference.hasReference() ? *reference : CString::mEmpty),
									mGPUTextureManagerInfo(gpuTextureManagerInfo)
							{}
						CGPUTextureReferenceInternals(const I<CGPUTexture>& gpuTexture,
								SGPUTextureManagerInfo& gpuTextureManagerInfo) :
							TReferenceCountable(),
									mReference(CString::mEmpty), mGPUTextureManagerInfo(gpuTextureManagerInfo),
									mGPUTexture(OI<I<CGPUTexture> >(gpuTexture))
							{}
		virtual			~CGPUTextureReferenceInternals()
							{
								// Cleanup
								unload();
							}

						// Instance methods
				bool	hasTwoLastReferences() const
							{ return getReferenceCount() == 2; }
				bool	getIsLoaded() const
							{ return mGPUTexture.hasInstance(); }
		virtual	void	loadOrQueueForLoading()
							{}
		virtual	void	finishLoading()
							{}
		virtual	void	unload()
							{
								// Check for texture
								if (mGPUTexture.hasInstance())
									// Unregister with GPU Render Engine
									mGPUTextureManagerInfo.mGPU.unregisterTexture(*mGPUTexture);
							}

		CString					mReference;
		SGPUTextureManagerInfo&	mGPUTextureManagerInfo;
		OI<I<CGPUTexture> >		mGPUTexture;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPULoadableTextureReferenceInternals

class CGPULoadableTextureReferenceInternals : public CGPUTextureReferenceInternals {
	public:
						// Lifecycle methods
						CGPULoadableTextureReferenceInternals(const OR<const CString>& reference,
								const OV<CGPUTexture::DataFormat>& dataFormat,
								CGPUTextureManager::ReferenceOptions referenceOptions,
								SGPUTextureManagerInfo& gpuTextureManagerInfo) :
							CGPUTextureReferenceInternals(reference, gpuTextureManagerInfo),
									mDataFormat(dataFormat), mReferenceOptions(referenceOptions),
									mWorkItem(nil), mFinishLoadingTriggered(false), mGPUTextureDataInfo(nil)
							{}

						// CGPUTextureReferenceInternals methods
				void	loadOrQueueForLoading()
							{
								// Check options
								if (mReferenceOptions & CGPUTextureManager::kReferenceOptionsLoadImmediately)
									// Finish loading
									finishLoading();
								else
									// Setup workItem
									mGPUTextureManagerInfo.mWorkItemQueue.add(load, this);
							}
				void	finishLoading()
							{
								// Go all the way
								mFinishLoadingTriggered = true;

								// Setup to load
								mLoadLock.lock();
								if (!mGPUTexture.hasInstance()) {
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
				void	unload()
							{
								// Check for workItem
								if (mWorkItem != nil) {
									// Cancel
									mGPUTextureManagerInfo.mWorkItemQueue.cancel(*mWorkItem);
									mWorkItem = nil;
								}

								// Check for render materia texture
								if (mGPUTextureDataInfo != nil)
									// Cleanup
									Delete(mGPUTextureDataInfo);

								// Do super
								CGPUTextureReferenceInternals::unload();
							}

						// Instance methods
				bool	isLoadingContinuing()
							{ return mFinishLoadingTriggered || ((mWorkItem != nil) && !mWorkItem->isCancelled()); }

						// Instance methods for subclasses to call
				void	loadComplete(const CData& data, CGPUTexture::DataFormat dataFormat, S2DSizeU16 size)
							{
								// Store
								mGPUTextureDataInfo = new SGPUTextureDataInfo(data, dataFormat, size);
								registerTexture();
							}

						// Private methods
				void	registerTexture()
							{
								// Register texture
								mGPUTexture =
										OI<I<CGPUTexture> >(
												mGPUTextureManagerInfo.mGPU.registerTexture(mGPUTextureDataInfo->mData,
														mGPUTextureDataInfo->mDataFormat, mGPUTextureDataInfo->mSize));
							}

						// Instance methods for subclasses to implement or override
		virtual	void	load() = 0;

						// Class methods
		static	void	load(CWorkItem& workItem, void* userData)
							{
								// Get info
								CGPULoadableTextureReferenceInternals&	internals =
																				*((CGPULoadableTextureReferenceInternals*)
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

		OV<CGPUTexture::DataFormat>				mDataFormat;
		CGPUTextureManager::ReferenceOptions	mReferenceOptions;

		CLock									mLoadLock;
		CWorkItem*								mWorkItem;
		bool									mFinishLoadingTriggered;
		SGPUTextureDataInfo*					mGPUTextureDataInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBitmapGPUTextureReferenceInternals

class CBitmapGPUTextureReferenceInternals : public CGPULoadableTextureReferenceInternals {
	public:
										CBitmapGPUTextureReferenceInternals(const CBitmap& bitmap,
												const OR<const CString>& reference,
												const OV<CGPUTexture::DataFormat>& dataFormat,
												CGPUTextureManager::ReferenceOptions referenceOptions,
												SGPUTextureManagerInfo& gpuTextureManagerInfo) :
											CGPULoadableTextureReferenceInternals(reference, dataFormat,
													referenceOptions, gpuTextureManagerInfo),
											mBitmap(new CBitmap(bitmap)), mLoadingBitmap(nil)
											{}
										CBitmapGPUTextureReferenceInternals(const OR<const CString>& reference,
												const OV<CGPUTexture::DataFormat>& dataFormat,
												CGPUTextureManager::ReferenceOptions referenceOptions,
												SGPUTextureManagerInfo& gpuTextureManagerInfo) :
											CGPULoadableTextureReferenceInternals(reference, dataFormat,
													referenceOptions, gpuTextureManagerInfo),
											mBitmap(nil), mLoadingBitmap(nil)
											{}

										~CBitmapGPUTextureReferenceInternals()
											{
												// Cleanup
												Delete(mBitmap);
											}

										// CGPULoadableTextureReferenceInternals methods
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
													CBitmap::Format	bitmapUseFormat = bitmapUse->getFormat();

													if (!mDataFormat.hasValue())
														// Set value
														mDataFormat =
																resolvedGPUTextureDataFormat(bitmapUse->getFormat());

													CBitmap::Format	bitmapUseResolvedBitmapFormat =
																			resolvedBitmapFormat(*mDataFormat,
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
													loadComplete(bitmapUse->getPixelData(), mDataFormat.getValue(),
															S2DSizeU16(bitmapSize.mWidth, bitmapSize.mHeight));

												// Cleanup
												Delete(mLoadingBitmap);
												Delete(convertedBitmap);
											}

										// Class methods
		static	CGPUTexture::DataFormat	resolvedGPUTextureDataFormat(CBitmap::Format bitmapFormat)
											{
												switch (bitmapFormat) {
													// 16 bit formats
//													case CBitmap::kFormatRGB565:
//														return kGPUTextureDataFormatRGB565;
//													case CBitmap::kFormatRGBA4444:
//														return kGPUTextureDataFormatRGBA4444;
//													case CBitmap::kFormatRGBA5551:
//														return kGPUTextureDataFormatRGBA5551;

													// 24 bit formats
													case CBitmap::kFormatRGB888:
															return CGPUTexture::kDataFormatRGBA8888;

													// 32 bit formats
													case CBitmap::kFormatRGBA8888:
														return CGPUTexture::kDataFormatRGBA8888;
													case CBitmap::kFormatARGB8888:
														return CGPUTexture::kDataFormatRGBA8888;

													default:
														return CGPUTexture::kDataFormatRGBA8888;
												}
											}
		static	CBitmap::Format			resolvedBitmapFormat(CGPUTexture::DataFormat dataFormat,
												CBitmap::Format fallbackBitmapFormat)
											{
												// What is the render material texture format
												switch (dataFormat) {
													// Convertable formats
//													case CGPUTexture::kDataFormatRGB565:	return CBitmap::kFormatRGB565;
//													case CGPUTexture::kDataFormatRGBA4444:	return CBitmap::kFormatRGBA4444;
//													case CGPUTexture::kDataFormatRGBA5551:	return CBitmap::kFormatRGBA5551;

													case CGPUTexture::kDataFormatRGBA8888:	return CBitmap::kFormatRGBA8888;

													// Everything else does not correspond to a bitmap format
													default:								return fallbackBitmapFormat;
												}
											}

		CBitmap*	mBitmap;

		CBitmap*	mLoadingBitmap;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDataGPUTextureReferenceInternals

class CDataGPUTextureReferenceInternals : public CGPULoadableTextureReferenceInternals {
	public:
				// Lifecycle methods
				CDataGPUTextureReferenceInternals(const I<CDataSource>& dataSource, CGPUTexture::DataFormat dataFormat,
						S2DSizeU16 size, const OR<const CString>& reference,
						CGPUTextureManager::ReferenceOptions referenceOptions,
						SGPUTextureManagerInfo& gpuTextureManagerInfo) :
					CGPULoadableTextureReferenceInternals(reference, dataFormat, referenceOptions,
							gpuTextureManagerInfo),
					mDataSource(dataSource), mSize(size)
					{}

				// CGPULoadableTextureReferenceInternals methods
		void	load()
					{
						// Setup
						CData	textureData;

						// Is loading continuing
						if (isLoadingContinuing()) {
							// Read data
							TIResult<CData>	dataResult = mDataSource->readData();
							LogIfErrorAndReturn(dataResult.getError(), "reading data from data provider");

							textureData = dataResult.getValue();
						}

						// Is loading continuing
						if (isLoadingContinuing())
							// Create render material texture
							loadComplete(textureData, mDataFormat.getValue(), mSize);
					}

		const	I<CDataSource>	mDataSource;
		const	S2DSizeU16		mSize;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBitmapProcGPUTextureReferenceInternals

class CBitmapProcGPUTextureReferenceInternals : public CBitmapGPUTextureReferenceInternals {
	public:
				// Lifecycle methods
				CBitmapProcGPUTextureReferenceInternals(const I<CDataSource>& dataSource,
						CGPUTextureManager::BitmapProc bitmapProc, const OR<const CString>& reference,
						const OV<CGPUTexture::DataFormat>& dataFormat,
						CGPUTextureManager::ReferenceOptions referenceOptions,
						SGPUTextureManagerInfo& gpuTextureManagerInfo) :
					CBitmapGPUTextureReferenceInternals(reference, dataFormat, referenceOptions, gpuTextureManagerInfo),
						mDataSource(dataSource), mBitmapProc(bitmapProc)
					{}

				// CGPULoadableTextureReferenceInternals methods
		void	load()
					{
						// Is loading continuing
						if (isLoadingContinuing())
							// Create bitmap
							mLoadingBitmap = new CBitmap(mBitmapProc(*mDataSource->readData().getValue()));

						// Do super
						CBitmapGPUTextureReferenceInternals::load();
					}

		const	I<CDataSource>					mDataSource;
				CGPUTextureManager::BitmapProc	mBitmapProc;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUTextureReference

// MARK: Lifecyclde methods

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureReference::CGPUTextureReference(CGPUTextureReferenceInternals& internals)
//----------------------------------------------------------------------------------------------------------------------
{
	// A reference is specified to disallow anyone else creating an instance of this class.  This constructor is only
	//	ever called from internal to this file so we know that we are simply passing the required pointer as a
	//	reference.
	mInternals = &internals;
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
	// Check which path we are following
	if (mInternals->hasTwoLastReferences()) {
		// Two references
		mInternals->removeReference();
		mInternals->mGPUTextureManagerInfo.mGPUTextureReferences -= *this;
	} else
		// Otherwise
		mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CString& CGPUTextureReference::getReference() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mReference;
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
void CGPUTextureReference::unload() const
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->unload();
}

//----------------------------------------------------------------------------------------------------------------------
const I<CGPUTexture>& CGPUTextureReference::getGPUTexture() const
//----------------------------------------------------------------------------------------------------------------------
{
	return *mInternals->mGPUTexture;
}

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureReference& CGPUTextureReference::operator=(const CGPUTextureReference& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if assignment to self
	if (this == &other)
		return *this;

	// Remove reference to ourselves
	mInternals->removeReference();

	// Add reference to other
	mInternals = other.mInternals->addReference();

	return *this;
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
		ReferenceOptions referenceOptions)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if we have a reference
	if (reference.hasReference())
		// Look for file in already loaded render material images
		for (TIteratorD<CGPUTextureReference> iterator =
						mInternals->mGPUTextureManagerInfo.mGPUTextureReferences.getIterator();
				iterator.hasValue(); iterator.advance()) {
			// Check this one
			if (!iterator.getValue().getReference().isEmpty() && (iterator.getValue().getReference() == *reference))
				// Found existing
				return iterator.getValue();
		}

	// Create new
	CGPULoadableTextureReferenceInternals*	gpuLoadableTextureReferenceInternals =
													new CBitmapGPUTextureReferenceInternals(bitmap, reference,
															OV<CGPUTexture::DataFormat>(), referenceOptions,
															mInternals->mGPUTextureManagerInfo);
	CGPUTextureReference					textureReference(*gpuLoadableTextureReferenceInternals);
	mInternals->mGPUTextureManagerInfo.mGPUTextureReferences += textureReference;

	// Load
	gpuLoadableTextureReferenceInternals->loadOrQueueForLoading();

	return textureReference;
}

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureReference CGPUTextureManager::gpuTextureReference(const CBitmap& bitmap, CGPUTexture::DataFormat dataFormat,
		const OR<const CString>& reference, ReferenceOptions referenceOptions)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if we have a reference
	if (reference.hasReference())
		// Look for file in already loaded render material images
		for (TIteratorD<CGPUTextureReference> iterator =
						mInternals->mGPUTextureManagerInfo.mGPUTextureReferences.getIterator();
				iterator.hasValue(); iterator.advance()) {
			// Check this one
			if (!iterator.getValue().getReference().isEmpty() && (iterator.getValue().getReference() == *reference))
				// Found existing
				return iterator.getValue();
		}

	// Create new
	CGPULoadableTextureReferenceInternals*	gpuLoadableTextureReferenceInternals =
													new CBitmapGPUTextureReferenceInternals(bitmap, reference,
															OV<CGPUTexture::DataFormat>(dataFormat), referenceOptions,
															mInternals->mGPUTextureManagerInfo);
	CGPUTextureReference					textureReference(*gpuLoadableTextureReferenceInternals);
	mInternals->mGPUTextureManagerInfo.mGPUTextureReferences += textureReference;

	// Load
	gpuLoadableTextureReferenceInternals->loadOrQueueForLoading();

	return textureReference;
}

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureReference CGPUTextureManager::gpuTextureReference(const I<CDataSource>& dataSource, BitmapProc bitmapProc,
		const OR<const CString>& reference, ReferenceOptions referenceOptions)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if we have a reference
	if (reference.hasReference())
		// Look for file in already loaded render material images
		for (TIteratorD<CGPUTextureReference> iterator =
						mInternals->mGPUTextureManagerInfo.mGPUTextureReferences.getIterator();
				iterator.hasValue(); iterator.advance()) {
			// Check this one
			if (!iterator.getValue().getReference().isEmpty() && (iterator.getValue().getReference() == *reference))
				// Found existing
				return iterator.getValue();
		}

	// Create new
	CGPULoadableTextureReferenceInternals*	gpuLoadableTextureReferenceInternals =
													new CBitmapProcGPUTextureReferenceInternals(dataSource,
															bitmapProc, reference, OV<CGPUTexture::DataFormat>(),
															referenceOptions, mInternals->mGPUTextureManagerInfo);
	CGPUTextureReference					textureReference(*gpuLoadableTextureReferenceInternals);
	mInternals->mGPUTextureManagerInfo.mGPUTextureReferences += textureReference;

	// Load
	gpuLoadableTextureReferenceInternals->loadOrQueueForLoading();

	return textureReference;
}

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureReference CGPUTextureManager::gpuTextureReference(const I<CDataSource>& dataSource, BitmapProc bitmapProc,
		CGPUTexture::DataFormat dataFormat, const OR<const CString>& reference, ReferenceOptions referenceOptions)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if we have a reference
	if (reference.hasReference())
		// Look for file in already loaded render material images
		for (TIteratorD<CGPUTextureReference> iterator =
						mInternals->mGPUTextureManagerInfo.mGPUTextureReferences.getIterator();
				iterator.hasValue(); iterator.advance()) {
			// Check this one
			if (!iterator.getValue().getReference().isEmpty() && (iterator.getValue().getReference() == *reference))
				// Found existing
				return iterator.getValue();
		}

	// Create new
	CGPULoadableTextureReferenceInternals*	gpuLoadableTextureReferenceInternals =
													new CBitmapProcGPUTextureReferenceInternals(dataSource,
															bitmapProc, reference,
															OV<CGPUTexture::DataFormat>(dataFormat), referenceOptions,
															mInternals->mGPUTextureManagerInfo);
	CGPUTextureReference					textureReference(*gpuLoadableTextureReferenceInternals);
	mInternals->mGPUTextureManagerInfo.mGPUTextureReferences += textureReference;

	// Load
	gpuLoadableTextureReferenceInternals->loadOrQueueForLoading();

	return textureReference;
}

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureReference CGPUTextureManager::gpuTextureReference(const I<CDataSource>& dataSource,
		CGPUTexture::DataFormat dataFormat, S2DSizeU16 size, const OR<const CString>& reference,
		ReferenceOptions referenceOptions)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if we have a reference
	if (reference.hasReference())
		// Look for file in already loaded render material images
		for (TIteratorD<CGPUTextureReference> iterator =
						mInternals->mGPUTextureManagerInfo.mGPUTextureReferences.getIterator();
				iterator.hasValue(); iterator.advance()) {
			// Check this one
			if (!iterator.getValue().getReference().isEmpty() && (iterator.getValue().getReference() == *reference))
				// Found existing
				return iterator.getValue();
		}

	// Create new
	CGPULoadableTextureReferenceInternals*	gpuLoadableTextureReferenceInternals =
													new CDataGPUTextureReferenceInternals(dataSource, dataFormat,
															size, reference, referenceOptions,
															mInternals->mGPUTextureManagerInfo);
	CGPUTextureReference					textureReference(*gpuLoadableTextureReferenceInternals);
	mInternals->mGPUTextureManagerInfo.mGPUTextureReferences += textureReference;

	// Load
	gpuLoadableTextureReferenceInternals->loadOrQueueForLoading();

	return textureReference;
}

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureReference CGPUTextureManager::gpuTextureReference(const I<CGPUTexture>& gpuTexture)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create new
	CGPUTextureReferenceInternals*	gpuTextureReferenceInternals =
											new CGPUTextureReferenceInternals(gpuTexture,
													mInternals->mGPUTextureManagerInfo);
	CGPUTextureReference					textureReference(*gpuTextureReferenceInternals);
	mInternals->mGPUTextureManagerInfo.mGPUTextureReferences += textureReference;

	return textureReference;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPUTextureManager::loadAll()
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all
	for (TIteratorD<CGPUTextureReference> iterator =
					mInternals->mGPUTextureManagerInfo.mGPUTextureReferences.getIterator();
			iterator.hasValue(); iterator.advance())
		// Start loading
		iterator.getValue().load();
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
	for (TIteratorD<CGPUTextureReference> iterator =
					mInternals->mGPUTextureManagerInfo.mGPUTextureReferences.getIterator();
			iterator.hasValue(); iterator.advance())
		// Unload
		iterator.getValue().unload();
}
