//----------------------------------------------------------------------------------------------------------------------
//	CGPUTextureManager.cpp			©2019 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CGPUTextureManager.h"

#include "ConcurrencyPrimitives.h"
#include "CLogServices.h"
#include "CReferenceCountable.h"
#include "CWorkItemQueue.h"
#include "TWrappers.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SGPUTextureDataInfo

struct SGPUTextureDataInfo {
	// Methods
	public:
								// Lifecycle methods
								SGPUTextureDataInfo(const CBitmap& bitmap) : mBitmap(bitmap) {}
								SGPUTextureDataInfo(const CData& data, S2DSizeU16 dimensions) :
									mData(data), mDataDimensions(dimensions)
									{}
								SGPUTextureDataInfo(const SGPUTextureDataInfo& other) :
									mBitmap(other.mBitmap), mData(other.mData), mDataDimensions(other.mDataDimensions)
									{}

								// Instance methods
		const	OV<CBitmap>&	getBitmap() const
									{ return mBitmap; }

		const	OV<CData>&		getData() const
									{ return mData; }
		const	OV<S2DSizeU16>&	getDataDimensions() const
									{ return mDataDimensions; }

	// Properties
	private:
		OV<CBitmap>				mBitmap;

		OV<CData>				mData;
		OV<S2DSizeU16>			mDataDimensions;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SGPUTextureManagerInfo

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
// MARK: - CGPUTextureReference::Internals

class CGPUTextureReference::Internals : public TReferenceCountableAutoDelete<Internals> {
	public:
						// Lifecycle methods
						Internals(const OV<CString>& reference, SGPUTextureManagerInfo& gpuTextureManagerInfo) :
							TReferenceCountableAutoDelete(),
									mReference(reference), mGPUTextureManagerInfo(gpuTextureManagerInfo)
							{}
						Internals(const I<CGPUTexture>& gpuTexture, SGPUTextureManagerInfo& gpuTextureManagerInfo) :
							TReferenceCountableAutoDelete(),
									mReference(CString::mEmpty), mGPUTextureManagerInfo(gpuTextureManagerInfo),
									mGPUTexture(OV<I<CGPUTexture> >(gpuTexture))
							{}
		virtual			~Internals()
							{
								// Cleanup
								unload();
							}

						// Instance methods
				bool	hasTwoLastReferences() const
							{ return getReferenceCount() == 2; }
				bool	getIsLoaded() const
							{ return mGPUTexture.hasValue(); }
		virtual	void	loadOrQueueForLoading()
							{}
		virtual	void	finishLoading()
							{}
		virtual	void	unload()
							{
								// Check for texture
								if (mGPUTexture.hasValue())
									// Unregister with GPU Render Engine
									mGPUTextureManagerInfo.mGPU.unregisterTexture(*mGPUTexture);
							}

		OV<CString>				mReference;
		SGPUTextureManagerInfo&	mGPUTextureManagerInfo;
		OV<I<CGPUTexture> >		mGPUTexture;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPULoadableTextureReferenceInternals

class CGPULoadableTextureReferenceInternals : public CGPUTextureReference::Internals {
	public:
						// Lifecycle methods
						CGPULoadableTextureReferenceInternals(const OV<CString>& reference,
								const OV<CGPUTexture::DataFormat>& gpuTextureDataFormat,
								CGPUTextureManager::ReferenceOptions referenceOptions,
								SGPUTextureManagerInfo& gpuTextureManagerInfo) :
							CGPUTextureReference::Internals(reference, gpuTextureManagerInfo),
									mGPUTextureDataFormat(gpuTextureDataFormat), mReferenceOptions(referenceOptions),
									mFinishLoadingTriggered(false)
							{}
						~CGPULoadableTextureReferenceInternals()
							{
								// Check for workItem
								mLoadLock.lock();
								if (mWorkItem.hasValue()) {
									// Cancel
									mGPUTextureManagerInfo.mWorkItemQueue.cancel(*mWorkItem);
									mWorkItem.removeValue();
								}
								mLoadLock.unlock();

								// Check for render materia texture
								mGPUTextureDataInfo.removeValue();
							}

