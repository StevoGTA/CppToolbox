//----------------------------------------------------------------------------------------------------------------------
//	CDataSource.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDataSource.h"

#include "CData.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDataSourceInternals

class CDataSourceInternals {
	public:
								CDataSourceInternals(const CDataProvider* dataProvider) :
									mDataProvider(dataProvider), mReferenceCount(1)
									{}
								~CDataSourceInternals()
									{
										DisposeOf(mDataProvider);
									}

		CDataSourceInternals*	addReference()
									{ mReferenceCount++; return this; }
		void					removeReference()
									{
										// Decrement reference count and check if we are the last one
										if (--mReferenceCount == 0) {
											// We going away
											CDataSourceInternals*	THIS = this;
											DisposeOf(THIS);
										}
									}

		const	CDataProvider*	mDataProvider;
				UInt32			mReferenceCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDataSource

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CDataSource::CDataSource(const CDataProvider* dataProvider)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CDataSourceInternals(dataProvider);
}

//----------------------------------------------------------------------------------------------------------------------
CDataSource::CDataSource(const CDataSource& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CDataSource::~CDataSource()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UInt64 CDataSource::getSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mDataProvider->getSize();
}

//----------------------------------------------------------------------------------------------------------------------
UError CDataSource::readData(void* buffer, UInt64 byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mDataProvider->readData(buffer, byteCount);
}

//----------------------------------------------------------------------------------------------------------------------
CData CDataSource::readData(UInt64 byteCount, UError& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CData	data((CDataSize) byteCount);

	// Read
	outError = readData(data.getMutableBytePtr(), byteCount);

	return (outError == kNoError) ? data : CData::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
CData CDataSource::readData(UError& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt64	byteCount = getSize() - getPos();
	CData	data((CDataSize) byteCount);

	// Read
	outError = readData(data.getMutableBytePtr(), byteCount);

	return (outError == kNoError) ? data : CData::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CDataSource::getPos() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mDataProvider->getPos();
}

//----------------------------------------------------------------------------------------------------------------------
UError CDataSource::setPos(EDataProviderPosition position, SInt64 newPos) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mDataProvider->setPos(position, newPos);
}

//----------------------------------------------------------------------------------------------------------------------
void CDataSource::reset() const
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mDataProvider->reset();
}
