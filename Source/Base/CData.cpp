//----------------------------------------------------------------------------------------------------------------------
//	CData.cpp			©2018 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CData.h"

#include "CppToolboxAssert.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

CData	CData::mEmpty;
CData	CData::mZeroByte("", 1, false);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDataInternals

class CDataInternals : public TCopyOnWriteReferenceCountable<CDataInternals> {
	public:
						CDataInternals(CDataSize initialSize, const void* initialBuffer = nil,
								bool copySourceData = true) :
							TCopyOnWriteReferenceCountable(), mFreeOnDelete(copySourceData), mBufferSize(initialSize)
							{
								// Check for initial buffer
								if (initialBuffer != nil) {
									// Check free on delete
									if (copySourceData) {
										// mBufferSize 0, initialBuffer not nil, copySourceData true
										// mBufferSize >0, initialBuffer not nil, copySourceData true
										mBuffer = ::malloc(mBufferSize);
										::memcpy(mBuffer, initialBuffer, mBufferSize);
									} else {
										// mBufferSize 0, initialBuffer not nil, copySourceData false
										// mBufferSize >0, initialBuffer not nil, copySourceData false
										mBuffer = (void*) initialBuffer;
									}
								} else if (mBufferSize > 0)
									// mBufferSize >0, initialBuffer nil
									mBuffer = ::calloc(1, mBufferSize);
								else
									// mBufferSize 0, initialBuffer nil
									mBuffer = nil;
							}
						CDataInternals(const CDataInternals& other) :
							TCopyOnWriteReferenceCountable(), mFreeOnDelete(true),
									mBuffer((other.mBufferSize > 0) ? ::malloc(other.mBufferSize) : nil),
									mBufferSize(other.mBufferSize)
							{
								// Do we have any data
								if (mBufferSize > 0)
									// Copy data
									::memcpy(mBuffer, other.mBuffer, mBufferSize);
							}
						~CDataInternals()
							{
								// Cleanup
								if (mFreeOnDelete)
									// Free!
									::free(mBuffer);
							}

		CDataInternals*	setSize(CDataSize size)
							{
								// Prepare for write
								CDataInternals*	dataInternals = prepareForWrite();

								// Update size
								dataInternals->mBufferSize = size;
								dataInternals->mBuffer = ::realloc(dataInternals->mBuffer, dataInternals->mBufferSize);

								return dataInternals;
							}

		bool		mFreeOnDelete;
		void*		mBuffer;
		CDataSize	mBufferSize;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CData

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CData::CData(CDataSize initialSize)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CDataInternals(initialSize);
}

//----------------------------------------------------------------------------------------------------------------------
CData::CData(const CData& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CData::CData(const void* buffer, CDataSize bufferSize, bool copySourceData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CDataInternals(bufferSize, buffer, copySourceData);
}

//----------------------------------------------------------------------------------------------------------------------
CData::CData(const CString& base64String)
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
	CStringLength	stringLength = base64String.getLength();
	if (stringLength == 0) {
		// No string
		mInternals = CData::mEmpty.mInternals->addReference();

		return;
	}

			SCString		cString = base64String.getCString();
	const	char*			stringPtr = *cString;
			bool			pad1 = ((stringLength % 4) != 0) || (stringPtr[stringLength - 1] == '=');
			bool			pad2 = pad1 && (((stringLength % 4) > 2) || (stringPtr[stringLength - 2] != '='));
			CStringLength	last = (stringLength - (pad1 ? 1 : 0)) / 4 << 2;

	// Setup internals
	CDataSize	dataSize = last / 4 * 3 + (pad1 ? 1 : 0) + (pad2 ? 1 : 0);
	mInternals = new CDataInternals(dataSize);

	// Convert
	UInt8*	dataPtr = (UInt8*) mInternals->mBuffer;
	for (UInt32 i = 0; i < last; i += 4) {
		// Convert these 4 characters to 3 bytes
		UInt32	bytes =
						(sMap[stringPtr[i]] << 18) | (sMap[stringPtr[i + 1]] << 12) | (sMap[stringPtr[i + 2]] << 6) |
								sMap[stringPtr[i + 3]];
		*dataPtr++ = bytes >> 16;
		*dataPtr++ = bytes >> 8 & 0xFF;
		*dataPtr++ = bytes & 0xFF;
	}

	if (pad1) {
		// Have extra bytes
		UInt32	bytes = (sMap[stringPtr[last]] << 18) | (sMap[stringPtr[last + 1]] << 12);
		*dataPtr++ = bytes >> 16;
		if (pad2) {
			// One more byte
			bytes |= sMap[stringPtr[last + 2]] << 6;
			*dataPtr++ = bytes >> 8 & 0xFF;
		}
	}
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
CDataSize CData::getSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mBufferSize;
}

//----------------------------------------------------------------------------------------------------------------------
void CData::setSize(CDataSize size)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set size
	mInternals = mInternals->setSize(size);
}

//----------------------------------------------------------------------------------------------------------------------
void CData::increaseSizeBy(CDataSize size)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update size
	mInternals = mInternals->setSize(mInternals->mBufferSize + size);
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
	mInternals = mInternals->prepareForWrite();

	return mInternals->mBuffer;
}

