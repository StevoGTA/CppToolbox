//----------------------------------------------------------------------------------------------------------------------
//	CData.cpp			©2018 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CData.h"

#include "CppToolboxAssert.h"
#include "CReferenceCountable.h"
#include "CString.h"
#include "TBuffer.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

const	CData	CData::mEmpty;
const	CData	CData::mZeroByte("", 1, false);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CData::Internals

class CData::Internals : public TCopyOnWriteReferenceCountable<Internals> {
	public:
				Internals(CData::ByteCount initialByteCount, const void* initialBuffer = nil,
						bool copySourceData = true) :
					TCopyOnWriteReferenceCountable(),
							mFreeOnDelete(copySourceData), mBufferByteCount(initialByteCount)
					{
						// Check for initial buffer
						if (initialBuffer != nil) {
							// Check free on delete
							if (copySourceData) {
								// mBufferByteCount 0, initialBuffer not nil, copySourceData true
								// mBufferByteCount >0, initialBuffer not nil, copySourceData true
								mBuffer = ::malloc((size_t) mBufferByteCount);
								::memcpy(mBuffer, initialBuffer, (size_t) mBufferByteCount);
							} else {
								// mBufferByteCount 0, initialBuffer not nil, copySourceData false
								// mBufferByteCount >0, initialBuffer not nil, copySourceData false
								mBuffer = (void*) initialBuffer;
							}
						} else if (mBufferByteCount > 0)
							// mBufferByteCount >0, initialBuffer nil
							mBuffer = ::calloc(1, (size_t) mBufferByteCount);
						else
							// mBufferByteCount 0, initialBuffer nil
							mBuffer = nil;
					}
				Internals(const Internals& other) :
					TCopyOnWriteReferenceCountable(), mFreeOnDelete(true),
							mBuffer(
									(other.mBufferByteCount > 0) ?
											::malloc((size_t) other.mBufferByteCount) : nil),
							mBufferByteCount(other.mBufferByteCount)
					{
						// Do we have any data
						if (mBufferByteCount > 0)
							// Copy data
							::memcpy(mBuffer, other.mBuffer, (size_t) mBufferByteCount);
					}
				~Internals()
					{
						// Cleanup
						if (mFreeOnDelete)
							// Free!
							::free(mBuffer);
					}

		void	setByteCount(CData::ByteCount byteCount)
					{
						// Update buffer
						if (mFreeOnDelete)
							// Can just realloc the buffer
							mBuffer = ::realloc(mBuffer, (size_t) byteCount);
						else {
							// Alloc new buffer
							void*	buffer = ::malloc((size_t) byteCount);
							::memcpy(buffer, mBuffer, mBufferByteCount);
							mBuffer = buffer;
						}

						// Update byte count
						mBufferByteCount = byteCount;
					}

		bool				mFreeOnDelete;
		void*				mBuffer;
		CData::ByteCount	mBufferByteCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CData

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CData::CData(ByteCount initialByteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals(initialByteCount);
}

//----------------------------------------------------------------------------------------------------------------------
CData::CData(const CData& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CData::CData(const void* buffer, ByteCount bufferByteCount, bool copySourceData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals(bufferByteCount, buffer, copySourceData);
}

//----------------------------------------------------------------------------------------------------------------------
CData::CData(SInt8 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals(sizeof(SInt8), &value);
}

//----------------------------------------------------------------------------------------------------------------------
CData::CData(UInt8 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals(sizeof(UInt8), &value);
}

//----------------------------------------------------------------------------------------------------------------------
CData::~CData()
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove reference
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CData::ByteCount CData::getByteCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mBufferByteCount;
}

//----------------------------------------------------------------------------------------------------------------------
void CData::setByteCount(ByteCount byteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	Internals::prepareForWrite(&mInternals);

	// Set byte count
	mInternals->setByteCount(byteCount);
}

//----------------------------------------------------------------------------------------------------------------------
void CData::increaseByteCountBy(ByteCount byteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	Internals::prepareForWrite(&mInternals);

	// Update byte count
	mInternals->setByteCount(mInternals->mBufferByteCount + byteCount);
}

//----------------------------------------------------------------------------------------------------------------------
const void* CData::getBytePtr() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mBuffer;
}

