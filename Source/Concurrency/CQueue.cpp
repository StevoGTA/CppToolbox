//----------------------------------------------------------------------------------------------------------------------
//	CQueue.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CQueue.h"

/*
	Inspired by
		https://www.codeproject.com/Articles/3479/The-Bip-Buffer-The-Circular-Buffer-with-a-Twist
		https://ferrous-systems.com/blog/lock-free-ring-buffer/
*/

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSRSWBIPQueueInternals

class CSRSWBIPQueueInternals {
	public:
		CSRSWBIPQueueInternals(UInt32 size) :
			mBuffer((UInt8*) ::malloc(size)), mSize(size), mReadPtr(mBuffer), mWritePtr(mBuffer),
					mWriteWatermarkPtr(mBuffer + mSize)
			{}
		~CSRSWBIPQueueInternals()
			{ ::free(mBuffer); }

		// General
		UInt8*	mBuffer;
		UInt32	mSize;

		// Reader
		UInt8*	mReadPtr;

		// Writer
		UInt8*	mWritePtr;
		UInt8*	mWriteWatermarkPtr;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSRSWBIPQueue

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CSRSWBIPQueue::CSRSWBIPQueue(UInt32 size)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CSRSWBIPQueueInternals(size);
}

//----------------------------------------------------------------------------------------------------------------------
CSRSWBIPQueue::~CSRSWBIPQueue()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CSRSWBIPQueue::SReadBufferInfo CSRSWBIPQueue::requestRead() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Capture info locally
	UInt8*	writePtr = mInternals->mWritePtr;
	UInt8*	writeWatermarkPtr = mInternals->mWriteWatermarkPtr;

	// Check stuffs
	if (writePtr > mInternals->mReadPtr)
		// Write leads, read follows
		return SReadBufferInfo(mInternals->mReadPtr, (UInt32) (writePtr - mInternals->mReadPtr));
	else if (writePtr == mInternals->mReadPtr)
		// Nothing to read
		return SReadBufferInfo();
	else
		// Can read to write watermark
		return SReadBufferInfo(mInternals->mReadPtr, (UInt32) (writeWatermarkPtr - mInternals->mReadPtr));
}

//----------------------------------------------------------------------------------------------------------------------
void CSRSWBIPQueue::commitRead(UInt32 size)
//----------------------------------------------------------------------------------------------------------------------
{
	// Capture info locally
	UInt8*	writeWatermarkPtr = mInternals->mWriteWatermarkPtr;

	// Update stuffs
	mInternals->mReadPtr += size;
	if (mInternals->mReadPtr == writeWatermarkPtr)
		// Reached the end, wrap around
		mInternals->mReadPtr = mInternals->mBuffer;
}

//----------------------------------------------------------------------------------------------------------------------
CSRSWBIPQueue::SWriteBufferInfo CSRSWBIPQueue::requestWrite(UInt32 maxSize) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Capture info locally
	UInt8*	readPtr = mInternals->mReadPtr;

	// Check situation
	if (mInternals->mWritePtr >= readPtr) {
		// Write trails read
		if ((UInt32) (mInternals->mWriteWatermarkPtr - mInternals->mWritePtr) >= maxSize)
			// Can write more from current position
			return SWriteBufferInfo(mInternals->mWritePtr, maxSize);
		else if ((UInt32) (readPtr - mInternals->mBuffer) >= maxSize) {
			// Must start at beginning of the buffer now
			mInternals->mWriteWatermarkPtr = mInternals->mWritePtr;
			mInternals->mWritePtr = mInternals->mBuffer;

			return SWriteBufferInfo(mInternals->mWritePtr, maxSize);
		} else
			// Not enough space
			return SWriteBufferInfo();
	} else {
		// Write preceeds read
		if ((UInt32) (readPtr - mInternals->mWritePtr) >= maxSize)
			// Can write more from current position
			return SWriteBufferInfo(mInternals->mWritePtr, maxSize);
		else
			// Not enough space
			return SWriteBufferInfo();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CSRSWBIPQueue::commitWrite(UInt32 size)
//----------------------------------------------------------------------------------------------------------------------
{
	// Capture info locally
	UInt8*	readPtr = mInternals->mReadPtr;

	// Update stuffs
	if (mInternals->mWritePtr >= readPtr)
		// Reset watermark
		mInternals->mWriteWatermarkPtr = mInternals->mBuffer + mInternals->mSize;
	mInternals->mWritePtr += size;
	if (mInternals->mWritePtr == mInternals->mWriteWatermarkPtr)
		// Back to beginning
		mInternals->mWritePtr = mInternals->mBuffer;
}

//----------------------------------------------------------------------------------------------------------------------
void CSRSWBIPQueue::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset
	mInternals->mReadPtr = mInternals->mBuffer;

	mInternals->mWritePtr = mInternals->mBuffer;
	mInternals->mWriteWatermarkPtr = mInternals->mBuffer + mInternals->mSize;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSRSWBIPSegmentedQueueInternals

class CSRSWBIPSegmentedQueueInternals {
	public:
		CSRSWBIPSegmentedQueueInternals(UInt32 segmentSize, UInt32 segmentCount) :
			mBuffer((UInt8*) ::malloc(segmentSize * segmentCount)), mSegmentSize(segmentSize),
					mSegmentCount(segmentCount), mReadPtr(mBuffer), mWritePtr(mBuffer),
					mWriteWatermarkPtr(mBuffer + mSegmentSize)
			{}
		~CSRSWBIPSegmentedQueueInternals()
			{ ::free(mBuffer); }

		// General
		UInt8*	mBuffer;
		UInt32	mSegmentSize;
		UInt32	mSegmentCount;

		// Reader
		UInt8*	mReadPtr;

		// Writer
		UInt8*	mWritePtr;
		UInt8*	mWriteWatermarkPtr;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSRSWBIPSegmentedQueue

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CSRSWBIPSegmentedQueue::CSRSWBIPSegmentedQueue(UInt32 segmentSize, UInt32 segmentCount)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CSRSWBIPSegmentedQueueInternals(segmentSize, segmentCount);
}

//----------------------------------------------------------------------------------------------------------------------
CSRSWBIPSegmentedQueue::~CSRSWBIPSegmentedQueue()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UInt32 CSRSWBIPSegmentedQueue::getSegmentCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mSegmentCount;
}

