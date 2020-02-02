//----------------------------------------------------------------------------------------------------------------------
//	CByteParceller.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CByteParceller.h"

#include "CData.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CByteParcellerInternals

class CByteParcellerInternals : public TReferenceCountable<CByteParcellerInternals> {
	public:
		CByteParcellerInternals(const CDataSource* dataSource) : TReferenceCountable(), mDataSource(dataSource) {}
		~CByteParcellerInternals()
			{ DisposeOf(mDataSource); }

		const	CDataSource*	mDataSource;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CByteParceller

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CByteParceller::CByteParceller(const CDataSource* dataSource)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CByteParcellerInternals(dataSource);
}

//----------------------------------------------------------------------------------------------------------------------
CByteParceller::CByteParceller(const CByteParceller& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CByteParceller::~CByteParceller()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UInt64 CByteParceller::getSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mDataSource->getSize();
}

//----------------------------------------------------------------------------------------------------------------------
UError CByteParceller::readData(void* buffer, UInt64 byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mDataSource->readData(buffer, byteCount);
}

//----------------------------------------------------------------------------------------------------------------------
CData CByteParceller::readData(UInt64 byteCount, UError& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CData	data((CDataSize) byteCount);

	// Read
	outError = readData(data.getMutableBytePtr(), byteCount);

	return (outError == kNoError) ? data : CData::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
CData CByteParceller::readData(UError& outError) const
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
SInt64 CByteParceller::getPos() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mDataSource->getPos();
}

//----------------------------------------------------------------------------------------------------------------------
UError CByteParceller::setPos(EDataSourcePosition position, SInt64 newPos) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mDataSource->setPos(position, newPos);
}

//----------------------------------------------------------------------------------------------------------------------
void CByteParceller::reset() const
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mDataSource->reset();
}
