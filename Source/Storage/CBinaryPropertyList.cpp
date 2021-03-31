//----------------------------------------------------------------------------------------------------------------------
//	CBinaryPropertyList.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CBinaryPropertyList.h"

#include "CppToolboxAssert.h"
#include "TBuffer.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

struct SObjectOffset {
	// Properties
	UInt64	mOffset;
};

struct SBinaryPListTrailer {
	// Properties
	UInt8	mUnused[6];
	UInt8	mObjectOffsetFieldSize;
	UInt8	mObjectIndexFieldSize;
	UInt64	mTotalObjectCount;
	UInt64	mTopObjectIndex;
	UInt64	mObjectOffsetTableOffset;
};

const	UInt8	kMarkerTypeArray			= 0xA0;
const	UInt8	kMarkerTypeBooleanTrue		= 0x09;
const	UInt8	kMarkerTypeBooleanFalse		= 0x08;
const	UInt8	kMarkerTypeData				= 0x40;
const	UInt8	kMarkerTypeDictionary		= 0xD0;
const	UInt8	kMarkerTypeFloat32			= 0x22;
const	UInt8	kMarkerTypeFloat64			= 0x23;
const	UInt8	kMarkerTypeInteger1Byte		= 0x10;
const	UInt8	kMarkerTypeInteger2Bytes	= 0x11;
const	UInt8	kMarkerTypeInteger4Bytes	= 0x12;
const	UInt8	kMarkerTypeInteger8Bytes	= 0x13;
const	UInt8	kMarkerTypeStringASCII		= 0x50;
const	UInt8	kMarkerTypeStringUnicode16	= 0x60;
const	UInt8	kMarkerCountMask			= 0x0F;

static	CData	sBinaryPListV10Header((UInt8*) "bplist00", 8, false);

static	CString	sErrorDomain(OSSTR("CBinaryPropertyList"));
static	SError	sUnknownFormatError(sErrorDomain, 1, CString(OSSTR("Unknown format")));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBPLDataSource declaration

class CBPLDataSource : public TReferenceCountable<CBPLDataSource> {
	public:
							// Lifecycle methods
							CBPLDataSource(const CByteParceller& byteParceller, UInt8 objectOffsetFieldSize,
									UInt8 objectIndexFieldSize, UInt64 totalObjectCount,
									UInt64 objectOffsetTableOffset);
							~CBPLDataSource();

							// Instance methods
		void				readData(SInt64 offset, void* buffer, UInt64 byteCount, const char* errorWhen);
		UInt8				readMarker(UInt64 objectIndex, OR<UInt64> count = OR<UInt64>());
		UInt64				readIndex();
		UInt64				readCount(const char* errorWhen);

		UInt64				readValue(SInt64 offset, UInt8 fieldSize, const char* errorWhen);
		UInt64				readValue(UInt8 fieldSize, const char* errorWhen);
		CDictionary			readDictionary(UInt64 objectIndex);

		CDictionary::Value*	createDictionaryValue(UInt64 objectIndex);

		CByteParceller	mByteParceller;
		OI<SError>		mError;

