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

class CDataInternals {
	public:
						CDataInternals(CDataSize initialSize, const void* initialBuffer = nil,
								bool copySourceData = true) :
							mFreeOnDelete(copySourceData), mBufferSize(initialSize), mReferenceCount(1)
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
									mBuffer = ::malloc(mBufferSize);
								else
									// mBufferSize 0, initialBuffer nil
									mBuffer = nil;
							}
						~CDataInternals()
							{
								// Cleanup
								if (mFreeOnDelete)
									// Free!
									::free(mBuffer);
							}

		CDataInternals*	addReference() { mReferenceCount++; return this; }
		void			removeReference()
							{
								// Decrement reference count and check if we are the last one
								if (--mReferenceCount == 0) {
									// We going away
									CDataInternals*	THIS = this;
									DisposeOf(THIS);
								}
							}

		CDataInternals*	prepareForWrite()
							{
								// Check reference count.  If there is more than 1 reference, we implement a
								//	"copy on write".  So we will clone ourselves so we have a personal buffer that
								//	can be changed while leaving the exiting buffer as-is for the other references.
								if (mReferenceCount > 1) {
									// Multiple references, copy
									CDataInternals*	copy = new CDataInternals(mBufferSize);
									::memcpy(copy->mBuffer, mBuffer, mBufferSize);

									// One less reference here
									mReferenceCount--;

									return copy;
								} else
									// Only a single reference
									return this;
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
		UInt32		mReferenceCount;
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
void CData::copyBytes(void* destinationBuffer, CDataByteIndex startByte, CDataSize byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(destinationBuffer);
	if (destinationBuffer == nil)
		return;

	AssertFailIf((startByte + byteCount) > (CDataByteIndex) getSize());
	if ((startByte + byteCount) > (CDataByteIndex) getSize())
		return;

	// Setup
	if (byteCount == kCDataBytesAll)
		byteCount = getSize() - startByte;

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
