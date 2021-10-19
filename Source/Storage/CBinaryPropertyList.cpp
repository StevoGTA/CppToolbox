//----------------------------------------------------------------------------------------------------------------------
//	CBinaryPropertyList.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CBinaryPropertyList.h"

#include "CByteReader.h"
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
// MARK: - CBPLReader declaration

class CBPLReader : public TReferenceCountable<CBPLReader> {
	public:
					// Lifecycle methods
					CBPLReader(const I<CSeekableDataSource>& seekableDataSource, UInt8 objectOffsetFieldSize,
							UInt8 objectIndexFieldSize, UInt64 totalObjectCount, UInt64 objectOffsetTableOffset);
					~CBPLReader();

					// Instance methods
		void		readData(SInt64 offset, void* buffer, UInt64 byteCount, const char* errorWhen);
		UInt8		readMarker(UInt64 objectIndex, OR<UInt64> count = OR<UInt64>());
		UInt64		readIndex();
		UInt64		readCount(const char* errorWhen);

		UInt64		readValue(SInt64 offset, UInt8 fieldSize, const char* errorWhen);
		UInt64		readValue(UInt8 fieldSize, const char* errorWhen);
		CDictionary	readDictionary(UInt64 objectIndex);

		SValue*		createDictionaryValue(UInt64 objectIndex);

		// Properties
		CByteReader	mByteReader;
		OI<SError>	mError;

		UInt8		mObjectIndexFieldSize;
		UInt64		mTotalObjectCount;
		UInt64*		mObjectOffsets;
		CString**	mStrings;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBPLDictionaryInfo

class CBPLDictionaryInfo {
	public:
										// Lifecycle methods
										CBPLDictionaryInfo(CBPLReader& bplReader, CDictionary::KeyCount keyCount) :
											mBPLReader(bplReader), mKeyCount(keyCount)
											{
												// Setup
												mKeyHashes = new UInt32[mKeyCount];
												mKeys = new CString*[mKeyCount];
												mValueIndexes = new UInt64[mKeyCount];

												mDictionaryValues = new SValue*[mKeyCount];
												::memset(mDictionaryValues, 0, sizeof(SValue*) * mKeyCount);

												// Setup keys
												for (CDictionary::KeyCount i = 0; i < mKeyCount; i++) {
													// Setup key
													UInt64	objectIndex = mBPLReader.readIndex();
													mKeys[i] = mBPLReader.mStrings[objectIndex];
													mKeyHashes[i] = CHasher::getValueForHashable(*mKeys[i]);
												}

												// Setup values
												for (CDictionary::KeyCount i = 0; i < mKeyCount; i++)
													// Read value index
													mValueIndexes[i] = mBPLReader.readIndex();

												// Add reference
												mBPLReader.addReference();
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

												mBPLReader.removeReference();
											}

										// Class methods
		static	CDictionary				dictionaryWith(CBPLReader& bplDataSource, CDictionary::KeyCount keyCount)
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
		static	OR<SValue>				getValue(const CString& key, CBPLDictionaryInfo* bplDictionaryInfo)
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
																	bplDictionaryInfo->mBPLReader
																			.createDictionaryValue(
																					bplDictionaryInfo->
																							mValueIndexes[i]);

														return OR<SValue>(
																*bplDictionaryInfo->mDictionaryValues[i]);
													}
												}

												return OR<SValue>();
											}
		static	void					disposeUserData(CBPLDictionaryInfo* bplDictionaryInfo)
											{ Delete(bplDictionaryInfo); }

		CBPLReader&				mBPLReader;
		CDictionary::KeyCount	mKeyCount;
		UInt32*					mKeyHashes;
		CString**				mKeys;
		UInt64*					mValueIndexes;
		SValue**				mDictionaryValues;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBPLReader definition