		UInt8			mObjectIndexFieldSize;
		UInt64			mTotalObjectCount;
		UInt64*			mObjectOffsets;
		CString**		mStrings;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBPLDictionaryInfo

class CBPLDictionaryInfo {
	public:
										// Lifecycle methods
										CBPLDictionaryInfo(CBPLDataSource& bplDataSource,
												CDictionary::KeyCount keyCount) :
											mBPLDataSource(bplDataSource), mKeyCount(keyCount)
											{
												// Setup
												mKeyHashes = new UInt32[mKeyCount];
												mKeys = new CString*[mKeyCount];
												mValueIndexes = new UInt64[mKeyCount];

												mDictionaryValues = new CDictionary::Value*[mKeyCount];
												::memset(mDictionaryValues, 0, sizeof(CDictionary::Value*) * mKeyCount);

												// Setup keys
												for (CDictionary::KeyCount i = 0; i < mKeyCount; i++) {
													// Setup key
													UInt64	objectIndex = mBPLDataSource.readIndex();
													mKeys[i] = mBPLDataSource.mStrings[objectIndex];
													mKeyHashes[i] = CHasher::getValueForHashable(*mKeys[i]);
												}

												// Setup values
												for (CDictionary::KeyCount i = 0; i < mKeyCount; i++)
													// Read value index
													mValueIndexes[i] = mBPLDataSource.readIndex();

												// Add reference
												mBPLDataSource.addReference();
											}
										~CBPLDictionaryInfo()
											{
												// Cleanup
												DeleteArray(mKeyHashes);
												DeleteArray(mKeys);
												DeleteArray(mValueIndexes);

												for (CDictionary::KeyCount i = 0; i < mKeyCount; i++) {
													// Check if have value
													if (mDictionaryValues[i] != nil) {
														// Dispose
														mDictionaryValues[i]->dispose(nil);
														Delete(mDictionaryValues[i]);
													}
												}
												DeleteArray(mDictionaryValues);

												mBPLDataSource.removeReference();
											}

										// Class methods
		static	CDictionary				dictionaryWith(CBPLDataSource& bplDataSource, CDictionary::KeyCount keyCount)
											{
												return CDictionary(
														CDictionary::Procs(
																(CDictionary::Procs::GetKeyCountProc) getKeyCount,
																(CDictionary::Procs::GetKeyAtIndexProc) getKeyAtIndex,
																(CDictionary::Procs::GetValueProc) getValue,
																nil, nil, nil,
																(CDictionary::Procs::DisposeUserDataProc)
																		disposeUserData,
																new CBPLDictionaryInfo(bplDataSource, keyCount)));
											}
		static	CDictionary::KeyCount	getKeyCount(CBPLDictionaryInfo* bplDictionaryInfo)
											{ return bplDictionaryInfo->mKeyCount; }
		static	CString					getKeyAtIndex(UInt32 index, CBPLDictionaryInfo* bplDictionaryInfo)
											{ return *bplDictionaryInfo->mKeys[index]; }
		static	OR<CDictionary::Value>	getValue(const CString& key, CBPLDictionaryInfo* bplDictionaryInfo)
											{
												// Iterate keys
												UInt32	keyHash = CHasher::getValueForHashable(key);
												for (CDictionary::KeyCount i = 0;
														i < bplDictionaryInfo->mKeyCount; i++) {
													// Compare this key
													if ((bplDictionaryInfo->mKeyHashes[i] == keyHash) &&
															(*bplDictionaryInfo->mKeys[i] == key)) {
														// Found
														if (bplDictionaryInfo->mDictionaryValues[i] == nil)
															// Construct value
															bplDictionaryInfo->mDictionaryValues[i] =
																	bplDictionaryInfo->mBPLDataSource
																			.createDictionaryValue(
																					bplDictionaryInfo->
																							mValueIndexes[i]);

														return OR<CDictionary::Value>(
																*bplDictionaryInfo->mDictionaryValues[i]);
													}
												}

												return OR<CDictionary::Value>();
											}
		static	void					disposeUserData(CBPLDictionaryInfo* bplDictionaryInfo)
											{ Delete(bplDictionaryInfo); }

