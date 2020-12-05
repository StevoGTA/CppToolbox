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

		SDictionaryValue*	createDictionaryValue(UInt64 objectIndex);

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

												mDictionaryValues = new SDictionaryValue*[mKeyCount];
												::memset(mDictionaryValues, 0, sizeof(SDictionaryValue*) * mKeyCount);

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
														CDictionary::ProcsInfo(getKeyCount, getValue, disposeUserData,
																new CBPLDictionaryInfo(bplDataSource, keyCount)));
											}
		static	CDictionary::KeyCount	getKeyCount(void* userData)
											{
												// Get CBPLDictionaryInfo
												CBPLDictionaryInfo*	bplDictionaryInfo = (CBPLDictionaryInfo*) userData;

												return bplDictionaryInfo->mKeyCount;
											}
		static	OR<SDictionaryValue>	getValue(const CString& key, void* userData)
											{
												// Get CBPLDictionaryInfo
												CBPLDictionaryInfo*	bplDictionaryInfo = (CBPLDictionaryInfo*) userData;

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

														return OR<SDictionaryValue>(
																*bplDictionaryInfo->mDictionaryValues[i]);
													}
												}

												return OR<SDictionaryValue>();
											}
		static	void					disposeUserData(void* userData)
											{
												// Get CBPLDictionaryInfo
												CBPLDictionaryInfo*	bplDictionaryInfo = (CBPLDictionaryInfo*) userData;
												Delete(bplDictionaryInfo);
											}

		CBPLDataSource&			mBPLDataSource;
		CDictionary::KeyCount	mKeyCount;
		UInt32*					mKeyHashes;
		CString**				mKeys;
		UInt64*					mValueIndexes;
		SDictionaryValue**		mDictionaryValues;
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
		CData	data = mByteParceller.readData(count, mError);
		if (mError.hasInstance())
			// Error
			continue;

		// Create string
		mStrings[i] =
				new CString(data,
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
	UInt8	countMarker = mByteParceller.readUInt8(mError);
	LogIfErrorAndReturnValue(mError, errorWhen, 0);

	UInt8	fieldSize;
	switch (countMarker) {
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
SDictionaryValue* CBPLDataSource::createDictionaryValue(UInt64 objectIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check for string
	if (mStrings[objectIndex] != nil)
		// Is a string
		return new SDictionaryValue(*mStrings[objectIndex]);

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

					return new SDictionaryValue(array);
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

					return new SDictionaryValue(array);
				}
			}
		}

		case kMarkerTypeBooleanTrue:
			// True
			return new SDictionaryValue(true);

		case kMarkerTypeBooleanFalse:
			// False
			return new SDictionaryValue(false);

		case kMarkerTypeData:
			// Data
// TODO
AssertFailUnimplemented();

		case kMarkerTypeDictionary:
			// Dictionary
			return new SDictionaryValue(readDictionary(objectIndex));

		case kMarkerTypeFloat32: {
			// Float32
			SwappableFloat32	swappableFloat32;
			swappableFloat32.mStoredValue = (UInt32) readValue(4, "reading float32");

			return new SDictionaryValue(swappableFloat32.mValue);
			} break;

		case kMarkerTypeFloat64: {
			// Float64
			SwappableFloat64	swappableFloat64;
			swappableFloat64.mStoredValue = readValue(8, "reading float64");

			return new SDictionaryValue(swappableFloat64.mValue);
			} break;

		case kMarkerTypeInteger1Byte:
			// Integer, 1 byte
			return new SDictionaryValue((SInt8) readValue(1, "reading integer, 1 byte"));

		case kMarkerTypeInteger2Bytes:
			// Integer, 2 bytes
			return new SDictionaryValue((SInt16) readValue(2, "reading integer, 2 bytes"));

		case kMarkerTypeInteger4Bytes:
			// Integer, 4 bytes
			return new SDictionaryValue((SInt32) readValue(4, "reading integer, 4 bytes"));

		case kMarkerTypeInteger8Bytes:
			// Integer, 8 bytes
			return new SDictionaryValue((SInt64) readValue(8, "reading integer, 8 bytes"));

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
CDictionary CBinaryPropertyList::dictionaryFrom(const CByteParceller& byteParceller, OI<SError>& outError)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt64	dataSize = byteParceller.getSize();

	// Check size
	if (dataSize < (sBinaryPListV10Header.getSize() + sizeof(SBinaryPListTrailer))) {
		// Too small to be a binary property list
		outError = OI<SError>(sUnknownFormatError);
		LogIfErrorAndReturnValue(outError, "checking data source size", CDictionary());
	}

	// Validate header
	CData	header = byteParceller.readData(sBinaryPListV10Header.getSize(), outError);
	LogIfErrorAndReturnValue(outError, "reading header", CDictionary());
	if (header != sBinaryPListV10Header) {
		// Header does not match
		outError = OI<SError>(sUnknownFormatError);
		LogIfErrorAndReturnValue(outError, "validating header", CDictionary());
	}

	// Validate trailer
	outError = byteParceller.setPos(CDataSource::kPositionFromEnd, sizeof(SBinaryPListTrailer));
	LogIfErrorAndReturnValue(outError, "positiong data source to read trailer", CDictionary());

	SBinaryPListTrailer	trailer;
	outError = byteParceller.readData(&trailer, sizeof(SBinaryPListTrailer));
	LogIfErrorAndReturnValue(outError, "reading trailer", CDictionary());

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
		outError = OI<SError>(sUnknownFormatError);
		LogIfErrorAndReturnValue(outError, "top object is not a dictionary", CDictionary());
	}

	// Create dictionary
	CDictionary	dictionary = bplDataSource->readDictionary(topObjectIndex);

	// Remove reference
	bplDataSource->removeReference();

	return dictionary;
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CBinaryPropertyList::writeTo(const CFile& file)
//----------------------------------------------------------------------------------------------------------------------
{
AssertFailUnimplemented();
return OI<SError>();
}
