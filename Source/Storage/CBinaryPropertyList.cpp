//----------------------------------------------------------------------------------------------------------------------
//	CBinaryPropertyList.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CBinaryPropertyList.h"

#include "CByteReader.h"
#include "CFileWriter.h"
#include "CLogServices.h"
#include "CReferenceCountable.h"
#include "TBuffer.h"
#include "Tuple.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

#pragma pack(push, 1)

struct SBinaryPListTrailer {
	public:
				SBinaryPListTrailer(UInt8 objectOffsetByteCount, UInt8 objectIndexByteCount, UInt64 totalObjectCount,
						UInt64 topObjectIndex, UInt64 objectOffsetTableOffset) :
					mObjectOffsetByteCount(objectOffsetByteCount),
							mObjectIndexByteCount(objectIndexByteCount),
							mTotalObjectCount(EndianU64_NtoB(totalObjectCount)),
							mTopObjectIndex(EndianU64_NtoB(topObjectIndex)),
							mObjectOffsetTableOffset(EndianU64_NtoB(objectOffsetTableOffset))
					{
						::memset(mUnused, 0, sizeof(mUnused));
					}

		UInt8	getObjectOffsetByteCount() const
					{ return mObjectOffsetByteCount; }
		UInt8	getObjectIndexByteCount() const
					{ return mObjectIndexByteCount; }
		UInt64	getTotalObjectCount() const
					{ return EndianU64_BtoN(mTotalObjectCount); }
		UInt64	getTopObjectIndex() const
					{ return EndianU64_BtoN(mTopObjectIndex); }
		UInt64	getObjectOffsetTableOffset() const
					{ return EndianU64_BtoN(mObjectOffsetTableOffset); }

	// Properties (in storage endian)
	private:
		UInt8	mUnused[6];
		UInt8	mObjectOffsetByteCount;
		UInt8	mObjectIndexByteCount;
		UInt64	mTotalObjectCount;
		UInt64	mTopObjectIndex;
		UInt64	mObjectOffsetTableOffset;
};

#pragma pack(pop)

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
static	SError	sInvalidObjectIndexError(sErrorDomain, 2, CString(OSSTR("Invalid object index")));
static	SError	sInvalidMarkerError(sErrorDomain, 3, CString(OSSTR("Invalid marker")));
static	SError	sInvalidCountMarkerError(sErrorDomain, 4, CString(OSSTR("Invalid count marker")));
static	SError	sInvalidObjectTypeError(sErrorDomain, 5, CString(OSSTR("Invalid object type")));
static	SError	sUnableToDetermineObjectTypeError(sErrorDomain, 6, CString(OSSTR("Unable to determine object type")));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBPLReader declaration

class CBPLReader {
	public:
		typedef	TV2<UInt8, UInt64>	MarkerAndCount;

												CBPLReader(const I<CRandomAccessDataSource>& randomAccessDataSource,
														const TBuffer<UInt64>& objectOffsets,
														UInt8 objectIndexByteCount);

				TVResult<TArray<CDictionary> >	getArrayOfDictionaries(I<CBPLReader>& bplReader,
														const TBuffer<UInt64>& objectIndexes,
														bool composeStandardDictionary);
				TVResult<TArray<CString> >		getArrayOfStrings(const TBuffer<UInt64>& objectIndexes);
				TVResult<CData>					getData(UInt64 objectIndex);
				TVResult<CDictionary>			getDictionary(I<CBPLReader>& bplReader, UInt64 objectIndex,
														bool composeStandardDictionary);
				TVResult<CString>				getString(UInt64 objectIndex);
				TVResult<SValue>				getValue(I<CBPLReader>& bplReader, UInt64 objectIndex,
														bool composeStandardDictionary);

				OV<SError>						read(void* buffer, UInt64 byteCount, const CString& when);
				OV<SError>						read(const TBuffer<UInt8>& buffer, const CString& when)
													{ return read(*buffer, buffer.getByteCount(), when); }

				TVResult<MarkerAndCount>		readMarkerAndCount(UInt64 objectIndex);
				TVResult<TBuffer<UInt64> >		readObjectIndexes(UInt64 count);
				OV<SError>						validateObjects(const TBuffer<UInt64>& objectIndexes);
				OV<SError>						validateObjectsAsString(const TBuffer<UInt64>& objectIndexes);

		static	TVResult<CDictionary>			getDictionary(const I<CRandomAccessDataSource>& randomAccessDataSource,
														bool composeStandardDictionary);

		const	I<CRandomAccessDataSource>	mRandomAccessDataSource;
				UInt64						mRandomAccessDataSourcePosition;

		const	TBuffer<UInt64>				mObjectOffsets;
				UInt8						mObjectIndexByteCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

class CBPLDictionaryBacking : public CDictionary::Backing {
	public:

	class IteratorInfo : public CDictionary::IteratorInfo {
		public:
								IteratorInfo(CBPLDictionaryBacking& backing) :
									CDictionary::IteratorInfo(),
											mBacking(backing), mCurrentIndex(0)
									{}

					UInt32		getCount() const
									{ return mBacking.getCount(); }
					UInt32		getCurrentIndex() const
									{ return mCurrentIndex; }
			const	CString&	getCurrentKey() const
									{ return mBacking.getKeyAtIndex(mCurrentIndex); }
					SValue&		getCurrentValue() const
									{ return mBacking.getValueAtIndex(mCurrentIndex); }

					void		advance()
									{ ++mCurrentIndex; }

		private:
			CBPLDictionaryBacking&	mBacking;
			UInt32					mCurrentIndex;
	};

												CBPLDictionaryBacking(I<CBPLReader>& bplReader,
														const TBuffer<UInt64>& keyObjectIndexes,
														const TBuffer<UInt64>& valueObjectIndexes) :
													mBPLReader(bplReader), mKeyObjectIndexes(keyObjectIndexes),
															mValueObjectIndexes(valueObjectIndexes)
													{}

				CDictionary::Count				getCount() const
													{ return (CDictionary::Count) mKeyObjectIndexes.getCount(); }
				OR<SValue>						getValue(const CString& key) const
													{
														// Iterate all entries
														for (UInt32 i = 0; i < mKeyObjectIndexes.getCount(); i++) {
															// Check key
															if (key ==
																	((CBPLDictionaryBacking&) *this).getKeyAtIndex(i))
																// Found
																return OR<SValue>(
																		((CBPLDictionaryBacking&) *this)
																				.getValueAtIndex(i));
														}

														return OR<SValue>();
													}
				void							set(const CString& key, const SValue& value)
													{}
				void							remove(const CString& key)
													{}
				void							remove(const TSet<CString>& keys)
													{}
				void							removeAll()
													{}

				I<CDictionary::IteratorInfo>	getIteratorInfo() const
													{ return I<CDictionary::IteratorInfo>(
															new IteratorInfo((CBPLDictionaryBacking&) *this)); }

				I<Backing>						prepareForWrite()
													{ return I<Backing>(nil); }
				SValue::OpaqueEqualsProc		getOpaqueEqualsProc() const
													{ return nil; }