//----------------------------------------------------------------------------------------------------------------------
CSRSWBIPSegmentedQueue::SReadBufferInfo CSRSWBIPSegmentedQueue::requestRead() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Capture info locally
	UInt8*	writePtr = mInternals->mWritePtr;
	UInt8*	writeWatermarkPtr = mInternals->mWriteWatermarkPtr;

	// Check stuffs
	if (writePtr > mInternals->mReadPtr)
		// Can read to write pointer
		return SReadBufferInfo(mInternals->mReadPtr, mInternals->mSegmentSize,
				(UInt32) (writePtr - mInternals->mReadPtr));
	else if (writePtr < mInternals->mReadPtr)
		// Can read to write watermark pointer
		return SReadBufferInfo(mInternals->mReadPtr, mInternals->mSegmentSize,
				(UInt32) (writeWatermarkPtr - mInternals->mReadPtr));
	else
		// Queue is empty
		return SReadBufferInfo();
}

//----------------------------------------------------------------------------------------------------------------------
void CSRSWBIPSegmentedQueue::commitRead(UInt32 size)
//----------------------------------------------------------------------------------------------------------------------
{
	// Capture info locally
	UInt8*	writeWatermarkPtr = mInternals->mWriteWatermarkPtr;

	// Update stuffs
	mInternals->mReadPtr += size;
	if (mInternals->mReadPtr == writeWatermarkPtr)
		// Reached the end, wrap around
		mInternals->mReadPtr = mInternals->mBuffer;
}

//----------------------------------------------------------------------------------------------------------------------
CSRSWBIPSegmentedQueue::SWriteBufferInfo CSRSWBIPSegmentedQueue::requestWrite(UInt32 maxSize) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Capture info locally
	UInt8*	readPtr = mInternals->mReadPtr;

	// Check situation
	if (mInternals->mWritePtr >= readPtr) {
		// Reset watermark
		mInternals->mWriteWatermarkPtr = mInternals->mBuffer + mInternals->mSegmentSize;

		// Can write up to end of buffer, or set watermark and write to beginning of buffer
		if ((UInt32) (mInternals->mWriteWatermarkPtr - mInternals->mWritePtr) >= maxSize)
			// Can write more from current position
			return SWriteBufferInfo(mInternals->mWritePtr, mInternals->mSegmentSize, maxSize);
		else if ((UInt32) (readPtr - mInternals->mBuffer) >= maxSize) {
			// Must start at beginning of the buffer now
			mInternals->mWriteWatermarkPtr = mInternals->mWritePtr;
			mInternals->mWritePtr = mInternals->mBuffer;

			return SWriteBufferInfo(mInternals->mWritePtr, mInternals->mSegmentSize, maxSize);
		} else
			// Not enough space
			return SWriteBufferInfo();
	} else {
		// Write pointer is before read pointer
		if ((UInt32) (readPtr - mInternals->mWritePtr) >= maxSize)
			// Can write more from current position
			return SWriteBufferInfo(mInternals->mWritePtr, mInternals->mSegmentSize, maxSize);
		else
			// Not enough space
			return SWriteBufferInfo();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CSRSWBIPSegmentedQueue::commitWrite(UInt32 size)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update stuffs
	mInternals->mWritePtr += size;
	if (mInternals->mWritePtr == mInternals->mWriteWatermarkPtr)
		// Back to beginning
		mInternals->mWritePtr = mInternals->mBuffer;
}

//----------------------------------------------------------------------------------------------------------------------
void CSRSWBIPSegmentedQueue::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset
	mInternals->mReadPtr = mInternals->mBuffer;

	mInternals->mWritePtr = mInternals->mBuffer;
	mInternals->mWriteWatermarkPtr = mInternals->mBuffer + mInternals->mSegmentSize;
}
