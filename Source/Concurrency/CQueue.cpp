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

class CSRSWBIPQueueInternals : public TCopyOnWriteReferenceCountable<CSRSWBIPQueueInternals> {
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
CSRSWBIPQueue::CSRSWBIPQueue(const CSRSWBIPQueue& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CSRSWBIPQueue::~CSRSWBIPQueue()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CSRSWBIPQueue::ReadBufferInfo CSRSWBIPQueue::requestRead() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Capture info locally
	UInt8*	writePtr = mInternals->mWritePtr;
	UInt8*	writeWatermarkPtr = mInternals->mWriteWatermarkPtr;

	// Check if time to go back to the beginning
	if (mInternals->mReadPtr == writeWatermarkPtr)
		// Reached the end, wrap around
		mInternals->mReadPtr = mInternals->mBuffer;

	// Check stuffs
	if (writePtr > mInternals->mReadPtr)
		// Can read to write pointer
		return ReadBufferInfo(mInternals->mReadPtr, (UInt32) (writePtr - mInternals->mReadPtr));
	else if (writePtr < mInternals->mReadPtr)
		// Can read to write watermark pointer
		return ReadBufferInfo(mInternals->mReadPtr, (UInt32) (writeWatermarkPtr - mInternals->mReadPtr));
	else
		// Queue is empty
		return ReadBufferInfo();
}

//----------------------------------------------------------------------------------------------------------------------
void CSRSWBIPQueue::commitRead(UInt32 size)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update stuffs
	mInternals->mReadPtr += size;
}

//----------------------------------------------------------------------------------------------------------------------
CSRSWBIPQueue::WriteBufferInfo CSRSWBIPQueue::requestWrite(UInt32 maxSize) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Capture info locally
	UInt8*	readPtr = mInternals->mReadPtr;

	// Check situation
	if (mInternals->mWritePtr >= readPtr) {
		// Reset watermark
		mInternals->mWriteWatermarkPtr = mInternals->mBuffer + mInternals->mSize;

		// Can write up to end of buffer, or set watermark and write to beginning of buffer
		if ((UInt32) (mInternals->mWriteWatermarkPtr - mInternals->mWritePtr) >= maxSize)
			// Can write more from current position
			return WriteBufferInfo(mInternals->mWritePtr, maxSize);
		else if ((UInt32) (readPtr - mInternals->mBuffer) >= maxSize) {
			// Must start at beginning of the buffer now
			mInternals->mWriteWatermarkPtr = mInternals->mWritePtr;
			mInternals->mWritePtr = mInternals->mBuffer;

			return WriteBufferInfo(mInternals->mWritePtr, maxSize);
		} else
			// Not enough space
			return WriteBufferInfo();
	} else {
		// Write pointer is before read pointer
		if ((UInt32) (readPtr - mInternals->mWritePtr) >= maxSize)
			// Can write more from current position
			return WriteBufferInfo(mInternals->mWritePtr, maxSize);
		else
			// Not enough space
			return WriteBufferInfo();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CSRSWBIPQueue::commitWrite(UInt32 size)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update stuffs
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
CSRSWBIPSegmentedQueue::ReadBufferInfo CSRSWBIPSegmentedQueue::requestRead() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Capture info locally
	UInt8*	writePtr = mInternals->mWritePtr;
	UInt8*	writeWatermarkPtr = mInternals->mWriteWatermarkPtr;

	// Check if time to go back to the beginning
	if (mInternals->mReadPtr == writeWatermarkPtr)
		// Reached the end, wrap around
		mInternals->mReadPtr = mInternals->mBuffer;

	// Check stuffs
	if (writePtr > mInternals->mReadPtr)
		// Can read to write pointer
		return ReadBufferInfo(mInternals->mReadPtr, mInternals->mSegmentSize,
				(UInt32) (writePtr - mInternals->mReadPtr));
	else if (writePtr < mInternals->mReadPtr)
		// Can read to write watermark pointer
		return ReadBufferInfo(mInternals->mReadPtr, mInternals->mSegmentSize,
				(UInt32) (writeWatermarkPtr - mInternals->mReadPtr));
	else
		// Queue is empty
		return ReadBufferInfo();
}