//----------------------------------------------------------------------------------------------------------------------
void* CData::getMutableBytePtr()
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	Internals::prepareForWrite(&mInternals);

	return mInternals->mBuffer;
}

//----------------------------------------------------------------------------------------------------------------------
void CData::copyBytes(void* destinationBuffer, ByteIndex startByteIndex, OV<ByteCount> byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	ByteCount	byteCountUse = byteCount.hasValue() ? *byteCount : getByteCount() - startByteIndex;

	// Parameter check
	AssertNotNil(destinationBuffer);
	if (destinationBuffer == nil)
		return;

	AssertFailIf((startByteIndex + byteCountUse) > (ByteIndex) getByteCount());
	if ((startByteIndex + byteCountUse) > (ByteIndex) getByteCount())
		return;

	// Copy
	::memcpy(destinationBuffer, (UInt8*) mInternals->mBuffer + startByteIndex, (size_t) byteCountUse);
}

//----------------------------------------------------------------------------------------------------------------------
void CData::appendBytes(const void* buffer, ByteCount bufferByteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Punt if no actual data to append
	if (bufferByteCount == 0)
		return;

	// Parameter check
	AssertNotNil(buffer);
	if (buffer == nil)
		return;

	// Prepare for write
	Internals::prepareForWrite(&mInternals);

	// Setup
	ByteCount	originalByteCount = mInternals->mBufferByteCount;
	mInternals->setByteCount(mInternals->mBufferByteCount + bufferByteCount);

	// Copy
	::memcpy((UInt8*) mInternals->mBuffer + originalByteCount, buffer, (size_t) bufferByteCount);
}

//----------------------------------------------------------------------------------------------------------------------
void CData::replaceBytes(ByteIndex startByteIndex, ByteCount byteCount, const void* buffer, ByteCount bufferByteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(buffer);
	if (buffer == nil)
		return;

	AssertFailIf((startByteIndex + byteCount) > getByteCount());
	if ((startByteIndex + byteCount) > getByteCount())
		return;

	// Prepare for write
	Internals::prepareForWrite(&mInternals);

	// Check what is happening
	ByteCount	resultByteCount = mInternals->mBufferByteCount - byteCount + bufferByteCount;
	if (resultByteCount == mInternals->mBufferByteCount)
		// Overall byte count is staying the same
		::memcpy((UInt8*) mInternals->mBuffer + startByteIndex, buffer, (size_t) bufferByteCount);
	else if (resultByteCount > mInternals->mBufferByteCount) {
		// Overall byte count is increasing
		// [0...startByteIndex] stays the same
		// [startByteIndex...startByteIndex+byteCount] becomes [startByteIndex...startByteIndex+bufferByteCount]
		// [startByteIndex+byteCount...end] stays the same
		mInternals->setByteCount(resultByteCount);
		::memmove((UInt8*) mInternals->mBuffer + startByteIndex + bufferByteCount,
				(UInt8*) mInternals->mBuffer + startByteIndex + byteCount,
				(size_t) (resultByteCount - startByteIndex - bufferByteCount));
		::memcpy((UInt8*) mInternals->mBuffer + startByteIndex, buffer, (size_t) bufferByteCount);
	} else {
		// Overall byte count is decreasing
		// [0...startByteIndex] stays the same
		// [startByteIndex...startByteIndex+byteCount] becomes [startByteIndex...startByteIndex+bufferByteCount]
		// [startByteIndex+byteCount...end] stays the same
		::memmove((UInt8*) mInternals->mBuffer + startByteIndex + bufferByteCount,
				(UInt8*) mInternals->mBuffer + startByteIndex + byteCount,
				(size_t) (resultByteCount - startByteIndex - bufferByteCount));
		::memcpy((UInt8*) mInternals->mBuffer + startByteIndex, buffer, (size_t) bufferByteCount);
		mInternals->setByteCount(resultByteCount);
	}
}