		const	CString&						getKeyAtIndex(UInt32 index)
													{
														// Check if have key already
														if (!mKeyByIndex[index].hasReference())
															// Decode
															mKeyByIndex.set(index,
																	*mBPLReader->getString(mKeyObjectIndexes[index]));

														return *mKeyByIndex[index];
													}
				SValue&							getValueAtIndex(UInt32 index)
													{
														// Check if have value already
														if (!mValueByIndex[index].hasReference())
															// Decode
															mValueByIndex.set(index,
																	*mBPLReader->getValue(mBPLReader,
																			mValueObjectIndexes[index], false));

														return *mValueByIndex[index];
													}

		I<CBPLReader>	mBPLReader;

		TBuffer<UInt64>	mKeyObjectIndexes;
		TNKeyConvertibleDictionary<UInt32, CString>	mKeyByIndex;

		TBuffer<UInt64>	mValueObjectIndexes;
		TNKeyConvertibleDictionary<UInt32, SValue>	mValueByIndex;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBPLReader

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CBPLReader::CBPLReader(const I<CRandomAccessDataSource>& randomAccessDataSource, const TBuffer<UInt64>& objectOffsets,
		UInt8 objectIndexByteCount) :
		mRandomAccessDataSource(randomAccessDataSource), mRandomAccessDataSourcePosition(0),
		mObjectOffsets(objectOffsets), mObjectIndexByteCount(objectIndexByteCount)
//----------------------------------------------------------------------------------------------------------------------
{
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
TVResult<TArray<CDictionary> > CBPLReader::getArrayOfDictionaries(I<CBPLReader>& bplReader,
		const TBuffer<UInt64>& objectIndexes, bool composeStandardDictionary)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNArray<CDictionary>	array;

	// Iterate object indexes
	for (UInt64 i = 0; i < objectIndexes.getCount(); i++) {
		// Get Dictionary
		TVResult<CDictionary>	dictionary = getDictionary(bplReader, objectIndexes[i], composeStandardDictionary);
		ReturnValueIfResultError(dictionary, TVResult<TArray<CDictionary> >(dictionary.getError()));

		// Add
		array += *dictionary;
	}

	return TVResult<TArray<CDictionary> >(array);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<TArray<CString> > CBPLReader::getArrayOfStrings(const TBuffer<UInt64>& objectIndexes)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNArray<CString>	array;

	// Iterate object indexes
	for (UInt64 i = 0; i < objectIndexes.getCount(); i++) {
		// Get String
		TVResult<CString>	string = getString(objectIndexes[i]);
		ReturnValueIfResultError(string, TVResult<TArray<CString> >(string.getError()));

		// Add
		array += *string;
	}

	return TVResult<TArray<CString> >(array);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> CBPLReader::getData(UInt64 objectIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Read marker and count
	TVResult<MarkerAndCount>	markerAndCount = readMarkerAndCount(objectIndex);
	ReturnValueIfResultError(markerAndCount, TVResult<CData>(markerAndCount.getError()));

	// Validate marker
	if (markerAndCount->getA() != kMarkerTypeData)
		// Invalid marker
		LogErrorAndReturnValue(sInvalidMarkerError, CString(OSSTR("validating data marker")),
				TVResult<CData>(sInvalidMarkerError));

	UInt64	count = markerAndCount->getB();

	// Check count
	if (count == 0)
		// Empty string
		return TVResult<CData>(CData::mEmpty);

	// Read bytes
	CData		data(count);
	OV<SError>	error = read(data.getMutableBuffer(count), CString(OSSTR("reading data bytes")));
	ReturnValueIfError(error, TVResult<CData>(*error));

	return TVResult<CData>(data);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CDictionary> CBPLReader::getDictionary(I<CBPLReader>& bplReader, UInt64 objectIndex,
		bool composeStandardDictionary)
//----------------------------------------------------------------------------------------------------------------------
{
	// Read marker and count
	TVResult<MarkerAndCount>	markerAndCount = readMarkerAndCount(objectIndex);
	ReturnValueIfResultError(markerAndCount, TVResult<CDictionary>(markerAndCount.getError()));

	// Validate marker
	if (markerAndCount->getA() != kMarkerTypeDictionary)
		// Invalid marker
		LogErrorAndReturnValue(sInvalidMarkerError, CString(OSSTR("validating dictionary marker")),
				TVResult<CDictionary>(sInvalidMarkerError));

	UInt64	count = markerAndCount->getB();

	// Read key object indexes
	TVResult<TBuffer<UInt64> >	keyObjectIndexes = readObjectIndexes(count);
	ReturnValueIfResultError(keyObjectIndexes, TVResult<CDictionary>(keyObjectIndexes.getError()));

	// Read value object indexes
	TVResult<TBuffer<UInt64> >	valueObjectIndexes = readObjectIndexes(count);
	ReturnValueIfResultError(valueObjectIndexes, TVResult<CDictionary>(valueObjectIndexes.getError()));

	// Validate key objects
	OV<SError>	error = validateObjectsAsString(*keyObjectIndexes);
	ReturnValueIfError(error, TVResult<CDictionary>(*error));

	// Validate value objects
	error = validateObjects(*valueObjectIndexes);
	ReturnValueIfError(error, TVResult<CDictionary>(*error));

	// Check if composing standard dictionary
	if (composeStandardDictionary) {
		// Compose standard dictionary
		CDictionary	dictionary;

		// Iterate items
		for (UInt64 i = 0; i < count; i++) {
			// Get key
			TVResult<CString>	key = bplReader->getString((*keyObjectIndexes)[i]);
			ReturnValueIfResultError(key, TVResult<CDictionary>(key.getError()));

			TVResult<SValue>	value = bplReader->getValue(bplReader, (*valueObjectIndexes)[i], true);
			ReturnValueIfResultError(value, TVResult<CDictionary>(value.getError()));

			// Store
			dictionary.set(*key, *value);
		}

		return TVResult<CDictionary>(dictionary);
	} else
		// Use storage-backed dictionary
		return TVResult<CDictionary>(
				CDictionary(
						I<CDictionary::Backing>(
								new CBPLDictionaryBacking(bplReader, *keyObjectIndexes, *valueObjectIndexes))));
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CString> CBPLReader::getString(UInt64 objectIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Read marker and count
	TVResult<MarkerAndCount>	markerAndCount = readMarkerAndCount(objectIndex);
	ReturnValueIfResultError(markerAndCount, TVResult<CString>(markerAndCount.getError()));

	// Validate marker
	if ((markerAndCount->getA() != kMarkerTypeStringASCII) && (markerAndCount->getA() != kMarkerTypeStringUnicode16))
		// Invalid marker
		LogErrorAndReturnValue(sInvalidMarkerError, CString(OSSTR("validating string marker")),
				TVResult<CString>(sInvalidMarkerError));

	UInt64	count = markerAndCount->getB();

	// Check count
	if (count == 0)
		// Empty string
		return TVResult<CString>(CString::mEmpty);

	// Read bytes
	UInt64			byteCount = (markerAndCount->getA() == kMarkerTypeStringASCII) ? count : count * 2;
	TBuffer<UInt8>	buffer(byteCount);
	OV<SError>		error = read(*buffer, byteCount, CString(OSSTR("reading string bytes")));
	ReturnValueIfError(error, TVResult<CString>(*error));

	return TVResult<CString>(
			CString(buffer,
					(markerAndCount->getA() == kMarkerTypeStringASCII) ?
							CString::kEncodingASCII : CString::kEncodingUTF16BE));
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<SValue> CBPLReader::getValue(I<CBPLReader>& bplReader, UInt64 objectIndex, bool composeStandardDictionary)
//----------------------------------------------------------------------------------------------------------------------
{
	// Read marker and count
	TVResult<MarkerAndCount>	markerAndCount = readMarkerAndCount(objectIndex);
	ReturnValueIfResultError(markerAndCount, TVResult<SValue>(markerAndCount.getError()));

	UInt64	count = markerAndCount->getB();

	// Check marker
	switch (markerAndCount->getA()) {
		case kMarkerTypeArray: {
			// Array
			if (count == 0)
				// Empty array
				return TVResult<SValue>(SValue::emptyArray());

			// Read object indexes
			TVResult<TBuffer<UInt64> >	objectIndexes = readObjectIndexes(count);
			ReturnValueIfResultError(objectIndexes, TVResult<SValue>(objectIndexes.getError()));

			OV<SError>	error = validateObjects(*objectIndexes);
			ReturnValueIfError(error, TVResult<SValue>(*error));

			// Determine array type
			markerAndCount = readMarkerAndCount((*objectIndexes)[0]);
			ReturnValueIfResultError(markerAndCount, TVResult<SValue>(markerAndCount.getError()));

			UInt8	marker = markerAndCount->getA();
			if ((marker >= kMarkerTypeInteger1Byte) && (marker <= kMarkerTypeInteger8Bytes)) {
				// Integers, so use the widest.  The integer markers increase with width.
				for (UInt64 i = 1; i < count; i++) {
					// Get marker
					markerAndCount = readMarkerAndCount((*objectIndexes)[i]);
					ReturnValueIfResultError(markerAndCount, TVResult<SValue>(markerAndCount.getError()));

					// Check if a wider integer
					if ((markerAndCount->getA() > marker) && (markerAndCount->getA() <= kMarkerTypeInteger8Bytes))
						// Wider
						marker = markerAndCount->getA();
				}
			}

			switch (marker) {
				case kMarkerTypeArray:
					// Array
AssertFailUnimplemented();

				case kMarkerTypeBooleanTrue:
				case kMarkerTypeBooleanFalse:
					// Boolean
AssertFailUnimplemented();

				case kMarkerTypeData:
					// Data
AssertFailUnimplemented();

				case kMarkerTypeDictionary: {
					// Dictionary
					TVResult<TArray<CDictionary> >	array =
															getArrayOfDictionaries(bplReader, *objectIndexes,
																	composeStandardDictionary);
					ReturnValueIfResultError(array, TVResult<SValue>(array.getError()));

					return TVResult<SValue>(*array);
				}

				case kMarkerTypeFloat32: {
					// Float32s
					TNumberArray<Float32>	array;
					for (UInt64 i = 0; i < count; i++) {
						// Get value
						TVResult<SValue>	value =
													getValue(bplReader, (*objectIndexes)[i], composeStandardDictionary);
						ReturnValueIfResultError(value, TVResult<SValue>(value.getError()));

						array += value->getFloat32();
					}

					return TVResult<SValue>(SValue(array));
				}

				case kMarkerTypeFloat64:
					// Float64
AssertFailUnimplemented();

				case kMarkerTypeInteger1Byte: {
					// Integer, 1 byte
					TNumberArray<UInt8>	array;
					for (UInt64 i = 0; i < count; i++) {
						// Get value
						TVResult<SValue>	value =
													getValue(bplReader, (*objectIndexes)[i], composeStandardDictionary);
						ReturnValueIfResultError(value, TVResult<SValue>(value.getError()));

						array += value->getUInt8();
					}

					return TVResult<SValue>(SValue(array));
				}

				case kMarkerTypeInteger2Bytes: {
					// Integer, 2 bytes
					TNumberArray<UInt16>	array;
					for (UInt64 i = 0; i < count; i++) {
						// Get value
						TVResult<SValue>	value =
													getValue(bplReader, (*objectIndexes)[i], composeStandardDictionary);
						ReturnValueIfResultError(value, TVResult<SValue>(value.getError()));

						array += value->getUInt16();
					}

					return TVResult<SValue>(SValue(array));
				}

				case kMarkerTypeInteger4Bytes: {
					// Integer, 4 bytes
					TNumberArray<UInt32>	array;
					for (UInt64 i = 0; i < count; i++) {
						// Get value
						TVResult<SValue>	value =
													getValue(bplReader, (*objectIndexes)[i], composeStandardDictionary);
						ReturnValueIfResultError(value, TVResult<SValue>(value.getError()));

						array += value->getUInt32();
					}

					return TVResult<SValue>(SValue(array));
				}

				case kMarkerTypeInteger8Bytes: {
					// Integer, 8 bytes
					TNumberArray<UInt64>	array;
					for (UInt64 i = 0; i < count; i++) {
						// Get value
						TVResult<SValue>	value =
													getValue(bplReader, (*objectIndexes)[i], composeStandardDictionary);
						ReturnValueIfResultError(value, TVResult<SValue>(value.getError()));

						array += value->getUInt64();
					}

					return TVResult<SValue>(SValue(array));
				}

				case kMarkerTypeStringASCII:
				case kMarkerTypeStringUnicode16: {
					// String
					TVResult<TArray<CString> >	array = getArrayOfStrings(*objectIndexes);
					ReturnValueIfResultError(array, TVResult<SValue>(array.getError()));

					return TVResult<SValue>(*array);
				}
			}
		}

		case kMarkerTypeBooleanTrue:
			// True
			return TVResult<SValue>(SValue(true));

		case kMarkerTypeBooleanFalse:
			// False
			return TVResult<SValue>(SValue(false));

		case kMarkerTypeData: {
			// Data
			TVResult<CData>	data = getData(objectIndex);
			ReturnValueIfResultError(data, TVResult<SValue>(data.getError()));

			return TVResult<SValue>(SValue(*data));
			}

		case kMarkerTypeDictionary: {
			// Dictionary
			TVResult<CDictionary>	dictionary = getDictionary(bplReader, objectIndex, composeStandardDictionary);
			ReturnValueIfResultError(dictionary, TVResult<SValue>(dictionary.getError()));

			return TVResult<SValue>(SValue(*dictionary));
			}

		case kMarkerTypeFloat32: {
			// Float32
			StoredFloat32	storedValue;
			OV<SError>		error = read(&storedValue, sizeof(StoredFloat32), CString(OSSTR("reading float32")));
			ReturnValueIfError(error, TVResult<SValue>(*error));

			return TVResult<SValue>(SValue(EndianF32_BtoN(storedValue)));
			}

		case kMarkerTypeFloat64: {
			// Float64
			StoredFloat64	storedValue;
			OV<SError>		error = read(&storedValue, sizeof(StoredFloat64), CString(OSSTR("reading float64")));
			ReturnValueIfError(error, TVResult<SValue>(*error));

			return TVResult<SValue>(SValue(EndianF64_BtoN(storedValue)));
			}

		case kMarkerTypeInteger1Byte: {
			// Integer, 1 byte
			UInt8		value;
			OV<SError>	error = read(&value, sizeof(UInt8), CString(OSSTR("reading integer, 1 byte")));
			ReturnValueIfError(error, TVResult<SValue>(*error));

			return TVResult<SValue>(SValue(value));
			}

		case kMarkerTypeInteger2Bytes: {
			// Integer, 2 bytes
			UInt16		value;
			OV<SError>	error = read(&value, sizeof(UInt16), CString(OSSTR("reading integer, 2 bytes")));
			ReturnValueIfError(error, TVResult<SValue>(*error));

			return TVResult<SValue>(SValue(EndianU16_BtoN(value)));
			}

		case kMarkerTypeInteger4Bytes: {
			// Integer, 4 bytes
			UInt32		value;
			OV<SError>	error = read(&value, sizeof(UInt32), CString(OSSTR("reading integer, 4 bytes")));
			ReturnValueIfError(error, TVResult<SValue>(*error));

			return TVResult<SValue>(SValue(EndianU32_BtoN(value)));
			}

		case kMarkerTypeInteger8Bytes: {
			// Integer, 8 bytes
			UInt64		value;
			OV<SError>	error = read(&value, sizeof(UInt64), CString(OSSTR("reading integer, 8 bytes")));
			ReturnValueIfError(error, TVResult<SValue>(*error));

			return TVResult<SValue>(SValue(EndianU64_BtoN(value)));
			}

		case kMarkerTypeStringASCII:
		case kMarkerTypeStringUnicode16: {
			// String
			TVResult<CString>	string = getString(objectIndex);
			ReturnValueIfResultError(string, TVResult<SValue>(string.getError()));

			return TVResult<SValue>(SValue(*string));
			}

		default:
			// Invalid
			LogError(sInvalidMarkerError, CString(OSSTR("validating marker")));

			return TVResult<SValue>(sInvalidMarkerError);
	}
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CBPLReader::read(void* buffer, UInt64 byteCount, const CString& when)
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	OV<SError>	error = mRandomAccessDataSource->read(mRandomAccessDataSourcePosition, buffer, byteCount);
	LogIfErrorAndReturnError(error, when);

	// Update
	mRandomAccessDataSourcePosition += byteCount;

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CBPLReader::MarkerAndCount> CBPLReader::readMarkerAndCount(UInt64 objectIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Validate object index
	if (objectIndex >= mObjectOffsets.getCount())
		// Invalid object index
		return TVResult<MarkerAndCount>(sInvalidObjectIndexError);

	// Update position
	mRandomAccessDataSourcePosition = mObjectOffsets[objectIndex];

	// Read marker
	UInt8		marker;
	OV<SError>	error = read(&marker, sizeof(UInt8), CString(OSSTR("reading marker")));
	ReturnValueIfError(error, TVResult<MarkerAndCount>(*error));

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
			return TVResult<MarkerAndCount>(MarkerAndCount(marker, 0));

		default: {
			// Have count
			UInt64	count = marker & kMarkerCountMask;
			marker = marker & ~kMarkerCountMask;

			if (count < 15)
				// Count included in marker
				return TVResult<MarkerAndCount>(MarkerAndCount(marker, count));

			// Read count marker
			UInt8	countMarker;
			error = read(&countMarker, sizeof(UInt8), CString(OSSTR("reading count marker")));
			ReturnValueIfError(error, TVResult<MarkerAndCount>(*error));

			UInt8	countByteCount;
			switch (countMarker) {
				case kMarkerTypeInteger1Byte:	countByteCount = 1;	break;
				case kMarkerTypeInteger2Bytes:	countByteCount = 2;	break;
				case kMarkerTypeInteger4Bytes:	countByteCount = 4;	break;
				case kMarkerTypeInteger8Bytes:	countByteCount = 8;	break;
				default:						return TVResult<MarkerAndCount>(sInvalidCountMarkerError);
			}

			// Read count
			count = 0;
			error =
					read((UInt8*) &count + sizeof(UInt64) - countByteCount, countByteCount,
							CString(OSSTR("reading marker and count count")));
			ReturnValueIfError(error, TVResult<MarkerAndCount>(*error));

			return TVResult<MarkerAndCount>(MarkerAndCount(marker, EndianU64_BtoN(count)));
			}
	}
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<TBuffer<UInt64> > CBPLReader::readObjectIndexes(UInt64 count)
//----------------------------------------------------------------------------------------------------------------------
{
	TBuffer<UInt8>	objectIndexBuffer(count * mObjectIndexByteCount);
	OV<SError>		error =
							read(*objectIndexBuffer, count * mObjectIndexByteCount,
									CString(OSSTR("reading object indexes")));
	ReturnValueIfError(error, TVResult<TBuffer<UInt64> >(*error));

	// Prepare object indexes
	TBuffer<UInt64>	objectIndexes(count);
	for (UInt64 i = 0; i < count; i++) {
		// Copy object indexes into usable storage
		UInt64	objectIndex = 0;
		::memcpy((UInt8*) &objectIndex + sizeof(UInt64) - mObjectIndexByteCount,
				&(*objectIndexBuffer)[i * mObjectIndexByteCount], mObjectIndexByteCount);
		objectIndexes[i] = EndianU64_BtoN(objectIndex);
	}

	return TVResult<TBuffer<UInt64> >(objectIndexes);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CBPLReader::validateObjects(const TBuffer<UInt64>& objectIndexes)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate objects
	for (UInt64 i = 0; i < objectIndexes.getCount(); i++) {
		// Get Marker (and Count)
		TVResult<MarkerAndCount>	markerAndCount = readMarkerAndCount(objectIndexes[i]);
		ReturnErrorIfResultError(markerAndCount);

		// Validate marker
		switch (markerAndCount->getA()) {
			case kMarkerTypeArray:
			case kMarkerTypeBooleanTrue:
			case kMarkerTypeBooleanFalse:
			case kMarkerTypeData:
			case kMarkerTypeDictionary:
			case kMarkerTypeFloat32:
			case kMarkerTypeFloat64:
			case kMarkerTypeInteger1Byte:
			case kMarkerTypeInteger2Bytes:
			case kMarkerTypeInteger4Bytes:
			case kMarkerTypeInteger8Bytes:
			case kMarkerTypeStringASCII:
			case kMarkerTypeStringUnicode16:
				// Known
				break;

			default:
				// Unknown marker
				return OV<SError>(sInvalidMarkerError);
		}
	}

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CBPLReader::validateObjectsAsString(const TBuffer<UInt64>& objectIndexes)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate objects
	for (UInt64 i = 0; i < objectIndexes.getCount(); i++) {
		// Get Marker (and Count)
		TVResult<MarkerAndCount>	markerAndCount = readMarkerAndCount(objectIndexes[i]);
		ReturnErrorIfResultError(markerAndCount);

		// Validate marker
		if ((markerAndCount->getA() != kMarkerTypeStringASCII) &&
				(markerAndCount->getA() != kMarkerTypeStringUnicode16))
			// Unknown marker
			return OV<SError>(sInvalidMarkerError);
	}

	return OV<SError>();
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TVResult<CDictionary> CBPLReader::getDictionary(const I<CRandomAccessDataSource>& randomAccessDataSource,
		bool composeStandardDictionary)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt64		byteCount = randomAccessDataSource->getByteCount();
	OV<SError>	error;

	// Check file size
	if (byteCount < (sBinaryPListV10Header.getByteCount() + sizeof(SBinaryPListTrailer)))
		// Too small to be a binary property list
		LogErrorAndReturnValue(sUnknownFormatError, CString(OSSTR("checking header by byte count")),
				TVResult<CDictionary>(sUnknownFormatError));

	// Validate header
	TVResult<CData>	header = randomAccessDataSource->readData(0, sBinaryPListV10Header.getByteCount());
	LogIfResultErrorAndReturnValue(header, CString(OSSTR("reading header")), TVResult<CDictionary>(header.getError()));

	if (*header != sBinaryPListV10Header)
		// Header does not match
		LogErrorAndReturnValue(sUnknownFormatError, CString(OSSTR("validating header")),
				TVResult<CDictionary>(sUnknownFormatError));

	// Validate trailer
	TVResult<TBuffer<const UInt8> >	trailerBuffer =
											randomAccessDataSource->readUInt8Buffer(
													byteCount - sizeof(SBinaryPListTrailer),
													sizeof(SBinaryPListTrailer));
	LogIfResultErrorAndReturnValue(trailerBuffer, CString(OSSTR("reading trailer")),
				TVResult<CDictionary>(trailerBuffer.getError()));

	const	SBinaryPListTrailer&	trailer = *((const SBinaryPListTrailer*) **trailerBuffer);
			UInt8					objectOffsetByteCount = trailer.getObjectOffsetByteCount();
			UInt64					totalObjectCount = trailer.getTotalObjectCount();

	if ((objectOffsetByteCount == 0) || (objectOffsetByteCount > sizeof(UInt64)) ||
			(trailer.getObjectIndexByteCount() == 0) || (trailer.getObjectIndexByteCount() > sizeof(UInt64)))
		// Byte counts are out of range
		LogErrorAndReturnValue(sUnknownFormatError, CString(OSSTR("validating trailer")),
				TVResult<CDictionary>(sUnknownFormatError));

	// Read object offsets
	TVResult<TBuffer<const UInt8> >	objectOffsetBuffer =
											randomAccessDataSource->readUInt8Buffer(
													trailer.getObjectOffsetTableOffset(),
													totalObjectCount * objectOffsetByteCount);
	LogIfResultErrorAndReturnValue(objectOffsetBuffer, CString(OSSTR("reading object indexes")),
			TVResult<CDictionary>(objectOffsetBuffer.getError()));

	// Prepare object offsets
	TBuffer<UInt64>	objectOffsets(totalObjectCount);
	for (UInt64 i = 0; i < totalObjectCount; i++) {
		// Copy object offsets into usable storage
		UInt64	objectOffset = 0;
		::memcpy((UInt8*) &objectOffset + sizeof(UInt64) - objectOffsetByteCount,
				&(*objectOffsetBuffer)[i * objectOffsetByteCount], objectOffsetByteCount);
		objectOffsets[i] = EndianU64_BtoN(objectOffset);
	}

	// Create BPLReader
	I<CBPLReader>	bplReader(new CBPLReader(randomAccessDataSource, objectOffsets, trailer.getObjectIndexByteCount()));

	return bplReader->getDictionary(bplReader, trailer.getTopObjectIndex(), composeStandardDictionary);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBPLWriter

class CBPLWriter {
	public:
		typedef	TV2<UInt64, TSet<CString> >	PreprocessResult;
		typedef	TV2<UInt64, UInt64>			ObjectInfo;

								CBPLWriter(const CDictionary& dictionary) :
									mDictionary(dictionary)
									{}

		PreprocessResult		preprocess(const CDictionary& dictionary)
									{
										// Setup
										UInt64			objectCount = 1;
										TNSet<CString>	uniqueStrings;

										// Iterate all entries
										for (CDictionary::Iterator iterator = dictionary.getIterator(); iterator;
												iterator++) {
											// Add key
											uniqueStrings += iterator.getKey();

											// Check value type
											const	SValue&	value = iterator.getValue();
											switch (value.getType()) {
												case SValue::kTypeArrayOfDictionaries:
													// Array of Dictionaries - will only store if have at least 1 item
													if (value.getArrayOfDictionaries().getCount() > 0) {
														// Will store
														objectCount++;

														for (TArray<CDictionary>::Iterator arrayIterator =
																		value.getArrayOfDictionaries().getIterator();
																arrayIterator; arrayIterator++) {
															// Preprocess
															PreprocessResult	preprocessResult =
																						preprocess(*arrayIterator);
															objectCount += preprocessResult.getA();
															uniqueStrings += preprocessResult.getB();
														}
													}
													break;

												case SValue::kTypeArrayOfStrings:
													// Array of Strings - will only store if have at least 1 item
													if (value.getArrayOfStrings().getCount() > 0) {
														// Will store
														objectCount++;
														uniqueStrings += value.getArrayOfStrings();
													}
													break;

												case SValue::kTypeArrayOfFloat32s:
													// Array of Float32s - will only store if have at least 1 item
													if (value.getArrayOfFloat32s().getCount() > 0)
														// Will store the array and each number
														objectCount += 1 + value.getArrayOfFloat32s().getCount();
													break;

												case SValue::kTypeArrayOfUInt8s:
													// Array of UInt8s - will only store if have at least 1 item
													if (value.getArrayOfUInt8s().getCount() > 0)
														// Will store the array and each number
														objectCount += 1 + value.getArrayOfUInt8s().getCount();
													break;

												case SValue::kTypeArrayOfUInt16s:
													// Array of UInt16s - will only store if have at least 1 item
													if (value.getArrayOfUInt16s().getCount() > 0)
														// Will store the array and each number
														objectCount += 1 + value.getArrayOfUInt16s().getCount();
													break;

												case SValue::kTypeArrayOfUInt32s:
													// Array of UInt32s - will only store if have at least 1 item
													if (value.getArrayOfUInt32s().getCount() > 0)
														// Will store the array and each number
														objectCount += 1 + value.getArrayOfUInt32s().getCount();
													break;

												case SValue::kTypeArrayOfUInt64s:
													// Array of UInt64s - will only store if have at least 1 item
													if (value.getArrayOfUInt64s().getCount() > 0)
														// Will store the array and each number
														objectCount += 1 + value.getArrayOfUInt64s().getCount();
													break;

												case SValue::kTypeBool:
												case SValue::kTypeData:
												case SValue::kTypeFloat32:
												case SValue::kTypeFloat64:
												case SValue::kTypeSInt8:
												case SValue::kTypeSInt16:
												case SValue::kTypeSInt32:
												case SValue::kTypeSInt64:
												case SValue::kTypeUInt8:
												case SValue::kTypeUInt16:
												case SValue::kTypeUInt32:
												case SValue::kTypeUInt64:
													// Single object values
													objectCount++;
													break;

												case SValue::kTypeString:
													// String
													uniqueStrings += value.getString();
													break;

												case SValue::kTypeDictionary: {
													// Dictionary
													PreprocessResult	preprocessResult =
																				preprocess(value.getDictionary());
													objectCount += preprocessResult.getA();
													uniqueStrings += preprocessResult.getB();
													} break;

												default:
													// Cannot process these values
													LogErrorAndReturnValue(sInvalidObjectTypeError,
															CString(OSSTR("preprocessing dictionary")),
															PreprocessResult(0, TNSet<CString>()));
											}
										}

										return PreprocessResult(objectCount, uniqueStrings);
									}

		void					addIndex(CData& data, UInt64 index)
									{
										// Setup
										UInt64	swappedIndex = EndianU64_NtoB(index);

										// Add
										data.append((UInt8*) &swappedIndex + sizeof(UInt64) - mObjectIndexByteCount,
												mObjectIndexByteCount);
									}
		UInt8					getIntegerByteCount(UInt64 value)
									{
										// Check value
										if (value <= 0xFF)
											// 1 byte
											return 1;
										else if (value <= 0xFFFF)
											// 2 bytes
											return 2;
										else if (value <= 0xFFFFFFFF)
											// 4 bytes
											return 4;
										else
											// 8 bytes
											return 8;
									}

		TVResult<CData>			getData()
									{
										// Setup
										CData	data;
										mObjectIndexByString.removeAll();
										mObjectInfos.removeAll();

										// Write header
										data += sBinaryPListV10Header;

										// Preprocess
										PreprocessResult	preprocessResult = preprocess(mDictionary);

										// Process results
										mObjectIndexByteCount =
												getIntegerByteCount(
														preprocessResult.getA() + preprocessResult.getB().getCount());

										for (TSet<CString>::Iterator iterator = preprocessResult.getB().getIterator();
												iterator; iterator++)
											// Write and note index
											mObjectIndexByString.set(*iterator, write(data, *iterator).getA());

										// Write objects
										TVResult<ObjectInfo>	result = write(data, mDictionary);
										ReturnValueIfResultError(result, TVResult<CData>(result.getError()));

										// Get current state
										UInt64	topObjectIndex = result->getA();
										UInt8	objectOffsetByteCount = getIntegerByteCount(data.getByteCount());
										UInt64	totalObjectCount = mObjectInfos.getCount();
										UInt64	objectOffsetTableOffset = data.getByteCount();

										// Write object offsets
										for (TArray<ObjectInfo>::Iterator iterator = mObjectInfos.getIterator();
												iterator; iterator++) {
											// Write offset
											UInt64	swappedOffset = EndianU64_NtoB(iterator->getB());
											data.append(
													(UInt8*) &swappedOffset + sizeof(UInt64) - objectOffsetByteCount,
													objectOffsetByteCount);
										}

										// Write trailer
										SBinaryPListTrailer	trailer(objectOffsetByteCount, mObjectIndexByteCount,
																	totalObjectCount, topObjectIndex,
																	objectOffsetTableOffset);
										data.append(&trailer, sizeof(SBinaryPListTrailer));

										return TVResult<CData>(data);
									}
		void					writeMarkerAndCount(CData& data, UInt8 marker, UInt64 count)
									{
										// Check count
										if (count < 15)
											// Simple marker
											data.append((UInt8) (marker | count));
										else {
											// Marker, then count
											data.append((UInt8) (marker | kMarkerCountMask));

											// Write count marker
											UInt8	countByteCount = getIntegerByteCount(count);
											UInt8	countMarker;
											switch (countByteCount) {
												case 1:		countMarker = kMarkerTypeInteger1Byte;	break;
												case 2:		countMarker = kMarkerTypeInteger2Bytes;	break;
												case 4:		countMarker = kMarkerTypeInteger4Bytes;	break;
												default:	countMarker = kMarkerTypeInteger8Bytes;	break;
											}
											data.append(countMarker);

											// Write count
											UInt64	swappedCount = EndianU64_NtoB(count);
											data.append((UInt8*) &swappedCount + sizeof(UInt64) - countByteCount,
													countByteCount);
										}
									}
		TVResult<ObjectInfo>	write(CData& data, const TArray<CDictionary>& array)
									{
										// Setup
										CData	indexesData(array.getCount() * mObjectIndexByteCount);

										// Iterate values
										for (TArray<CDictionary>::Iterator iterator = array.getIterator(); iterator;
												iterator++) {
											// Add value
											TVResult<ObjectInfo>	result = write(data, *iterator);
											ReturnResultIfResultError(result);

											// Add info
											addIndex(indexesData, result->getA());
										}

										// Get object info
										ObjectInfo	objectInfo(mObjectInfos.getCount(), data.getByteCount());
										mObjectInfos += objectInfo;

										// Write marker and count
										writeMarkerAndCount(data, kMarkerTypeArray, array.getCount());

										// Write indexes
										data += indexesData;

										return TVResult<ObjectInfo>(objectInfo);
									}
		ObjectInfo				write(CData& data, const TArray<CString>& array)
									{
										// Setup
										CData	indexesData(array.getCount() * mObjectIndexByteCount);

										// Iterate values
										for (TArray<CString>::Iterator iterator = array.getIterator(); iterator;
												iterator++)
											// Add index
											addIndex(indexesData, mObjectIndexByString.getUInt64(*iterator));

										// Get object info
										ObjectInfo	objectInfo(mObjectInfos.getCount(), data.getByteCount());
										mObjectInfos += objectInfo;

										// Write marker and count
										writeMarkerAndCount(data, kMarkerTypeArray, array.getCount());

										// Write indexes
										data += indexesData;

										return objectInfo;
									}
		ObjectInfo				write(CData& data, const TNumberArray<Float32>& array)
									{
										// Setup
										CData	indexesData(array.getCount() * mObjectIndexByteCount);

										// Iterate values
										for (TNumberArray<Float32>::Iterator iterator = array.getIterator(); iterator;
												iterator++)
											// Write number and add index
											addIndex(indexesData, write(data, *iterator).getA());

										// Get object info
										ObjectInfo	objectInfo(mObjectInfos.getCount(), data.getByteCount());
										mObjectInfos += objectInfo;

										// Write marker and count
										writeMarkerAndCount(data, kMarkerTypeArray, array.getCount());

										// Write indexes
										data += indexesData;

										return objectInfo;
									}
		ObjectInfo				write(CData& data, const TNumberArray<UInt8>& array)
									{
										// Setup
										CData	indexesData(array.getCount() * mObjectIndexByteCount);

										// Iterate values
										for (TNumberArray<UInt8>::Iterator iterator = array.getIterator(); iterator;
												iterator++)
											// Write number and add index
											addIndex(indexesData,
													write(data, (UInt64) *iterator,
															getIntegerByteCount(*iterator)).getA());

										// Get object info
										ObjectInfo	objectInfo(mObjectInfos.getCount(), data.getByteCount());
										mObjectInfos += objectInfo;

										// Write marker and count
										writeMarkerAndCount(data, kMarkerTypeArray, array.getCount());

										// Write indexes
										data += indexesData;

										return objectInfo;
									}
		ObjectInfo				write(CData& data, const TNumberArray<UInt16>& array)
									{
										// Setup
										CData	indexesData(array.getCount() * mObjectIndexByteCount);

										// Iterate values
										for (TNumberArray<UInt16>::Iterator iterator = array.getIterator(); iterator;
												iterator++)
											// Write number and add index
											addIndex(indexesData,
													write(data, (UInt64) *iterator,
															getIntegerByteCount(*iterator)).getA());

										// Get object info
										ObjectInfo	objectInfo(mObjectInfos.getCount(), data.getByteCount());
										mObjectInfos += objectInfo;

										// Write marker and count
										writeMarkerAndCount(data, kMarkerTypeArray, array.getCount());

										// Write indexes
										data += indexesData;

										return objectInfo;
									}
		ObjectInfo				write(CData& data, const TNumberArray<UInt32>& array)
									{
										// Setup
										CData	indexesData(array.getCount() * mObjectIndexByteCount);

										// Iterate values
										for (TNumberArray<UInt32>::Iterator iterator = array.getIterator(); iterator;
												iterator++)
											// Write number and add index
											addIndex(indexesData,
													write(data, (UInt64) *iterator,
															getIntegerByteCount(*iterator)).getA());

										// Get object info
										ObjectInfo	objectInfo(mObjectInfos.getCount(), data.getByteCount());
										mObjectInfos += objectInfo;

										// Write marker and count
										writeMarkerAndCount(data, kMarkerTypeArray, array.getCount());

										// Write indexes
										data += indexesData;

										return objectInfo;
									}
		ObjectInfo				write(CData& data, const TNumberArray<UInt64>& array)
									{
										// Setup
										CData	indexesData(array.getCount() * mObjectIndexByteCount);

										// Iterate values
										for (TNumberArray<UInt64>::Iterator iterator = array.getIterator(); iterator;
												iterator++)
											// Write number and add index
											addIndex(indexesData,
													write(data, (UInt64) *iterator,
															getIntegerByteCount(*iterator)).getA());

										// Get object info
										ObjectInfo	objectInfo(mObjectInfos.getCount(), data.getByteCount());
										mObjectInfos += objectInfo;

										// Write marker and count
										writeMarkerAndCount(data, kMarkerTypeArray, array.getCount());

										// Write indexes
										data += indexesData;

										return objectInfo;
									}
		ObjectInfo				write(CData& data, bool value)
									{
										// Setup
										ObjectInfo	objectInfo(mObjectInfos.getCount(), data.getByteCount());
										mObjectInfos += objectInfo;

										// Write
										data.append(value ? kMarkerTypeBooleanTrue : kMarkerTypeBooleanFalse);

										return objectInfo;
									}
		ObjectInfo				write(CData& data, const CData& value)
									{
										// Setup
										ObjectInfo	objectInfo(mObjectInfos.getCount(), data.getByteCount());
										mObjectInfos += objectInfo;

										// Write marker and count
										writeMarkerAndCount(data, kMarkerTypeData, value.getByteCount());

										// Write data
										data += value;

										return objectInfo;
									}
		TVResult<ObjectInfo>	write(CData& data, const CDictionary& dictionary)
									{
										// Setup
										CData	keyIndexesData(dictionary.getCount() * mObjectIndexByteCount);
										CData	valueIndexesData(dictionary.getCount() * mObjectIndexByteCount);

										// Iterate items
										for (CDictionary::Iterator iterator = dictionary.getIterator(); iterator;
												iterator++) {
											// Check value type
											const	SValue&	value = iterator.getValue();
											if (value.getType() == SValue::kTypeString) {
												// String
												addIndex(keyIndexesData,
														mObjectIndexByString.getUInt64(iterator.getKey()));
												addIndex(valueIndexesData,
														mObjectIndexByString.getUInt64(value.getString()));
											} else {
												// Other
												OV<TVResult<ObjectInfo> >	result;
												switch (value.getType()) {
													case SValue::kTypeArrayOfDictionaries:
														// Array of Dictionaries - only store if have at least 1 item
														if (value.getArrayOfDictionaries().getCount() > 0) {
															// Write
															addIndex(keyIndexesData,
																	mObjectIndexByString.getUInt64(iterator.getKey()));
															result.setValue(
																	write(data, value.getArrayOfDictionaries()));
															break;
														} else
															// Will not write
															continue;

													case SValue::kTypeArrayOfStrings:
														// Array of Strings - only store if have at least 1 item
														if (value.getArrayOfStrings().getCount() > 0) {
															// Write
															addIndex(keyIndexesData,
																	mObjectIndexByString.getUInt64(iterator.getKey()));
															result.setValue(write(data, value.getArrayOfStrings()));
															break;
														} else
															// Will not write
															continue;

													case SValue::kTypeArrayOfFloat32s:
														// Array of Float32s - only store if have at least 1 item
														if (value.getArrayOfFloat32s().getCount() > 0) {
															// Write
															addIndex(keyIndexesData,
																	mObjectIndexByString.getUInt64(iterator.getKey()));
															result.setValue(write(data, value.getArrayOfFloat32s()));
															break;
														} else
															// Will not write
															continue;

													case SValue::kTypeArrayOfUInt8s:
														// Array of UInt8s - only store if have at least 1 item
														if (value.getArrayOfUInt8s().getCount() > 0) {
															// Write
															addIndex(keyIndexesData,
																	mObjectIndexByString.getUInt64(iterator.getKey()));
															result.setValue(write(data, value.getArrayOfUInt8s()));
															break;
														} else
															// Will not write
															continue;

													case SValue::kTypeArrayOfUInt16s:
														// Array of UInt16s - only store if have at least 1 item
														if (value.getArrayOfUInt16s().getCount() > 0) {
															// Write
															addIndex(keyIndexesData,
																	mObjectIndexByString.getUInt64(iterator.getKey()));
															result.setValue(write(data, value.getArrayOfUInt16s()));
															break;
														} else
															// Will not write
															continue;

													case SValue::kTypeArrayOfUInt32s:
														// Array of UInt32s - only store if have at least 1 item
														if (value.getArrayOfUInt32s().getCount() > 0) {
															// Write
															addIndex(keyIndexesData,
																	mObjectIndexByString.getUInt64(iterator.getKey()));
															result.setValue(write(data, value.getArrayOfUInt32s()));
															break;
														} else
															// Will not write
															continue;

													case SValue::kTypeArrayOfUInt64s:
														// Array of UInt64s - only store if have at least 1 item
														if (value.getArrayOfUInt64s().getCount() > 0) {
															// Write
															addIndex(keyIndexesData,
																	mObjectIndexByString.getUInt64(iterator.getKey()));
															result.setValue(write(data, value.getArrayOfUInt64s()));
															break;
														} else
															// Will not write
															continue;

													case SValue::kTypeBool:
														// Bool
														addIndex(keyIndexesData,
																mObjectIndexByString.getUInt64(iterator.getKey()));
														result.setValue(write(data, value.getBool()));
														break;

													case SValue::kTypeData:
														// Data
														addIndex(keyIndexesData,
																mObjectIndexByString.getUInt64(iterator.getKey()));
														result.setValue(write(data, value.getData()));
														break;

													case SValue::kTypeDictionary:
														// Dictionary
														addIndex(keyIndexesData,
																mObjectIndexByString.getUInt64(iterator.getKey()));
														result.setValue(write(data, value.getDictionary()));
														break;

													case SValue::kTypeFloat32:
														// FLoat32
														addIndex(keyIndexesData,
																mObjectIndexByString.getUInt64(iterator.getKey()));
														result.setValue(write(data, value.getFloat32()));
														break;

													case SValue::kTypeFloat64:
														// Float64
														addIndex(keyIndexesData,
																mObjectIndexByString.getUInt64(iterator.getKey()));
														result.setValue(write(data, value.getFloat64()));
														break;

													case SValue::kTypeSInt8:
													case SValue::kTypeSInt16:
													case SValue::kTypeSInt32:
													case SValue::kTypeSInt64:
													case SValue::kTypeUInt8:
													case SValue::kTypeUInt16:
													case SValue::kTypeUInt32:
													case SValue::kTypeUInt64:
														// Integer
														addIndex(keyIndexesData,
																mObjectIndexByString.getUInt64(iterator.getKey()));
														result.setValue(
																write(data, value.getUInt64(),
																		getIntegerByteCount(value.getUInt64())));
														break;

													default:
														// Unable to write
														LogErrorAndReturnValue(sInvalidObjectTypeError,
																CString(OSSTR("writing dictionary")),
																TVResult<ObjectInfo>(sInvalidObjectTypeError));
												}
												ReturnResultIfResultError((*result));
												addIndex(valueIndexesData, (*result)->getA());
											}
										}

										// Get object info
										ObjectInfo	objectInfo(mObjectInfos.getCount(), data.getByteCount());
										mObjectInfos += objectInfo;

										// Write marker and count
										writeMarkerAndCount(data, kMarkerTypeDictionary,
												keyIndexesData.getByteCount() / mObjectIndexByteCount);

										// Write key indexes and value indexes
										data += keyIndexesData + valueIndexesData;

										return TVResult<ObjectInfo>(objectInfo);
									}
		ObjectInfo				write(CData& data, Float32 value)
									{
										// Setup
										ObjectInfo	objectInfo(mObjectInfos.getCount(), data.getByteCount());
										mObjectInfos += objectInfo;

										// Write marker
										data.append(kMarkerTypeFloat32);

										// Write value
										data.append(EndianF32_NtoB(value));

										return objectInfo;
									}
		ObjectInfo				write(CData& data, Float64 value)
									{
										// Setup
										ObjectInfo	objectInfo(mObjectInfos.getCount(), data.getByteCount());
										mObjectInfos += objectInfo;

										// Write marker
										data.append(kMarkerTypeFloat64);

										// Write value
										data.append(EndianF64_NtoB(value));

										return objectInfo;
									}
		ObjectInfo				write(CData& data, UInt64 value, UInt8 byteCount)
									{
										// Setup
										ObjectInfo	objectInfo(mObjectInfos.getCount(), data.getByteCount());
										mObjectInfos += objectInfo;

										// Write marker
										UInt8	marker;
										switch (byteCount) {
											case 1:		marker = kMarkerTypeInteger1Byte;	break;
											case 2:		marker = kMarkerTypeInteger2Bytes;	break;
											case 4:		marker = kMarkerTypeInteger4Bytes;	break;
											default:	marker = kMarkerTypeInteger8Bytes;	break;
										}
										data.append(marker);

										// Write value
										UInt64	swappedValue = EndianU64_NtoB(value);
										data.append((UInt8*) &swappedValue + sizeof(UInt64) - byteCount, byteCount);

										return objectInfo;
									}
		ObjectInfo				write(CData& data, const CString& string)
									{
										// Setup
										ObjectInfo	objectInfo(mObjectInfos.getCount(), data.getByteCount());
										mObjectInfos += objectInfo;

										// Compose string info
										OV<CData>	stringData = string.getData(CString::kEncodingASCII, true);
										UInt8		marker;
										if (stringData.hasValue())
											// ASCII
											marker = kMarkerTypeStringASCII;
										else {
											// UTF16BE
											marker = kMarkerTypeStringUnicode16;
											stringData = string.getData(CString::kEncodingUTF16BE);
										}

										// Write marker + count
										writeMarkerAndCount(data, marker,
												(marker == kMarkerTypeStringASCII) ?
														stringData->getByteCount() : stringData->getByteCount() / 2);

										// Write data
										data += *stringData;

										return objectInfo;
									}

		const	CDictionary&		mDictionary;

				UInt8				mObjectIndexByteCount;
				CDictionary			mObjectIndexByString;
				TNArray<ObjectInfo>	mObjectInfos;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBinaryPropertyList

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TVResult<CDictionary> CBinaryPropertyList::dictionaryFrom(const I<CRandomAccessDataSource>& randomAccessDataSource,
		bool composeStandardDictionary)
//----------------------------------------------------------------------------------------------------------------------
{
	return CBPLReader::getDictionary(randomAccessDataSource, composeStandardDictionary);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CDictionary> CBinaryPropertyList::dictionaryFrom(const CData& data, bool composeStandardDictionary)
//----------------------------------------------------------------------------------------------------------------------
{
	return dictionaryFrom(I<CRandomAccessDataSource>(new CDataDataSource(data)), composeStandardDictionary);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> CBinaryPropertyList::dataFrom(const CDictionary& dictionary)
//----------------------------------------------------------------------------------------------------------------------
{
	return CBPLWriter(dictionary).getData();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CBinaryPropertyList::write(const CDictionary& dictionary, const CFile& file)
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose data
	TVResult<CData>	data = dataFrom(dictionary);
	ReturnErrorIfResultError(data);

	// Open file for writing
	CFileWriter	fileWriter(file);
	OV<SError>	error = fileWriter.open(CFileWriter::kModeCreate, false, true);
	ReturnErrorIfError(error);

	// Write
	error = fileWriter.write(*data);
	ReturnErrorIfError(error);

	return fileWriter.close();
}
