//----------------------------------------------------------------------------------------------------------------------
//	CAudioFrames.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioFrames.h"

#include "CReferenceCountable.h"
#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioFrames::SourceQueue::Internals

class CAudioFrames::SourceQueue::Internals : public TReferenceCountableAutoDelete<Internals> {
	public:
		Internals(UInt32 channelCount) :
			TReferenceCountableAutoDelete(),
					mBuffer(0), mChannelCount(channelCount), mFirstBufferedFrameIndex(0), mBufferedFrameCount(0),
					mConsumedFrameCount(0)
			{}

		TBuffer<Float32>	mBuffer;
		UInt32				mChannelCount;
		UInt64				mFirstBufferedFrameIndex;
		UInt32				mBufferedFrameCount;
		UInt32				mConsumedFrameCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioFrames::SourceQueue

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioFrames::SourceQueue::SourceQueue(UInt32 channelCount)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(channelCount);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioFrames::SourceQueue::SourceQueue(const SourceQueue& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CAudioFrames::SourceQueue::~SourceQueue()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UInt64 CAudioFrames::SourceQueue::getStartFrameIndex() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFirstBufferedFrameIndex + mInternals->mConsumedFrameCount;
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CAudioFrames::SourceQueue::getEndFrameIndex() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFirstBufferedFrameIndex + mInternals->mBufferedFrameCount;
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CAudioFrames::SourceQueue::getFrameCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mBufferedFrameCount - mInternals->mConsumedFrameCount;
}

//----------------------------------------------------------------------------------------------------------------------
TBuffer<Float32> CAudioFrames::SourceQueue::getFrames() const
//----------------------------------------------------------------------------------------------------------------------
{
	return TBuffer<Float32>(*mInternals->mBuffer + (UInt64) mInternals->mConsumedFrameCount * mInternals->mChannelCount,
			(UInt64) getFrameCount() * mInternals->mChannelCount);
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioFrames::SourceQueue::add(const CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	Info	readInfo = audioFrames.getReadInfo();
	UInt32	frameCount = readInfo.getFrameCount();
	AssertFailIf(readInfo.getSegmentCount() != 1);

	// Calculate needed capacity
	UInt32	capacityFrameCount = (UInt32) (mInternals->mBuffer.getCount() / mInternals->mChannelCount);

	// Check if have capacity
	if ((capacityFrameCount - mInternals->mBufferedFrameCount) < frameCount) {
		// Compact if possible and grow if necessary
		Float32*	availableSamplePtr =
							*mInternals->mBuffer +
									(UInt64) mInternals->mConsumedFrameCount * mInternals->mChannelCount;
		UInt32		availableFrameCount = mInternals->mBufferedFrameCount - mInternals->mConsumedFrameCount;
		size_t		availableByteCount = (size_t) availableFrameCount * mInternals->mChannelCount * sizeof(Float32);
		UInt32		frameCountNeeded = availableFrameCount + frameCount;
		if (frameCountNeeded > capacityFrameCount) {
			// Need to grow
			capacityFrameCount = std::max<UInt32>(capacityFrameCount * 2, frameCountNeeded);

			TBuffer<Float32>	buffer((UInt64) capacityFrameCount * mInternals->mChannelCount);
			::memcpy(*buffer, availableSamplePtr, availableByteCount);
			mInternals->mBuffer = buffer;
		} else
			// Compact only
			::memmove(*mInternals->mBuffer, availableSamplePtr, availableByteCount);
		mInternals->mFirstBufferedFrameIndex += mInternals->mConsumedFrameCount;
		mInternals->mBufferedFrameCount = availableFrameCount;
		mInternals->mConsumedFrameCount = 0;
	}

	// Append
	::memcpy(*mInternals->mBuffer + (UInt64) mInternals->mBufferedFrameCount * mInternals->mChannelCount,
			readInfo.getSegment(0), (size_t) frameCount * mInternals->mChannelCount * sizeof(Float32));
	mInternals->mBufferedFrameCount += frameCount;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioFrames::SourceQueue::consumeInto(CAudioFrames& audioFrames, UInt32 frameCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check
	AssertFailIf(frameCount > getFrameCount());

	// Copy
	audioFrames.reset();
	::memcpy(audioFrames.getWriteInfo().getSegment(0),
			*mInternals->mBuffer + (UInt64) mInternals->mConsumedFrameCount * mInternals->mChannelCount,
			(size_t) frameCount * mInternals->mChannelCount * sizeof(Float32));
	audioFrames.completeWrite(frameCount);

	// Consumed
	mInternals->mConsumedFrameCount += frameCount;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioFrames::SourceQueue::noteConsumedBefore(UInt64 frameIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if any available frames are before the given frame
	if (frameIndex > getStartFrameIndex())
		// Consume them
		mInternals->mConsumedFrameCount +=
				(UInt32) std::min<UInt64>(frameIndex - getStartFrameIndex(), getFrameCount());
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioFrames::SourceQueue::noteIgnoredAfter(UInt64 frameIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if any available frames are at or after the given frame
	if (frameIndex < getEndFrameIndex())
		// Free them
		mInternals->mBufferedFrameCount -=
				(UInt32) std::min<UInt64>(getEndFrameIndex() - frameIndex, getFrameCount());
}

//----------------------------------------------------------------------------------------------------------------------
CAudioFrames::SourceQueue& CAudioFrames::SourceQueue::operator=(const SourceQueue& other)
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
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioFrames::Internals

class CAudioFrames::Internals {
	public:
		Internals(UInt32 segmentCount, UInt32 segmentByteCount, UInt32 allocatedFrameCount,
				UInt32 bytesPerFramePerSegment) :
			mSegmentCount(segmentCount), mSegmentByteCount(segmentByteCount), mAllocatedFrameCount(allocatedFrameCount),
					mCurrentFrameCount(0), mBytesPerFramePerSegment(bytesPerFramePerSegment),
					mOwnsBuffer(true), mBuffer(::calloc(1, (size_t) segmentCount * segmentByteCount)),
					mBufferByteCount(segmentCount * segmentByteCount)
			{}
		Internals(UInt32 segmentCount, UInt32 segmentByteCount, UInt32 allocatedFrameCount,
				UInt32 bytesPerFramePerSegment, void* buffer) :
			mSegmentCount(segmentCount), mSegmentByteCount(segmentByteCount), mAllocatedFrameCount(allocatedFrameCount),
					mCurrentFrameCount(0), mBytesPerFramePerSegment(bytesPerFramePerSegment),
					mOwnsBuffer(false), mBuffer(buffer), mBufferByteCount(segmentCount * segmentByteCount)
			{}

		UInt32	mSegmentCount;
		UInt32	mSegmentByteCount;
		UInt32	mAllocatedFrameCount;
		UInt32	mCurrentFrameCount;
		UInt32	mBytesPerFramePerSegment;

		bool	mOwnsBuffer;
		void*	mBuffer;
		UInt32	mBufferByteCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioFrames

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioFrames::CAudioFrames(UInt32 bytesPerFrame, UInt32 frameCount)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(1, bytesPerFrame * frameCount, frameCount, bytesPerFrame);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioFrames::CAudioFrames(UInt32 segmentCount, UInt32 bytesPerFramePerSegment, UInt32 frameCountPerSegment)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals =
			new Internals(segmentCount, bytesPerFramePerSegment * frameCountPerSegment, frameCountPerSegment,
					bytesPerFramePerSegment);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioFrames::CAudioFrames(void* buffer, UInt32 segmentCount, UInt32 segmentByteCount, UInt32 frameCount,
		UInt32 bytesPerFramePerSegment)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(segmentCount, segmentByteCount, frameCount, bytesPerFramePerSegment, buffer);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioFrames::CAudioFrames(const Info& info, UInt32 segmentIndex, bool isRead)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals =
			new Internals(1, info.getSegmentByteCount(), info.getFrameCount(),
					info.getSegmentByteCount() / info.getFrameCount(), info.getSegment(segmentIndex));

	// Check if read
	if (isRead)
		// Read
		mInternals->mCurrentFrameCount = info.getFrameCount();
}

//----------------------------------------------------------------------------------------------------------------------
CAudioFrames::~CAudioFrames()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UInt32 CAudioFrames::getAllocatedFrameCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mAllocatedFrameCount - mInternals->mCurrentFrameCount;
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CAudioFrames::getCurrentFrameCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mCurrentFrameCount;
}

//----------------------------------------------------------------------------------------------------------------------
CAudioFrames::Info CAudioFrames::getReadInfo() const
//----------------------------------------------------------------------------------------------------------------------
{
	return Info(mInternals->mBuffer, mInternals->mSegmentCount, mInternals->mSegmentByteCount,
			mInternals->mCurrentFrameCount);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioFrames::Info CAudioFrames::getWriteInfo()
//----------------------------------------------------------------------------------------------------------------------
{
	return Info((UInt8*) mInternals->mBuffer + mInternals->mCurrentFrameCount * mInternals->mBytesPerFramePerSegment,
			mInternals->mSegmentCount, mInternals->mSegmentByteCount,
			mInternals->mAllocatedFrameCount - mInternals->mCurrentFrameCount);
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioFrames::completeWrite(UInt32 frameCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check
	AssertFailIf((mInternals->mCurrentFrameCount + frameCount) > mInternals->mAllocatedFrameCount);

	// Update
	mInternals->mCurrentFrameCount += frameCount;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioFrames::completeWrite(UInt32 frameCount, const Info& info)
//----------------------------------------------------------------------------------------------------------------------
{
	// Collect segment pointers
	TBuffer<void*>	sampleBufferPtrs(info.getSegmentCount());
	for (UInt32 i = 0; i < info.getSegmentCount(); i++)
		// Store
		sampleBufferPtrs[i] = info.getSegment(i);

	completeWrite(frameCount, *sampleBufferPtrs, info.getSegmentCount());
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioFrames::completeWrite(UInt32 frameCount, void* const* sampleBufferPtrs, UInt32 sampleBufferCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check
	AssertFailIf((mInternals->mCurrentFrameCount + frameCount) > mInternals->mAllocatedFrameCount);

	// Check what to do
	if ((mInternals->mSegmentCount == 1) && (sampleBufferCount == 1))
		// Interleaved and Interleaved coming in
		::memcpy((UInt8*) mInternals->mBuffer + mInternals->mCurrentFrameCount * mInternals->mBytesPerFramePerSegment,
				sampleBufferPtrs[0], frameCount * mInternals->mBytesPerFramePerSegment);
	else if (mInternals->mSegmentCount == 1) {
		// Interleaved and Non-interleaved coming in
		UInt8*	destinationStartPtr =
						(UInt8*) mInternals->mBuffer +
								mInternals->mCurrentFrameCount * mInternals->mBytesPerFramePerSegment;
		for (UInt32 sampleBufferIndex = 0; sampleBufferIndex < sampleBufferCount; sampleBufferIndex++) {
			// Check bytes per sample
			switch (mInternals->mBytesPerFramePerSegment / sampleBufferCount) {
				case 8: {
					// 8 bytes per sample
					const	UInt64*	sourcePtr = (UInt64*) sampleBufferPtrs[sampleBufferIndex];
							UInt64*	destinationPtr = (UInt64*) destinationStartPtr + sampleBufferIndex;

					// Perform
					for (UInt32 frameIndex = 0; frameIndex < frameCount; frameIndex++,
							destinationPtr += sampleBufferCount)
						// Copy sample
						*destinationPtr = (*sourcePtr++);
					} break;

				case 4: {
					// 4 bytes per sample
					const	UInt32*	sourcePtr = (UInt32*) sampleBufferPtrs[sampleBufferIndex];
							UInt32*	destinationPtr = (UInt32*) destinationStartPtr + sampleBufferIndex;

					// Perform
					for (UInt32 frameIndex = 0; frameIndex < frameCount; frameIndex++,
							destinationPtr += sampleBufferCount)
						// Copy sample
						*destinationPtr = (*sourcePtr++);
					} break;

				case 3: {
					// 3 bytes per sample
					const	UInt8*	sourcePtr = (UInt8*) sampleBufferPtrs[sampleBufferIndex];
							UInt8*	destinationPtr = destinationStartPtr + sampleBufferIndex;

					// Perform
					for (UInt32 frameIndex = 0; frameIndex < frameCount; frameIndex++,
							destinationPtr += sampleBufferCount * 3) {
						// Copy sample
						*destinationPtr = (*sourcePtr++);
						*(destinationPtr + 1) = (*sourcePtr++);
						*(destinationPtr + 2) = (*sourcePtr++);
					} } break;

				case 2: {
					// 2 bytes per sample
					const	UInt16*	sourcePtr = (UInt16*) sampleBufferPtrs[sampleBufferIndex];
							UInt16*	destinationPtr = (UInt16*) destinationStartPtr + sampleBufferIndex;

					// Perform
					for (UInt32 frameIndex = 0; frameIndex < frameCount; frameIndex++,
							destinationPtr += sampleBufferCount)
						// Copy sample
						*destinationPtr = (*sourcePtr++);
					} break;

				case 1: {
					// 1 byte per sample
					const	UInt8*	sourcePtr = (UInt8*) sampleBufferPtrs[sampleBufferIndex];
							UInt8*	destinationPtr = destinationStartPtr + sampleBufferIndex;

					// Perform
					for (UInt32 frameIndex = 0; frameIndex < frameCount; frameIndex++,
							destinationPtr += sampleBufferCount)
						// Copy sample
						*destinationPtr = (*sourcePtr++);
					} break;
			}
		}
	} else if (sampleBufferCount == 1) {
		// Non-interleaved and Interleaved coming in
		AssertFailUnimplemented();
	} else {
		// Non-interleaved and Non-interleaved coming in
		AssertFailIf(mInternals->mSegmentCount != sampleBufferCount);

		for (UInt32 sampleBufferIndex = 0; sampleBufferIndex < sampleBufferCount; sampleBufferIndex++)
			// Copy samples
			::memcpy(
					(UInt8*) mInternals->mBuffer + mInternals->mSegmentByteCount * sampleBufferIndex +
							mInternals->mCurrentFrameCount * mInternals->mBytesPerFramePerSegment,
					sampleBufferPtrs[sampleBufferIndex],
					frameCount * mInternals->mBytesPerFramePerSegment);
	}

	// Update
	mInternals->mCurrentFrameCount += frameCount;
}

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
//----------------------------------------------------------------------------------------------------------------------
UInt32 CAudioFrames::getAsRead(AudioBufferList& audioBufferList) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf(audioBufferList.mNumberBuffers != mInternals->mSegmentCount);

	// Setup
	const	UInt8*	buffer = (const UInt8*) mInternals->mBuffer;

	// Update AudioBufferList
	for (UInt32 i = 0; i < mInternals->mSegmentCount; i++) {
		// Update this buffer
		audioBufferList.mBuffers[i].mData = (void*) (buffer + mInternals->mSegmentByteCount * i);
		audioBufferList.mBuffers[i].mDataByteSize =
				mInternals->mCurrentFrameCount * mInternals->mBytesPerFramePerSegment;
	}

	return mInternals->mCurrentFrameCount;
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CAudioFrames::getAsWrite(AudioBufferList& audioBufferList)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf(audioBufferList.mNumberBuffers != mInternals->mSegmentCount);

	// Setup
	UInt8*	buffer = (UInt8*) mInternals->mBuffer;
	UInt32	frameCount = mInternals->mAllocatedFrameCount - mInternals->mCurrentFrameCount;

	// Update AudioBufferList
	for (UInt32 i = 0; i < mInternals->mSegmentCount; i++) {
		// Setup this buffer
		audioBufferList.mBuffers[i].mData =
				buffer +
						mInternals->mSegmentByteCount * i +
						mInternals->mCurrentFrameCount * mInternals->mBytesPerFramePerSegment;
		audioBufferList.mBuffers[i].mDataByteSize = frameCount * mInternals->mBytesPerFramePerSegment;
	}

	return frameCount;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioFrames::completeWrite(AudioBufferList& audioBufferList)
//----------------------------------------------------------------------------------------------------------------------
{
	completeWrite(audioBufferList.mBuffers[0].mDataByteSize / mInternals->mBytesPerFramePerSegment);
}
#endif

//----------------------------------------------------------------------------------------------------------------------
void CAudioFrames::limit(UInt32 maxFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mCurrentFrameCount = std::min<UInt32>(mInternals->mCurrentFrameCount, maxFrames);
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioFrames::toggleEndianness(UInt8 bits)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt32	byteCount = mInternals->mBufferByteCount;

	// Check bits
	switch (bits) {
		case 16: {
			// 16 bits
			UInt16*	buffer = (UInt16*) mInternals->mBuffer;
			for (UInt32 i = 0; i < byteCount / sizeof(UInt16); i++, buffer++)
				// Swap
				*buffer = Endian16_Swap(*buffer);
			} break;

		case 24: {
			// 24 bits
			UInt8*	buffer = (UInt8*) mInternals->mBuffer;
			for (UInt32 i = 0; i < byteCount / 3; i++, buffer += 3) {
				// Swap 24 bits
				UInt8	temp = *buffer;
				*buffer = *(buffer + 2);
				*(buffer + 2) = temp;
			}
			} break;

		case 32: {
			// 32 bits
			UInt32*	buffer = (UInt32*) mInternals->mBuffer;
			for (UInt32 i = 0; i < byteCount / sizeof(UInt32); i++, buffer++)
				// Swap
				*buffer = Endian32_Swap(*buffer);
			} break;

		default:
			// Huh?
			AssertFailUnimplemented()
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioFrames::toggle8BitSignedUnsigned()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt8*	buffer = (UInt8*) mInternals->mBuffer;
	UInt32	byteCount = mInternals->mBufferByteCount;

	// Do 8 byte chunks first
	while (byteCount >= 8) {
		// Do these 8 bytes
		*((UInt64*) buffer) ^= 0x8080808080808080LL;
		buffer += 8;
		byteCount -= 8;
	}

	// Finish the last 1-7 bytes
	while (byteCount-- > 0)
		// Do this byte
		*buffer++ ^= 0x80;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioFrames::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mCurrentFrameCount = 0;
}