		CBPLDataSource&			mBPLDataSource;
		CDictionary::KeyCount	mKeyCount;
		UInt32*					mKeyHashes;
		CString**				mKeys;
		UInt64*					mValueIndexes;
		CDictionary::Value**	mDictionaryValues;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBPLDataSource definition

//----------------------------------------------------------------------------------------------------------------------
CBPLDataSource::CBPLDataSource(const CByteParceller& byteParceller, UInt8 objectOffsetFieldSize,
		UInt8 objectIndexFieldSize, UInt64 totalObjectCount, UInt64 objectOffsetTableOffset) :
	TReferenceCountable(), mByteParceller(byteParceller), mObjectIndexFieldSize(objectIndexFieldSize),
			mTotalObjectCount(totalObjectCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mObjectOffsets = new UInt64[mTotalObjectCount];

	mStrings = new CString*[mTotalObjectCount];
	::memset(mStrings, 0, sizeof(CString*) * mTotalObjectCount);

	SInt64	offsetPosition = objectOffsetTableOffset;
	for (UInt64 i = 0; i < mTotalObjectCount; i++, offsetPosition += objectOffsetFieldSize) {
		// Update storage
		mObjectOffsets[i] = readValue(offsetPosition, objectOffsetFieldSize, "reading objects");

		// Read info
		UInt64	count;
		UInt8	marker = readMarker(i, OR<UInt64>(count));
		if (mError.hasInstance())
			// Error
			continue;

		// Validate marker
		if ((marker != kMarkerTypeStringASCII) && (marker != kMarkerTypeStringUnicode16))
			// Error
			continue;

		// Get string content
		OI<CData>	data = mByteParceller.readData(count, mError);
		if (mError.hasInstance())
			// Error
			continue;

		// Create string
		mStrings[i] =
				new CString(*data,
						(marker == kMarkerTypeStringASCII) ? CString::kEncodingASCII : CString::kEncodingUTF16BE);
	}
}

//----------------------------------------------------------------------------------------------------------------------
CBPLDataSource::~CBPLDataSource()
//----------------------------------------------------------------------------------------------------------------------
{
	DeleteArray(mObjectOffsets);

	for (UInt64 i = 0; i < mTotalObjectCount; i++) {
		Delete(mStrings[i]);
	}
	DeleteArray(mStrings);
}

//----------------------------------------------------------------------------------------------------------------------
void CBPLDataSource::readData(SInt64 offset, void* buffer, UInt64 byteCount, const char* errorWhen)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set position
	mError = mByteParceller.setPos(CDataSource::kPositionFromBeginning, offset);
	LogIfErrorAndReturn(mError, errorWhen);