//----------------------------------------------------------------------------------------------------------------------
CString CData::getHexString(bool uppercase) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Statics
	static	const	char	sTableLowercase[] =
									{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
	static	const	char	sTableUppercase[] =
									{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

	// Setup
			TBuffer<char>	buffer((UInt32) getByteCount() * 2);
	const	UInt8*			bytePtr = (const UInt8*) getBytePtr();
	const	char*			table = uppercase ? sTableUppercase : sTableLowercase;
	for (UInt32 i = 0; i < (UInt32) getByteCount(); i++, bytePtr++) {
		// Store
		buffer[i * 2] = table[(*bytePtr & 0xF0) >> 4];
		buffer[i * 2 + 1] = table[*bytePtr & 0x0F];
	}

	return CString(*buffer, (CString::Length) getByteCount() * 2);
}

//----------------------------------------------------------------------------------------------------------------------
CString CData::getBase64String(bool prettyPrint) const
//----------------------------------------------------------------------------------------------------------------------
{
	// From http://web.mit.edu/freebsd/head/contrib/wpa/src/utils/base64.c
	static	const	char*	sTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	// Setup
	ByteCount		dataByteCount = mInternals->mBufferByteCount;
	CString::Length	stringLength = (CString::Length) (dataByteCount + 2) / 3 * 4;	// 3 byte blocks to 4 characters
	if (prettyPrint)
		// Add for newlines
		stringLength += (stringLength + 71) / 72; // line feeds
	if (stringLength < dataByteCount)
		// Integer overflow
		return CString::mEmpty;

	// Convert
	const	UInt8*			bytePtr = (UInt8*) mInternals->mBuffer;
	const	UInt8*			endBytePtr = bytePtr + mInternals->mBufferByteCount;

			TBuffer<char>	stringBuffer(stringLength);
			char*			stringPtr = *stringBuffer;

			CString::Length	currentLineLength = 0;
	while ((endBytePtr - bytePtr) >= 3) {
		// Convert the next 3 bytes to 4 characters
		*stringPtr++ = sTable[bytePtr[0] >> 2];
		*stringPtr++ = sTable[((bytePtr[0] & 0x03) << 4) | (bytePtr[1] >> 4)];
		*stringPtr++ = sTable[((bytePtr[1] & 0x0f) << 2) | (bytePtr[2] >> 6)];
		*stringPtr++ = sTable[bytePtr[2] & 0x3f];

		// Update
		bytePtr += 3;
		currentLineLength += 4;
		if (prettyPrint && (currentLineLength >= 72)) {
			// Add newline
			*stringPtr++ = '\n';
			currentLineLength = 0;
		}
	}

	if ((endBytePtr - bytePtr) > 0) {
		// Convert last 1 or 2 bytes
		*stringPtr++ = sTable[bytePtr[0] >> 2];
		if (endBytePtr - bytePtr == 1) {
			// Convert last 1 byte
			*stringPtr++ = sTable[(bytePtr[0] & 0x03) << 4];
			*stringPtr++ = '=';
			*stringPtr++ = '=';
		} else {
			// Convert last 2 bytes
			*stringPtr++ = sTable[((bytePtr[0] & 0x03) << 4) |
			(bytePtr[1] >> 4)];
			*stringPtr++ = sTable[(bytePtr[1] & 0x0f) << 2];
			*stringPtr++ = '=';
		}

		// Update
		currentLineLength += 4;
	}

	if (prettyPrint && (currentLineLength > 0))
		// Add newline
		*stringPtr++ = '\n';

	return CString(*stringBuffer, stringLength, CString::kEncodingUTF8);
}

//----------------------------------------------------------------------------------------------------------------------
CData CData::subData(ByteIndex byteIndex, const OV<ByteCount>& byteCount, bool copySourceData) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertFailIf(byteIndex >= mInternals->mBufferByteCount);
	if (byteIndex >= mInternals->mBufferByteCount)
		return mEmpty;

	AssertFailIf(byteCount.hasValue() && (byteIndex + *byteCount) >= mInternals->mBufferByteCount);
	if (byteCount.hasValue() && (byteIndex + *byteCount) >= mInternals->mBufferByteCount)
		return mEmpty;

	return CData((UInt8*) mInternals->mBuffer + byteIndex,
			byteCount.hasValue() ? *byteCount : mInternals->mBufferByteCount - byteIndex, copySourceData);
}