						// CGPUTextureReference::Internals methods
				void	loadOrQueueForLoading()
							{
								// Check options
								if (mReferenceOptions & CGPUTextureManager::kReferenceOptionsLoadImmediately)
									// Finish loading
									finishLoading();
								else {
									// Setup workItem
									mLoadLock.lock();
									mWorkItem.setValue(
											mGPUTextureManagerInfo.mWorkItemQueue.add((CWorkItem::Proc) workItemLoad,
													this, mReference));
									mLoadLock.unlock();
								}
							}
				void	finishLoading()
							{
								// Go all the way
								mFinishLoadingTriggered = true;

								// Setup to load
								mLoadLock.lock();

								// Cancel work item
								if (mWorkItem.hasValue()) {
									// Cancel
									mGPUTextureManagerInfo.mWorkItemQueue.cancel(*mWorkItem);
									mWorkItem.removeValue();
								}

								if (!mGPUTexture.hasValue()) {
									// Need to finish
									if (!mGPUTextureDataInfo.hasValue())
										// Do full load
										load();
									else
										// Register texture
										registerTexture();
								}

								// All done
								mLoadLock.unlock();
							}
				void	unload()
							{
								// Check for workItem
								mLoadLock.lock();
								if (mWorkItem.hasValue()) {
									// Cancel
									mGPUTextureManagerInfo.mWorkItemQueue.cancel(*mWorkItem);
									mWorkItem.removeValue();
								}
								mLoadLock.unlock();

								// Check for render materia texture
								mGPUTextureDataInfo.removeValue();

								// Do super
								CGPUTextureReference::Internals::unload();
							}

						// Instance methods for subclasses to call
				bool	isLoadingContinuing()
							{ return mFinishLoadingTriggered ||
									(mWorkItem.hasValue() && !(*mWorkItem)->isCancelled()); }

				void	loadComplete(const CBitmap& bitmap, CGPUTexture::DataFormat gpuTextureDataFormat)
							{
								// Store
								mGPUTextureDataFormat.setValue(gpuTextureDataFormat);
								mGPUTextureDataInfo.setValue(SGPUTextureDataInfo(bitmap));

								// Register texture
								registerTexture();
							}
				void	loadComplete(const CData& data, S2DSizeU16 dimensions,
								CGPUTexture::DataFormat gpuTextureDataFormat)
							{
								// Store
								mGPUTextureDataFormat.setValue(gpuTextureDataFormat);
								mGPUTextureDataInfo.setValue(SGPUTextureDataInfo(data, dimensions));

								// Register texture
								registerTexture();
							}

						// Private methods
				void	registerTexture()
							{
								// Check situation
								if (mGPUTextureDataInfo->getBitmap().hasValue())
									// Register texture from bitmap
									mGPUTexture =
											OV<I<CGPUTexture> >(
													mGPUTextureManagerInfo.mGPU.registerTexture(
															*mGPUTextureDataInfo->getBitmap(),
															*mGPUTextureDataFormat));
								else
									// Register texture from data
									mGPUTexture =
											OV<I<CGPUTexture> >(
													mGPUTextureManagerInfo.mGPU.registerTexture(
															*mGPUTextureDataInfo->getData(),
															*mGPUTextureDataInfo->getDataDimensions(),
															*mGPUTextureDataFormat));
							}

						// Instance methods for subclasses to implement or override
		virtual	void	load() = 0;

						// Class methods
		static	void	workItemLoad(const I<CWorkItem>& workItem, CGPULoadableTextureReferenceInternals* internals)
							{
								// Setup to load
								internals->mLoadLock.lock();

								// Check if loaded
								if (!internals->getIsLoaded())
									// Load
									internals->load();

								// Finished
								internals->mWorkItem.removeValue();

								// All done
								internals->mLoadLock.unlock();
							}

		OV<CGPUTexture::DataFormat>				mGPUTextureDataFormat;
		CGPUTextureManager::ReferenceOptions	mReferenceOptions;

