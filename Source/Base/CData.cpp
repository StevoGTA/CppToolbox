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

const	CData	CData::mEmpty(0);
const	CData	CData::mZeroByte("", 1, false);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CData::Internals

class CData::Internals : public TCopyOnWriteReferenceCountable<Internals> {
	public:
				Internals(CData::ByteCount preallocatedByteCount) :
					TCopyOnWriteReferenceCountable(),
							mOwnsBuffer(true), mBuffer(::calloc(1, (size_t) preallocatedByteCount)),
							mBufferAllocatedByteCount(preallocatedByteCount),
							mBufferUsedByteCount(0)
					{}
				Internals(const void* initialBuffer, CData::ByteCount byteCount, bool copySourceData) :
					TCopyOnWriteReferenceCountable(),
							mOwnsBuffer(copySourceData), mBufferAllocatedByteCount(byteCount),
							mBufferUsedByteCount(byteCount)
					{
						// Check if copying source data
						if (copySourceData) {
							// Copy and manage
							mBuffer = ::malloc((size_t) mBufferAllocatedByteCount);
							::memcpy(mBuffer, initialBuffer, (size_t) mBufferUsedByteCount);
						} else
							// Reference existing buffer
							mBuffer = (void*) initialBuffer;
					}
				Internals(const Internals& other) :
					TCopyOnWriteReferenceCountable(), mOwnsBuffer(true),
							mBuffer(::malloc((size_t) other.mBufferAllocatedByteCount)),
							mBufferAllocatedByteCount(other.mBufferAllocatedByteCount),
							mBufferUsedByteCount(other.mBufferUsedByteCount)
					{
						// Copy data
						::memcpy(mBuffer, other.mBuffer, (size_t) mBufferUsedByteCount);
					}
				~Internals()
					{
						// Cleanup
						if (mOwnsBuffer)
							// Free!
							::free(mBuffer);
					}

		void	reallocate(CData::ByteCount byteCount)
					{
						// Update buffer
						if (mOwnsBuffer) {
							// Can just realloc the buffer
							if (byteCount > mBufferAllocatedByteCount) {
								// Add space
								if (byteCount < (mBufferAllocatedByteCount + 1024)) {
									// Increase by 1024 bytes
									mBufferAllocatedByteCount += 1024;
									mBuffer = ::realloc(mBuffer, (size_t) mBufferAllocatedByteCount);
								} else {
									// Use requred byte count
									mBufferAllocatedByteCount = byteCount;
									mBuffer = ::realloc(mBuffer, (size_t) mBufferAllocatedByteCount);
								}
							}
						} else {
							// Alloc new buffer
							void*	buffer = ::malloc((size_t) byteCount);
							::memcpy(buffer, mBuffer, mBufferUsedByteCount);
							mOwnsBuffer = true;
							mBuffer = buffer;
							mBufferAllocatedByteCount = byteCount;
						}
					}

		bool				mOwnsBuffer;
		void*				mBuffer;
		CData::ByteCount	mBufferAllocatedByteCount;
		CData::ByteCount	mBufferUsedByteCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CData

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CData::CData(ByteCount preallocatedByteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals(preallocatedByteCount);
}

//----------------------------------------------------------------------------------------------------------------------
CData::CData(ByteCount byteCount, UInt8 fillValue)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals(byteCount);

	// Fill
	::memset(mInternals->mBuffer, fillValue, byteCount);
	mInternals->mBufferUsedByteCount = byteCount;
}

//----------------------------------------------------------------------------------------------------------------------
CData::CData(const void* buffer, ByteCount bufferByteCount, bool copySourceData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals(buffer, bufferByteCount, copySourceData);
}

//----------------------------------------------------------------------------------------------------------------------
CData::CData(const CData& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = other.mInternals->addReference();
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
	return mInternals->mBufferUsedByteCount;
}

//----------------------------------------------------------------------------------------------------------------------
const void* CData::getBytePtr() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mBuffer;
}

//----------------------------------------------------------------------------------------------------------------------
void CData::copyBytes(void* destinationBuffer, ByteIndex startByteIndex, ByteCount byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(destinationBuffer);
	if (destinationBuffer == nil)
		return;

	AssertFailIf((startByteIndex + byteCount) > (ByteIndex) mInternals->mBufferUsedByteCount);
	if ((startByteIndex + byteCount) > (ByteIndex) mInternals->mBufferUsedByteCount)
		return;

	// Copy
	::memcpy(destinationBuffer, (UInt8*) mInternals->mBuffer + startByteIndex, (size_t) byteCount);
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
			TBuffer<char>	buffer(mInternals->mBufferUsedByteCount * 2);
	const	UInt8*			bytePtr = (const UInt8*) getBytePtr();
	const	char*			table = uppercase ? sTableUppercase : sTableLowercase;
	for (ByteIndex i = 0; i <  mInternals->mBufferUsedByteCount; i++, bytePtr++) {
		// Store
		buffer[i * 2] = table[(*bytePtr & 0xF0) >> 4];
		buffer[i * 2 + 1] = table[*bytePtr & 0x0F];
	}

	return CString((const void*) *buffer, mInternals->mBufferUsedByteCount* 2, CString::kEncodingASCII);
}