//----------------------------------------------------------------------------------------------------------------------
CData& CData::operator=(const CData& other)
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
bool CData::operator==(const CData& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Compare
	return (mInternals->mBufferByteCount == other.mInternals->mBufferByteCount) &&
			(::memcmp(mInternals->mBuffer, other.mInternals->mBuffer, (size_t) mInternals->mBufferByteCount) == 0);
}

//----------------------------------------------------------------------------------------------------------------------
CData CData::operator+(const CData& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Create and setup data
	CData	data(mInternals->mBufferByteCount + other.mInternals->mBufferByteCount);
	::memcpy(data.mInternals->mBuffer, mInternals->mBuffer, (size_t) mInternals->mBufferByteCount);
	::memcpy((UInt8*) data.mInternals->mBuffer + mInternals->mBufferByteCount, other.mInternals->mBuffer,
			(size_t) other.mInternals->mBufferByteCount);

	return data;
}

//----------------------------------------------------------------------------------------------------------------------
CData CData::fromBase64String(const CString& base64String)
//----------------------------------------------------------------------------------------------------------------------
{
	// From https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c/13935718
	static	const	UInt8	sMap[256] =
									{
										0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
										0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
										0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  62, 63, 62, 62, 63,
										52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0,  0,  0,  0,  0,  0,
										0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
										15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0,  63,
										0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
										41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
									};

	// Setup
	CString::Length	stringLength = base64String.getLength();
	if (stringLength == 0)
		// No string
		return CData();

			CString::C		cString = base64String.getCString();
	const	char*			stringPtr = *cString;
			bool			pad1 = ((stringLength % 4) != 0) || (stringPtr[stringLength - 1] == '=');
			bool			pad2 = pad1 && (((stringLength % 4) > 2) || (stringPtr[stringLength - 2] != '='));
			CString::Length	last = (stringLength - (pad1 ? 1 : 0)) / 4 << 2;

	// Setup internals
	ByteCount	dataByteCount = (ByteCount) last / 4 * 3 + (pad1 ? 1 : 0) + (pad2 ? 1 : 0);
	CData		data(dataByteCount);

	// Convert
	UInt8*	dataPtr = (UInt8*) data.getMutableBytePtr();
	for (UInt32 i = 0; i < last; i += 4) {
		// Convert these 4 characters to 3 bytes
		UInt32	bytes =
						(sMap[stringPtr[i]] << 18) | (sMap[stringPtr[i + 1]] << 12) | (sMap[stringPtr[i + 2]] << 6) |
								sMap[stringPtr[i + 3]];
		*dataPtr++ = (UInt8) (bytes >> 16);
		*dataPtr++ = bytes >> 8 & 0xFF;
		*dataPtr++ = bytes & 0xFF;
	}

	if (pad1) {
		// Have extra bytes
		UInt32	bytes = (sMap[stringPtr[last]] << 18) | (sMap[stringPtr[last + 1]] << 12);
		*dataPtr++ = (UInt8) (bytes >> 16);
		if (pad2) {
			// One more byte
			bytes |= sMap[stringPtr[last + 2]] << 6;
			*dataPtr++ = bytes >> 8 & 0xFF;
		}
	}

	return data;
}