		CLock									mLoadLock;
		OV<I<CWorkItem> >						mWorkItem;
		bool									mFinishLoadingTriggered;
		OV<SGPUTextureDataInfo>					mGPUTextureDataInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBitmapGPUTextureReferenceInternals

class CBitmapGPUTextureReferenceInternals : public CGPULoadableTextureReferenceInternals {
	public:
										CBitmapGPUTextureReferenceInternals(const CBitmap& bitmap,
												const OV<CString>& reference,
												const OV<CGPUTexture::DataFormat>& dataFormat,
												CGPUTextureManager::ReferenceOptions referenceOptions,
												SGPUTextureManagerInfo& gpuTextureManagerInfo) :
											CGPULoadableTextureReferenceInternals(reference, dataFormat,
													referenceOptions, gpuTextureManagerInfo),
											mBitmap(new CBitmap(bitmap)), mLoadingBitmap(nil)
											{}
										CBitmapGPUTextureReferenceInternals(const OV<CString>& reference,
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

													if (!mGPUTextureDataFormat.hasValue())
														// Set value
														mGPUTextureDataFormat =
																resolvedGPUTextureDataFormat(bitmapUseFormat);

													CBitmap::Format	bitmapUseResolvedBitmapFormat =
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
													loadComplete(*bitmapUse, *mGPUTextureDataFormat);

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
						S2DSizeU16 dimensions, const OV<CString>& reference,
						CGPUTextureManager::ReferenceOptions referenceOptions,
						SGPUTextureManagerInfo& gpuTextureManagerInfo) :
					CGPULoadableTextureReferenceInternals(reference, dataFormat, referenceOptions,
							gpuTextureManagerInfo),
					mDataSource(dataSource), mDimensions(dimensions)
					{}

				// CGPULoadableTextureReferenceInternals methods
		void	load()
					{
						// Setup
						CData	textureData;

						// Is loading continuing
						if (isLoadingContinuing()) {
							// Read data
							TVResult<CData>	dataResult = mDataSource->readData();
							LogIfResultErrorAndReturn(dataResult, "reading data from data provider");

							textureData = *dataResult;
						}

						// Is loading continuing
						if (isLoadingContinuing())
							// Create render material texture
							loadComplete(textureData, mDimensions, *mGPUTextureDataFormat);
					}