//----------------------------------------------------------------------------------------------------------------------
CString CData::getBase64String(bool prettyPrint) const
//----------------------------------------------------------------------------------------------------------------------
{
	// From http://web.mit.edu/freebsd/head/contrib/wpa/src/utils/base64.c
	static	const	char*	sTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	// Setup
	ByteCount		dataByteCount = mInternals->mBufferUsedByteCount;
	CString::Length	stringLength = (CString::Length) (dataByteCount + 2) / 3 * 4;	// 3 byte blocks to 4 characters
	if (prettyPrint)
		// Add for newlines
		stringLength += (stringLength + 71) / 72; // line feeds
	if (stringLength < dataByteCount)
		// Integer overflow
		return CString::mEmpty;

	// Convert
	const	UInt8*			bytePtr = (UInt8*) mInternals->mBuffer;
	const	UInt8*			endBytePtr = bytePtr + mInternals->mBufferUsedByteCount;

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

	return CString((const void*) *stringBuffer, stringLength, CString::kEncodingASCII);
}

//----------------------------------------------------------------------------------------------------------------------
CData CData::subData(ByteIndex byteIndex, ByteCount byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertFailIf((byteIndex + byteCount) >= mInternals->mBufferUsedByteCount);
	if ((byteIndex + byteCount) >= mInternals->mBufferUsedByteCount)
		return mEmpty;

	return (byteIndex == 0) ?
			CData((UInt8*) mInternals->mBuffer, byteCount, false) :
			CData((UInt8*) mInternals->mBuffer + byteIndex, byteCount);
}

//----------------------------------------------------------------------------------------------------------------------
CData CData::subData(ByteIndex byteIndex) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertFailIf(byteIndex >= mInternals->mBufferUsedByteCount);
	if (byteIndex >= mInternals->mBufferUsedByteCount)
		return mEmpty;

	return (byteIndex == 0) ?
			CData(*this) :
			CData((UInt8*) mInternals->mBuffer + byteIndex, mInternals->mBufferUsedByteCount - byteIndex);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SRange64> CData::findSubData(const CData& subData, ByteIndex startIndex, const OV<ByteCount>& byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertFailIf(subData.mInternals->mBufferUsedByteCount == 0);
	if (subData.mInternals->mBufferUsedByteCount == 0)
		return OV<SRange64>();

	AssertFailIf(startIndex >= mInternals->mBufferUsedByteCount);
	if (startIndex >= mInternals->mBufferUsedByteCount)
		return OV<SRange64>();

	AssertFailIf(byteCount.hasValue() && ((startIndex + *byteCount) >= mInternals->mBufferUsedByteCount));
	if (byteCount.hasValue() && ((startIndex + *byteCount) >= mInternals->mBufferUsedByteCount))
		return OV<SRange64>();

	// Setup
	ByteCount	byteCount_ = byteCount.hasValue() ? *byteCount : mInternals->mBufferUsedByteCount - startIndex;

	// Search
	while ((startIndex < mInternals->mBufferUsedByteCount) &&
			((startIndex + subData.mInternals->mBufferUsedByteCount) < mInternals->mBufferUsedByteCount)) {
		// Look for first byte
		const	void*	ptr =
								::memchr((const char*) mInternals->mBuffer + startIndex,
										*((const char*) subData.getBytePtr()), byteCount_);
		if (ptr == nil)
			// Not found
			return OV<SRange64>();

		// Check if data matches
		int	result =
					::memcmp((const char*) mInternals->mBuffer + startIndex, subData.mInternals->mBuffer,
							subData.mInternals->mBufferUsedByteCount);
		if (result == 0)
			// Found
			return OV<SRange64>(SRange64(startIndex, subData.mInternals->mBufferUsedByteCount));

		// Start with next byte
		startIndex++;
	}

	return OV<SRange64>();
}

//----------------------------------------------------------------------------------------------------------------------
TBuffer<UInt8> CData::getMutableBuffer(ByteIndex byteIndex, ByteCount byteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	Internals::prepareForWrite(&mInternals);

	// Ensure we have space
	mInternals->reallocate(byteIndex + byteCount);

	// Update used byte count
	mInternals->mBufferUsedByteCount = byteIndex + byteCount;

	return TBuffer<UInt8>((UInt8*) mInternals->mBuffer + byteIndex, byteCount);
}

//----------------------------------------------------------------------------------------------------------------------
TBuffer<UInt8> CData::getMutableBuffer(ByteCount byteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	Internals::prepareForWrite(&mInternals);

	// Ensure we have space
	mInternals->reallocate(byteCount);

	// Update used byte count
	mInternals->mBufferUsedByteCount = byteCount;

	return TBuffer<UInt8>((UInt8*) mInternals->mBuffer, byteCount);
}