//----------------------------------------------------------------------------------------------------------------------
CBPLReader::CBPLReader(const I<CSeekableDataSource>& seekableDataSource, UInt8 objectOffsetFieldSize,
		UInt8 objectIndexFieldSize, UInt64 totalObjectCount, UInt64 objectOffsetTableOffset) :
	TReferenceCountable(), mByteReader(seekableDataSource, true), mObjectIndexFieldSize(objectIndexFieldSize),
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
		TIResult<CData>	dataResult = mByteReader.readData(count);
		if (dataResult.hasError()) {
			// Error
			mError = OI<SError>(dataResult.getError());
			continue;
		}

		// Create string
		mStrings[i] =
				new CString(dataResult.getValue(),
						(marker == kMarkerTypeStringASCII) ? CString::kEncodingASCII : CString::kEncodingUTF16BE);
	}
}

//----------------------------------------------------------------------------------------------------------------------
CBPLReader::~CBPLReader()
//----------------------------------------------------------------------------------------------------------------------
{
	DeleteArray(mObjectOffsets);

	for (UInt64 i = 0; i < mTotalObjectCount; i++) {
		Delete(mStrings[i]);
	}
	DeleteArray(mStrings);
}

//----------------------------------------------------------------------------------------------------------------------
void CBPLReader::readData(SInt64 offset, void* buffer, UInt64 byteCount, const char* errorWhen)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set position
	mError = mByteReader.setPos(CByteReader::kPositionFromBeginning, offset);
	LogIfErrorAndReturn(mError, errorWhen);

	// Read data
	mError = mByteReader.readData(buffer, byteCount);
	LogIfError(mError, errorWhen);
}