		const	I<CDataSource>	mDataSource;
		const	S2DSizeU16		mDimensions;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBitmapProcGPUTextureReferenceInternals

class CBitmapProcGPUTextureReferenceInternals : public CBitmapGPUTextureReferenceInternals {
	public:
				// Lifecycle methods
				CBitmapProcGPUTextureReferenceInternals(const I<CDataSource>& dataSource,
						CGPUTextureManager::BitmapProc bitmapProc, const OV<CString>& reference,
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
							mLoadingBitmap = new CBitmap(*mBitmapProc(*mDataSource->readData()));

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
CGPUTextureReference::CGPUTextureReference(Internals& internals)
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
const OV<CString>& CGPUTextureReference::getReference() const
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
// MARK: - CGPUTextureManager::Internals

class CGPUTextureManager::Internals : public TReferenceCountableAutoDelete<Internals> {
	public:
		Internals(CGPU& gpu) : TReferenceCountableAutoDelete(), mGPUTextureManagerInfo(gpu) {}

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
	mInternals = new Internals(gpu);
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
CGPUTextureReference CGPUTextureManager::gpuTextureReference(const CBitmap& bitmap, const OV<CString>& reference,
		ReferenceOptions referenceOptions)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if we have a reference
	if (reference.hasValue())
		// Look for file in already loaded render material images
		for (TIteratorD<CGPUTextureReference> iterator =
						mInternals->mGPUTextureManagerInfo.mGPUTextureReferences.getIterator();
				iterator.hasValue(); iterator.advance()) {
			// Check this one
			if (iterator->getReference() == reference)
				// Found existing
				return *iterator;
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
CGPUTextureReference CGPUTextureManager::gpuTextureReference(const CBitmap& bitmap,
		CGPUTexture::DataFormat gpuTextureDataFormat, const OV<CString>& reference, ReferenceOptions referenceOptions)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if we have a reference
	if (reference.hasValue())
		// Look for file in already loaded render material images
		for (TIteratorD<CGPUTextureReference> iterator =
						mInternals->mGPUTextureManagerInfo.mGPUTextureReferences.getIterator();
				iterator.hasValue(); iterator.advance()) {
			// Check this one
			if (iterator->getReference() == reference)
				// Found existing
				return *iterator;
		}

	// Create new
	CGPULoadableTextureReferenceInternals*	gpuLoadableTextureReferenceInternals =
													new CBitmapGPUTextureReferenceInternals(bitmap, reference,
															OV<CGPUTexture::DataFormat>(gpuTextureDataFormat),
															referenceOptions, mInternals->mGPUTextureManagerInfo);
	CGPUTextureReference					textureReference(*gpuLoadableTextureReferenceInternals);
	mInternals->mGPUTextureManagerInfo.mGPUTextureReferences += textureReference;

	// Load
	gpuLoadableTextureReferenceInternals->loadOrQueueForLoading();

	return textureReference;
}

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureReference CGPUTextureManager::gpuTextureReference(const I<CDataSource>& dataSource, BitmapProc bitmapProc,
		const OV<CString>& reference, ReferenceOptions referenceOptions)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if we have a reference
	if (reference.hasValue())
		// Look for file in already loaded render material images
		for (TIteratorD<CGPUTextureReference> iterator =
						mInternals->mGPUTextureManagerInfo.mGPUTextureReferences.getIterator();
				iterator.hasValue(); iterator.advance()) {
			// Check this one
			if (iterator->getReference() == reference)
				// Found existing
				return *iterator;
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
		CGPUTexture::DataFormat gpuTextureDataFormat, const OV<CString>& reference, ReferenceOptions referenceOptions)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if we have a reference
	if (reference.hasValue())
		// Look for file in already loaded render material images
		for (TIteratorD<CGPUTextureReference> iterator =
						mInternals->mGPUTextureManagerInfo.mGPUTextureReferences.getIterator();
				iterator.hasValue(); iterator.advance()) {
			// Check this one
			if (iterator->getReference() == reference)
				// Found existing
				return *iterator;
		}

	// Create new
	CGPULoadableTextureReferenceInternals*	gpuLoadableTextureReferenceInternals =
													new CBitmapProcGPUTextureReferenceInternals(dataSource,
															bitmapProc, reference,
															OV<CGPUTexture::DataFormat>(gpuTextureDataFormat),
															referenceOptions,
															mInternals->mGPUTextureManagerInfo);
	CGPUTextureReference					textureReference(*gpuLoadableTextureReferenceInternals);
	mInternals->mGPUTextureManagerInfo.mGPUTextureReferences += textureReference;

	// Load
	gpuLoadableTextureReferenceInternals->loadOrQueueForLoading();

	return textureReference;
}

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureReference CGPUTextureManager::gpuTextureReference(const I<CDataSource>& dataSource, S2DSizeU16 dimensions,
		CGPUTexture::DataFormat gpuTextureDataFormat, const OV<CString>& reference, ReferenceOptions referenceOptions)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if we have a reference
	if (reference.hasValue())
		// Look for file in already loaded render material images
		for (TIteratorD<CGPUTextureReference> iterator =
						mInternals->mGPUTextureManagerInfo.mGPUTextureReferences.getIterator();
				iterator.hasValue(); iterator.advance()) {
			// Check this one
			if (iterator->getReference() == reference)
				// Found existing
				return *iterator;
		}

	// Create new
	CGPULoadableTextureReferenceInternals*	gpuLoadableTextureReferenceInternals =
													new CDataGPUTextureReferenceInternals(dataSource,
															gpuTextureDataFormat, dimensions, reference,
															referenceOptions, mInternals->mGPUTextureManagerInfo);
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
	CGPUTextureReference::Internals*	internals =
												new CGPUTextureReference::Internals(gpuTexture,
														mInternals->mGPUTextureManagerInfo);
	CGPUTextureReference				textureReference(*internals);
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
		iterator->load();
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
		iterator->unload();
}