//----------------------------------------------------------------------------------------------------------------------
void CData::copyBytes(void* destinationBuffer, CDataByteIndex startByte, OV<CDataSize> count) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CDataSize	byteCount = count.hasValue() ? count.getValue() : getSize() - startByte;

	// Parameter check
	AssertNotNil(destinationBuffer);
	if (destinationBuffer == nil)
		return;

	AssertFailIf((startByte + byteCount) > (CDataByteIndex) getSize());
	if ((startByte + byteCount) > (CDataByteIndex) getSize())
		return;

	// Copy
	::memcpy(destinationBuffer, (UInt8*) mInternals->mBuffer + startByte, byteCount);
}

//----------------------------------------------------------------------------------------------------------------------
void CData::appendBytes(const void* buffer, CDataSize bufferSize)
//----------------------------------------------------------------------------------------------------------------------
{
	// Punt if no actual data to append
	if (bufferSize == 0)
		return;

	// Parameter check
	AssertNotNil(buffer);
	if (buffer == nil)
		return;

	// Setup
	CDataSize	originalSize = mInternals->mBufferSize;
	mInternals = mInternals->setSize(mInternals->mBufferSize + bufferSize);

	// Copy
	::memcpy((UInt8*) mInternals->mBuffer + originalSize, buffer, bufferSize);
}

//----------------------------------------------------------------------------------------------------------------------
void CData::replaceBytes(CDataByteIndex startByte, CDataSize byteCount, const void* buffer, CDataSize bufferSize)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(buffer);
	if (buffer == nil)
		return;

	AssertFailIf((startByte + byteCount) > getSize());
	if ((startByte + byteCount) > getSize())
		return;

	// Check what is happening
	CDataSize	resultSize = mInternals->mBufferSize - byteCount + bufferSize;
	if (resultSize == mInternals->mBufferSize) {
		// Overall size is staying the same
		mInternals = mInternals->prepareForWrite();
		::memcpy((UInt8*) mInternals->mBuffer + startByte, buffer, bufferSize);
	} else if (resultSize > mInternals->mBufferSize) {
		// Overall size is increasing
		// [0...startByte] stays the same
		// [startByte...startByte+byteCount] becomes [startByte...startByte+bufferSize]
		// [startByte+byteCount...end] stays the same
		mInternals = mInternals->setSize(resultSize);
		::memmove((UInt8*) mInternals->mBuffer + startByte + bufferSize,
				(UInt8*) mInternals->mBuffer + startByte + byteCount, resultSize - startByte - bufferSize);
		::memcpy((UInt8*) mInternals->mBuffer + startByte, buffer, bufferSize);
	} else {
		// Overall size is decreasing
		// [0...startByte] stays the same
		// [startByte...startByte+byteCount] becomes [startByte...startByte+bufferSize]
		// [startByte+byteCount...end] stays the same
		::memmove((UInt8*) mInternals->mBuffer + startByte + bufferSize,
				(UInt8*) mInternals->mBuffer + startByte + byteCount, resultSize - startByte - bufferSize);
		::memcpy((UInt8*) mInternals->mBuffer + startByte, buffer, bufferSize);
		mInternals = mInternals->setSize(resultSize);
	}
}

//----------------------------------------------------------------------------------------------------------------------
CString CData::getBase64String(bool prettyPrint) const
//----------------------------------------------------------------------------------------------------------------------
{
	// From http://web.mit.edu/freebsd/head/contrib/wpa/src/utils/base64.c
	static	const	char*	sTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	// Setup
	CDataSize		dataLength = mInternals->mBufferSize;
	CStringLength	stringLength = (dataLength + 2) / 3 * 4;	// 3 byte blocks to 4 characters
	if (prettyPrint)
		// Add for newlines
		stringLength += (stringLength + 71) / 72; // line feeds
	if (stringLength < dataLength)
		// Integer overflow
		return CString::mEmpty;

	// Convert
	const	UInt8*			bytePtr = (UInt8*) mInternals->mBuffer;
	const	UInt8*			endBytePtr = bytePtr + mInternals->mBufferSize;

			char*			stringBuffer[stringLength];
			char*			stringPtr = (char*) stringBuffer;

			CStringLength	currentLineLength = 0;
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

	return CString((char*) stringBuffer, stringLength, kStringEncodingUTF8);
}

//----------------------------------------------------------------------------------------------------------------------
CData& CData::operator=(const CData& other)
//----------------------------------------------------------------------------------------------------------------------
{
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
	return (mInternals->mBufferSize == other.mInternals->mBufferSize) &&
			(::memcmp(mInternals->mBuffer, other.mInternals->mBuffer, mInternals->mBufferSize) == 0);
}

//----------------------------------------------------------------------------------------------------------------------
CData CData::operator+(const CData& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Create and setup data
	CData	data(mInternals->mBufferSize + other.mInternals->mBufferSize);
	::memcpy(data.mInternals->mBuffer, mInternals->mBuffer, mInternals->mBufferSize);
	::memcpy((UInt8*) data.mInternals->mBuffer + mInternals->mBufferSize, other.mInternals->mBuffer,
			other.mInternals->mBufferSize);

	return data;
}