	// Read data
	mError = mByteParceller.readData(buffer, byteCount);
	LogIfError(mError, errorWhen);
}

//----------------------------------------------------------------------------------------------------------------------
UInt8 CBPLDataSource::readMarker(UInt64 objectIndex, OR<UInt64> count)
//----------------------------------------------------------------------------------------------------------------------
{
	// Read marker
	UInt8	marker = 0;
	readData(mObjectOffsets[objectIndex], &marker, 1, "reading marker");

	// Check marker
	switch (marker) {
		case kMarkerTypeBooleanTrue:
		case kMarkerTypeBooleanFalse:
		case kMarkerTypeFloat32:
		case kMarkerTypeFloat64:
		case kMarkerTypeInteger1Byte:
		case kMarkerTypeInteger2Bytes:
		case kMarkerTypeInteger4Bytes:
		case kMarkerTypeInteger8Bytes:
			// No count
			return marker;

		default:
			// May have count
			if (count.hasReference()) {
				// Read count
				*count = marker & kMarkerCountMask;
				if (*count == 15)
					// Read count
					*count = readCount("reading marker");
			}

			return marker & ~kMarkerCountMask;
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CBPLDataSource::readIndex()
//----------------------------------------------------------------------------------------------------------------------
{
	// Read index
	return readValue(mObjectIndexFieldSize, "reading index");
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CBPLDataSource::readCount(const char* errorWhen)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get count marker
	OV<UInt8>	countMarker = mByteParceller.readUInt8(mError);
	LogIfErrorAndReturnValue(mError, errorWhen, 0);

	UInt8	fieldSize;
	switch (*countMarker) {
		case kMarkerTypeInteger1Byte:	fieldSize = 1;	break;
		case kMarkerTypeInteger2Bytes:	fieldSize = 2;	break;
		case kMarkerTypeInteger4Bytes:	fieldSize = 4;	break;
		case kMarkerTypeInteger8Bytes:	fieldSize = 8;	break;
		default:						fieldSize = 1;	break;
	}

	return readValue(fieldSize, errorWhen);
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CBPLDataSource::readValue(SInt64 offset, UInt8 fieldSize, const char* errorWhen)
//----------------------------------------------------------------------------------------------------------------------
{
	// Read value
	UInt64	storedValue = 0;
	readData(offset, (UInt8*) &storedValue + sizeof(UInt64) - fieldSize, fieldSize, errorWhen);
	ReturnValueIfError(mError, 0);

	return EndianU64_BtoN(storedValue);
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CBPLDataSource::readValue(UInt8 fieldSize, const char* errorWhen)
//----------------------------------------------------------------------------------------------------------------------
{
	// Read value
	UInt64	storedValue = 0;
	mError = mByteParceller.readData((UInt8*) &storedValue + sizeof(UInt64) - fieldSize, fieldSize);
	LogIfErrorAndReturnValue(mError, errorWhen, 0);

	return EndianU64_BtoN(storedValue);
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary CBPLDataSource::readDictionary(UInt64 objectIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Read marker
	UInt64	count;
	readMarker(objectIndex, OR<UInt64>(count));

	return CBPLDictionaryInfo::dictionaryWith(*this, (CDictionary::KeyCount) count);
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary::Value* CBPLDataSource::createDictionaryValue(UInt64 objectIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check for string
	if (mStrings[objectIndex] != nil)
		// Is a string
		return new CDictionary::Value(*mStrings[objectIndex]);

	// Read marker and count
	UInt64	count;
	UInt8	marker = readMarker(objectIndex, OR<UInt64>(count));

	// What's our marker?
	switch (marker) {
		case kMarkerTypeArray: {
			// Array
			if (count == 0)
				return nil;

			// Read object indexes
			TBuffer<UInt64>	objectIndexes((UInt32) count);
			for (UInt64 i = 0; i < count; i++)
				(*objectIndexes)[i] = readIndex();

			// Determine array type
			marker = readMarker(objectIndexes[0]);
			switch (marker) {
				case kMarkerTypeArray:
					// Array (unimplemented)
AssertFailUnimplemented();

				case kMarkerTypeBooleanTrue:
				case kMarkerTypeBooleanFalse:
					// Boolean (unimplemented)
AssertFailUnimplemented();

				case kMarkerTypeData:
					// Data (unimplemented)
AssertFailUnimplemented();

				case kMarkerTypeDictionary: {
					// Dictionary
					TNArray<CDictionary>	array;
					for (UInt64 i = 0; i < count; i++)
						// Add dictionary
						array.add(readDictionary(objectIndexes[(UInt32) i]));

					return new CDictionary::Value(array);
				}

				case kMarkerTypeFloat32:
				case kMarkerTypeFloat64:
					// Float (unimplemented)
AssertFailUnimplemented();

				case kMarkerTypeInteger1Byte:
				case kMarkerTypeInteger2Bytes:
				case kMarkerTypeInteger4Bytes:
				case kMarkerTypeInteger8Bytes:
					// Integer (unimplemented)
AssertFailUnimplemented();

				case kMarkerTypeStringASCII:
				case kMarkerTypeStringUnicode16: {
					// String
					TNArray<CString>	array;
					for (UInt64 i = 0; i < count; i++)
						// Add string
						array.add(*mStrings[objectIndexes[(UInt32) i]]);

					return new CDictionary::Value(array);
				}
			}
		}

		case kMarkerTypeBooleanTrue:
			// True
			return new CDictionary::Value(true);

		case kMarkerTypeBooleanFalse:
			// False
			return new CDictionary::Value(false);

		case kMarkerTypeData:
			// Data
// TODO
AssertFailUnimplemented();

		case kMarkerTypeDictionary:
			// Dictionary
			return new CDictionary::Value(readDictionary(objectIndex));

		case kMarkerTypeFloat32: {
			// Float32
			SwappableFloat32	swappableFloat32;
			swappableFloat32.mStoredValue = (UInt32) readValue(4, "reading float32");

			return new CDictionary::Value(swappableFloat32.mValue);
			} break;

		case kMarkerTypeFloat64: {
			// Float64
			SwappableFloat64	swappableFloat64;
			swappableFloat64.mStoredValue = readValue(8, "reading float64");

			return new CDictionary::Value(swappableFloat64.mValue);
			} break;

		case kMarkerTypeInteger1Byte:
			// Integer, 1 byte
			return new CDictionary::Value((SInt8) readValue(1, "reading integer, 1 byte"));

		case kMarkerTypeInteger2Bytes:
			// Integer, 2 bytes
			return new CDictionary::Value((SInt16) readValue(2, "reading integer, 2 bytes"));

		case kMarkerTypeInteger4Bytes:
			// Integer, 4 bytes
			return new CDictionary::Value((SInt32) readValue(4, "reading integer, 4 bytes"));

		case kMarkerTypeInteger8Bytes:
			// Integer, 8 bytes
			return new CDictionary::Value((SInt64) readValue(8, "reading integer, 8 bytes"));

		default:
			// We should never get here
			AssertFail();
	}

	return nil;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBinaryPropertyList

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CBinaryPropertyList::DictionaryResult CBinaryPropertyList::dictionaryFrom(const CByteParceller& byteParceller)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt64		dataSize = byteParceller.getSize();
	OI<SError>	error;

	// Check size
	if (dataSize < (sBinaryPListV10Header.getSize() + sizeof(SBinaryPListTrailer))) {
		// Too small to be a binary property list
		DictionaryResult	dictionaryResult(sUnknownFormatError);
		LogIfErrorAndReturnValue(dictionaryResult.mError, "checking data source size", dictionaryResult);
	}

	// Validate header
	OI<CData>	header = byteParceller.readData(sBinaryPListV10Header.getSize(), error);
	LogIfErrorAndReturnValue(error, "reading header", DictionaryResult(*error));
	if (*header != sBinaryPListV10Header) {
		// Header does not match
		DictionaryResult	dictionaryResult(sUnknownFormatError);
		LogIfErrorAndReturnValue(dictionaryResult.mError, "validating header", dictionaryResult);
	}

	// Validate trailer
	error = byteParceller.setPos(CDataSource::kPositionFromEnd, sizeof(SBinaryPListTrailer));
	LogIfErrorAndReturnValue(error, "positiong data source to read trailer", DictionaryResult(*error));

	SBinaryPListTrailer	trailer;
	error = byteParceller.readData(&trailer, sizeof(SBinaryPListTrailer));
	LogIfErrorAndReturnValue(error, "reading trailer", DictionaryResult(*error));

	// Create CBPLDataSource
	UInt8			objectOffsetFieldSize = trailer.mObjectOffsetFieldSize;
	UInt8			objectIndexFieldSize = trailer.mObjectIndexFieldSize;
	UInt64			totalObjectCount = EndianU64_BtoN(trailer.mTotalObjectCount);
	UInt64			topObjectIndex = EndianU64_BtoN(trailer.mTopObjectIndex);
	UInt64			objectOffsetTableOffset = EndianU64_BtoN(trailer.mObjectOffsetTableOffset);
	CBPLDataSource*	bplDataSource =
							new CBPLDataSource(byteParceller, objectOffsetFieldSize, objectIndexFieldSize,
									totalObjectCount, objectOffsetTableOffset);

	// Get top level object
	UInt64	count;
	UInt8	marker = bplDataSource->readMarker(topObjectIndex, OR<UInt64>(count));
	if (marker != kMarkerTypeDictionary) {
		// Top object is not a dictionary
		Delete(bplDataSource);
		DictionaryResult	dictionaryResult(sUnknownFormatError);
		LogIfErrorAndReturnValue(dictionaryResult.mError, "top object is not a dictionary", dictionaryResult);
	}

	// Create dictionary
	CDictionary	dictionary = bplDataSource->readDictionary(topObjectIndex);

	// Remove reference
	bplDataSource->removeReference();

	return DictionaryResult(dictionary);
}

////----------------------------------------------------------------------------------------------------------------------
//OI<SError> CBinaryPropertyList::writeTo(const CFile& file)
////----------------------------------------------------------------------------------------------------------------------
//{
//AssertFailUnimplemented();
//return OI<SError>();
//}