//----------------------------------------------------------------------------------------------------------------------
CData& CData::append(const void* buffer, ByteCount bufferByteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Punt if no actual data to append
	if (bufferByteCount == 0)
		return  *this;

	// Parameter check
	AssertNotNil(buffer);
	if (buffer == nil)
		return  *this;

	// Prepare for write
	Internals::prepareForWrite(&mInternals);

	// Ensure we have space
	mInternals->reallocate(mInternals->mBufferUsedByteCount + bufferByteCount);

	// Append
	::memcpy((UInt8*) mInternals->mBuffer + mInternals->mBufferUsedByteCount, buffer, (size_t) bufferByteCount);
	mInternals->mBufferUsedByteCount += bufferByteCount;

	return  *this;
}

//----------------------------------------------------------------------------------------------------------------------
CData& CData::replace(ByteIndex startByteIndex, ByteCount byteCount, const void* buffer, ByteCount bufferByteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(buffer);
	if (buffer == nil)
		return  *this;

	AssertFailIf((startByteIndex + byteCount) > mInternals->mBufferUsedByteCount);
	if ((startByteIndex + byteCount) > mInternals->mBufferUsedByteCount)
		return  *this;

	// Prepare for write
	Internals::prepareForWrite(&mInternals);

	// Check what is happening
	if (byteCount == bufferByteCount)
		// Straight replace
		::memcpy((UInt8*) mInternals->mBuffer + startByteIndex, buffer, (size_t) bufferByteCount);
	else if (byteCount > bufferByteCount) {
		// Overall byte count is decreasing
		// [0...startByteIndex] stays the same
		// [startByteIndex...startByteIndex+byteCount] becomes [startByteIndex...startByteIndex+bufferByteCount]
		// [startByteIndex+byteCount...end] stays the same
		ByteCount	newUsedByteCount = mInternals->mBufferUsedByteCount - byteCount + bufferByteCount;

		::memcpy((UInt8*) mInternals->mBuffer + startByteIndex, buffer, (size_t) bufferByteCount);
		::memmove((UInt8*) mInternals->mBuffer + startByteIndex + bufferByteCount,
				(UInt8*) mInternals->mBuffer + startByteIndex + byteCount,
				(size_t) (newUsedByteCount - startByteIndex - bufferByteCount));
		mInternals->mBufferUsedByteCount = newUsedByteCount;
	} else {
		// Overall byte count is increasing
		// [0...startByteIndex] stays the same
		// [startByteIndex...startByteIndex+byteCount] becomes [startByteIndex...startByteIndex+bufferByteCount]
		// [startByteIndex+byteCount...end] stays the same
		ByteCount	newUsedByteCount = mInternals->mBufferUsedByteCount - byteCount + bufferByteCount;

		mInternals->reallocate(newUsedByteCount);
		::memmove((UInt8*) mInternals->mBuffer + startByteIndex + bufferByteCount,
				(UInt8*) mInternals->mBuffer + startByteIndex + byteCount,
				(size_t) (newUsedByteCount - startByteIndex - bufferByteCount));
		::memcpy((UInt8*) mInternals->mBuffer + startByteIndex, buffer, (size_t) bufferByteCount);
		mInternals->mBufferUsedByteCount = newUsedByteCount;
	}

	return  *this;
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
	return (mInternals->mBufferUsedByteCount == other.mInternals->mBufferUsedByteCount) &&
			(::memcmp(mInternals->mBuffer, other.mInternals->mBuffer, (size_t) mInternals->mBufferUsedByteCount) == 0);
}

//----------------------------------------------------------------------------------------------------------------------
CData CData::operator+(const CData& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Create and setup data
	CData	data(mInternals->mBufferUsedByteCount + other.mInternals->mBufferUsedByteCount);
	::memcpy(data.mInternals->mBuffer, mInternals->mBuffer, (size_t) mInternals->mBufferUsedByteCount);
	::memcpy((UInt8*) data.mInternals->mBuffer + mInternals->mBufferUsedByteCount, other.mInternals->mBuffer,
			(size_t) other.mInternals->mBufferUsedByteCount);
	data.mInternals->mBufferUsedByteCount = mInternals->mBufferUsedByteCount + other.mInternals->mBufferUsedByteCount;

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

			CString::C		cString = base64String.getUTF8String();
	const	char*			stringPtr = *cString;
			bool			pad1 = ((stringLength % 4) != 0) || (stringPtr[stringLength - 1] == '=');
			bool			pad2 = pad1 && (((stringLength % 4) > 2) || (stringPtr[stringLength - 2] != '='));
			CString::Length	last = (stringLength - (pad1 ? 1 : 0)) / 4 << 2;

	// Setup internals
	ByteCount	dataByteCount = (ByteCount) last / 4 * 3 + (pad1 ? 1 : 0) + (pad2 ? 1 : 0);
	CData		data(dataByteCount);

	// Convert
	UInt8*	dataPtr = *data.getMutableBuffer(dataByteCount);
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
