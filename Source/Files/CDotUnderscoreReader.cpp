//----------------------------------------------------------------------------------------------------------------------
//	CDotUnderscoreReader.cpp			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDotUnderscoreReader.h"

#include "CByteReader.h"

//----------------------------------------------------------------------------------------------------------------------
// Info from http://www.swiftforensics.com/2018/11/the-dot-underscore-file-format.html

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDotUnderscoreReader::Attribute

class CDotUnderscoreReader::Attribute {};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDotUnderscoreReader::Internals

class CDotUnderscoreReader::Internals {
	public:
		Internals(const TNDictionary<TNArray<Attribute> >& attributeMap, const OV<CData>& resourceFork) :
			mAttributeMap(attributeMap), mResourceFork(resourceFork)
			{}

		TNDictionary<TNArray<Attribute> >	mAttributeMap;
		OV<CData>							mResourceFork;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local data

#pragma pack(push, 1)

struct SDotUnderscoreHeader {
			// Methods
	UInt32	getMagic() const { return EndianU32_BtoN(mMagic); }
	UInt16	getVersion() const { return EndianU16_BtoN(mVersion); }

	// Properties (in storage endian)
	private:
		UInt32	mMagic;
		UInt16	mVersion;
		UInt16	mReserved;
		SInt8	mSource[16];
		UInt16	mUnknown1;
		UInt32	mUnknown2;
		UInt32	mHeaderByteCount;
		UInt32	mDataByteCount;
		UInt32	mUnknown3;
		UInt32	mLogicalFileByteCount;
		UInt32	mUnusedFileByteCount;
};

struct SDotUnderscoreMystery {

	// Properties (in storage endian)
	private:
		UInt8	mUnknown[34];
};

struct SDotUnderscoreATTR {
			// Methods
	UInt32	getResourceForkOffset() const { return EndianU32_BtoN(mResourceForkOffset); }

	// Properties (in storage endian)
	private:
		OSType	mATTR;
		UInt32	mUnknown1;
		UInt32	mResourceForkOffset;
		UInt32	mUnknown2;
		UInt32	mUnknown3;
		UInt32	mUnknown4;
		UInt32	mUnknown5;
		UInt32	mUnknown6;
		UInt16	mUnknown7;
};

#pragma pack(pop)

static	CString	sErrorDomain(OSSTR("CDotUnderscoreReader"));
static	SError	sInvalidData(sErrorDomain, 1, CString(OSSTR("Invalid Data")));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDotUnderscoreReader

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CDotUnderscoreReader::CDotUnderscoreReader(const TNDictionary<TNArray<Attribute> >& attributeMap,
		const OV<CData>& resourceFork)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals(attributeMap, resourceFork);
}

//----------------------------------------------------------------------------------------------------------------------
CDotUnderscoreReader::~CDotUnderscoreReader()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
OR<CData> CDotUnderscoreReader::getResourceFork() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mResourceFork.hasValue() ? OR<CData>((CData&) *mInternals->mResourceFork) : OR<CData>();
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TIResult<CDotUnderscoreReader> CDotUnderscoreReader::from(const I<CRandomAccessDataSource>& randomAccessDataSource)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CByteReader	byteReader(randomAccessDataSource, true);
	OV<SError>	error;

	// Read header
	if (byteReader.getByteCount() < sizeof(SDotUnderscoreHeader))
		// Not big enough to read header
		return TIResult<CDotUnderscoreReader>(sInvalidData);

	TVResult<CData>		data = byteReader.readData(sizeof(SDotUnderscoreHeader));
	ReturnValueIfResultError(data, TIResult<CDotUnderscoreReader>(data.getError()));

	const	SDotUnderscoreHeader&	header = *((const SDotUnderscoreHeader*) data->getBytePtr());
	if ((header.getMagic() != 0x00051607) || (header.getVersion() != 2))
		// Don't know how to read this data
		return TIResult<CDotUnderscoreReader>(sInvalidData);

	// Read mystery
	data = byteReader.readData(sizeof(SDotUnderscoreMystery));
	ReturnValueIfResultError(data, TIResult<CDotUnderscoreReader>(data.getError()));

	// Read ATTR
	data = byteReader.readData(sizeof(SDotUnderscoreATTR));
	ReturnValueIfResultError(data, TIResult<CDotUnderscoreReader>(data.getError()));

	const	SDotUnderscoreATTR&	attr = *((const SDotUnderscoreATTR*) data->getBytePtr());
	UInt32	resourceForkOffset = attr.getResourceForkOffset();

	// Read attributes
	TNDictionary<TNArray<Attribute> >	attributeMap;

	// Get resource fork
	error = byteReader.setPos(CByteReader::kPositionFromBeginning, resourceForkOffset);
	ReturnValueIfError(error, TIResult<CDotUnderscoreReader>(*error));

	data = byteReader.readData(byteReader.getByteCount() - resourceForkOffset);
	ReturnValueIfResultError(data, TIResult<CDotUnderscoreReader>(data.getError()));

	return TIResult<CDotUnderscoreReader>(
			OI<CDotUnderscoreReader>(new CDotUnderscoreReader(attributeMap, OV<CData>(*data))));
}