//----------------------------------------------------------------------------------------------------------------------
UInt8 CBPLReader::readMarker(UInt64 objectIndex, OR<UInt64> count)
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
UInt64 CBPLReader::readIndex()
//----------------------------------------------------------------------------------------------------------------------
{
	// Read index
	return readValue(mObjectIndexFieldSize, "reading index");
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CBPLReader::readCount(const char* errorWhen)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get count marker
	TVResult<UInt8>	countMarker = mByteReader.readUInt8();
	if (countMarker.hasError()) mError = OI<SError>(countMarker.getError());
	LogIfErrorAndReturnValue(mError, errorWhen, 0);

	UInt8	fieldSize;
	switch (countMarker.getValue()) {
		case kMarkerTypeInteger1Byte:	fieldSize = 1;	break;
		case kMarkerTypeInteger2Bytes:	fieldSize = 2;	break;
		case kMarkerTypeInteger4Bytes:	fieldSize = 4;	break;
		case kMarkerTypeInteger8Bytes:	fieldSize = 8;	break;
		default:						fieldSize = 1;	break;
	}

	return readValue(fieldSize, errorWhen);
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CBPLReader::readValue(SInt64 offset, UInt8 fieldSize, const char* errorWhen)
//----------------------------------------------------------------------------------------------------------------------
{
	// Read value
	UInt64	storedValue = 0;
	readData(offset, (UInt8*) &storedValue + sizeof(UInt64) - fieldSize, fieldSize, errorWhen);
	ReturnValueIfError(mError, 0);

	return EndianU64_BtoN(storedValue);
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CBPLReader::readValue(UInt8 fieldSize, const char* errorWhen)
//----------------------------------------------------------------------------------------------------------------------
{
	// Read value
	UInt64	storedValue = 0;
	mError = mByteReader.readData((UInt8*) &storedValue + sizeof(UInt64) - fieldSize, fieldSize);
	LogIfErrorAndReturnValue(mError, errorWhen, 0);

	return EndianU64_BtoN(storedValue);
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary CBPLReader::readDictionary(UInt64 objectIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Read marker
	UInt64	count;
	readMarker(objectIndex, OR<UInt64>(count));

	return CBPLDictionaryInfo::dictionaryWith(*this, (CDictionary::KeyCount) count);
}

//----------------------------------------------------------------------------------------------------------------------
SValue* CBPLReader::createDictionaryValue(UInt64 objectIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check for string
	if (mStrings[objectIndex] != nil)
		// Is a string
		return new SValue(*mStrings[objectIndex]);

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

					return new SValue(array);
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

					return new SValue(array);
				}
			}
		}

		case kMarkerTypeBooleanTrue:
			// True
			return new SValue(true);

		case kMarkerTypeBooleanFalse:
			// False
			return new SValue(false);

		case kMarkerTypeData:
			// Data
// TODO
AssertFailUnimplemented();

		case kMarkerTypeDictionary:
			// Dictionary
			return new SValue(readDictionary(objectIndex));

		case kMarkerTypeFloat32: {
			// Float32
			SwappableFloat32	swappableFloat32;
			swappableFloat32.mStoredValue = (UInt32) readValue(4, "reading float32");

			return new SValue(swappableFloat32.mValue);
			} break;

		case kMarkerTypeFloat64: {
			// Float64
			SwappableFloat64	swappableFloat64;
			swappableFloat64.mStoredValue = readValue(8, "reading float64");

			return new SValue(swappableFloat64.mValue);
			} break;

		case kMarkerTypeInteger1Byte:
			// Integer, 1 byte
			return new SValue((SInt8) readValue(1, "reading integer, 1 byte"));

		case kMarkerTypeInteger2Bytes:
			// Integer, 2 bytes
			return new SValue((SInt16) readValue(2, "reading integer, 2 bytes"));

		case kMarkerTypeInteger4Bytes:
			// Integer, 4 bytes
			return new SValue((SInt32) readValue(4, "reading integer, 4 bytes"));

		case kMarkerTypeInteger8Bytes:
			// Integer, 8 bytes
			return new SValue((SInt64) readValue(8, "reading integer, 8 bytes"));

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
TIResult<CDictionary> CBinaryPropertyList::dictionaryFrom(const I<CSeekableDataSource>& seekableDataSource)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt64		byteCount = seekableDataSource->getByteCount();
	OI<SError>	error;

	// Check size
	if (byteCount < (sBinaryPListV10Header.getByteCount() + sizeof(SBinaryPListTrailer))) {
		// Too small to be a binary property list
		LogErrorAndReturnValue(sUnknownFormatError, "checking data source size",
				TIResult<CDictionary>(sUnknownFormatError));
	}

	// Validate header
	CData	data(sBinaryPListV10Header.getByteCount());
	error = seekableDataSource->readData(0, data.getMutableBytePtr(), sBinaryPListV10Header.getByteCount());
	LogIfErrorAndReturnValue(error, "reading header", TIResult<CDictionary>(*error));
	if (data != sBinaryPListV10Header) {
		// Header does not match
		LogErrorAndReturnValue(sUnknownFormatError, "validating header", TIResult<CDictionary>(sUnknownFormatError));
	}

	// Validate trailer
	SBinaryPListTrailer	trailer;
	error = seekableDataSource->readData(byteCount - sizeof(SBinaryPListTrailer), &trailer, sizeof(SBinaryPListTrailer));
	LogIfErrorAndReturnValue(error, "reading trailer", TIResult<CDictionary>(*error));

	// Create CBPLReader
	UInt8		objectOffsetFieldSize = trailer.mObjectOffsetFieldSize;
	UInt8		objectIndexFieldSize = trailer.mObjectIndexFieldSize;
	UInt64		totalObjectCount = EndianU64_BtoN(trailer.mTotalObjectCount);
	UInt64		topObjectIndex = EndianU64_BtoN(trailer.mTopObjectIndex);
	UInt64		objectOffsetTableOffset = EndianU64_BtoN(trailer.mObjectOffsetTableOffset);
	CBPLReader*	bplReader =
						new CBPLReader(seekableDataSource, objectOffsetFieldSize, objectIndexFieldSize,
								totalObjectCount, objectOffsetTableOffset);

	// Get top level object
	UInt64	count;
	UInt8	marker = bplReader->readMarker(topObjectIndex, OR<UInt64>(count));
	if (marker != kMarkerTypeDictionary) {
		// Top object is not a dictionary
		Delete(bplReader);
		LogErrorAndReturnValue(sUnknownFormatError, "top object is not a dictionary",
				TIResult<CDictionary>(sUnknownFormatError));
	}

	// Create dictionary
	CDictionary	dictionary = bplReader->readDictionary(topObjectIndex);

	// Remove reference
	bplReader->removeReference();

	return TIResult<CDictionary>(dictionary);
}