//----------------------------------------------------------------------------------------------------------------------
void CSRSWBIPSegmentedQueue::commitRead(UInt32 size)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update stuffs
	mInternals->mReadPtr += size;
}

//----------------------------------------------------------------------------------------------------------------------
CSRSWBIPSegmentedQueue::WriteBufferInfo CSRSWBIPSegmentedQueue::requestWrite(UInt32 maxSize) const
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
			return WriteBufferInfo(mInternals->mWritePtr, mInternals->mSegmentSize, maxSize);
		else if ((UInt32) (readPtr - mInternals->mBuffer) >= maxSize) {
			// Must start at beginning of the buffer now
			mInternals->mWriteWatermarkPtr = mInternals->mWritePtr;
			mInternals->mWritePtr = mInternals->mBuffer;

			return WriteBufferInfo(mInternals->mWritePtr, mInternals->mSegmentSize, maxSize);
		} else
			// Not enough space
			return WriteBufferInfo();
	} else {
		// Write pointer is before read pointer
		if ((UInt32) (readPtr - mInternals->mWritePtr) >= maxSize)
			// Can write more from current position
			return WriteBufferInfo(mInternals->mWritePtr, mInternals->mSegmentSize, maxSize);
		else
			// Not enough space
			return WriteBufferInfo();
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

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSRSWMessageQueue

// MARK: Properties

OSType	CSRSWMessageQueue::ProcMessage::mType = MAKE_OSTYPE('P', 'r', 'o', 'c');

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CSRSWMessageQueue::CSRSWMessageQueue(UInt32 size) : CSRSWBIPQueue(size)
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
CSRSWMessageQueue::CSRSWMessageQueue(const CSRSWMessageQueue& other) : CSRSWBIPQueue(other)
//----------------------------------------------------------------------------------------------------------------------
{
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CSRSWMessageQueue::handleAll()
//----------------------------------------------------------------------------------------------------------------------
{
	// Process
	CSRSWBIPQueue::ReadBufferInfo	readBufferInfo = requestRead();
	while (readBufferInfo.hasBuffer()) {
		// Process message
		Message&	message = *((Message*) readBufferInfo.mBuffer);
		handle(message);

		// Processed
		commitRead(message.mSize);

		// Get next
		readBufferInfo = requestRead();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CSRSWMessageQueue::submit(const Message& message)
//----------------------------------------------------------------------------------------------------------------------
{
	// Query situation
	CSRSWBIPQueue::WriteBufferInfo	writeBufferInfo = requestWrite(message.mSize);

	// Validate
	AssertFailIf(writeBufferInfo.mSize < message.mSize);

	// Copy and commit
	::memcpy(writeBufferInfo.mBuffer, &message, message.mSize);
	commitWrite(message.mSize);
}

// MARK: Subclass methods

//----------------------------------------------------------------------------------------------------------------------
void CSRSWMessageQueue::handle(const Message& message)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check type
	if (message.mType == ProcMessage::mType)
		// Perform
		((ProcMessage&) message).perform();
}

#include "CArray.h"

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSRSWMessageQueuesInternals

class CSRSWMessageQueuesInternals {
	public:
		CSRSWMessageQueuesInternals() {}

		TNArray<CSRSWMessageQueue>	mMessageQueues;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSRSWMessageQueues

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CSRSWMessageQueues::CSRSWMessageQueues()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CSRSWMessageQueuesInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CSRSWMessageQueues::~CSRSWMessageQueues()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CSRSWMessageQueues::add(CSRSWMessageQueue& messageQueue)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mMessageQueues.add(messageQueue);
}

//----------------------------------------------------------------------------------------------------------------------
void CSRSWMessageQueues::remove(CSRSWMessageQueue& messageQueue)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mMessageQueues.remove(messageQueue);
}

//----------------------------------------------------------------------------------------------------------------------
void CSRSWMessageQueues::handleAll()
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all
	for (TIteratorD<CSRSWMessageQueue> iterator = mInternals->mMessageQueues.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Handle all
		CSRSWMessageQueue&	messageQueue = iterator.getValue();
		messageQueue.handleAll();
	}
}
