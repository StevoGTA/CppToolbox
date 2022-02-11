//----------------------------------------------------------------------------------------------------------------------
//	CAudioFrames.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioFrames.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioFramesInternals

class CAudioFramesInternals {
	public:
		CAudioFramesInternals(UInt32 segmentCount, UInt32 segmentByteCount, UInt32 availableFrameCount,
				UInt32 bytesPerFrame) :
			mSegmentCount(segmentCount), mSegmentByteCount(segmentByteCount), mAvailableFrameCount(availableFrameCount),
					mCurrentFrameCount(0), mBytesPerFrame(bytesPerFrame)
			{}

		UInt32	mSegmentCount;
		UInt32	mSegmentByteCount;
		UInt32	mAvailableFrameCount;
		UInt32	mCurrentFrameCount;
		UInt32	mBytesPerFrame;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioFrames

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioFrames::CAudioFrames(void* buffer, UInt32 segmentCount, UInt32 segmentByteCount, UInt32 frameCount,
		UInt32 bytesPerFrame) : CData(buffer, segmentCount * segmentByteCount, false)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals =
			new CAudioFramesInternals(segmentCount, segmentByteCount, frameCount, bytesPerFrame);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioFrames::CAudioFrames(UInt32 segmentCount, UInt32 bytesPerFrame, UInt32 frameCountPerSegment) :
		CData((CData::ByteCount) (segmentCount * frameCountPerSegment * bytesPerFrame))
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals =
			new CAudioFramesInternals(segmentCount, frameCountPerSegment * bytesPerFrame, frameCountPerSegment,
					bytesPerFrame);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioFrames::~CAudioFrames()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UInt32 CAudioFrames::getAvailableFrameCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mAvailableFrameCount - mInternals->mCurrentFrameCount;
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
	// Setup
	TNumericArray<void*>	segments;
	for (UInt32 i = 0; i < mInternals->mSegmentCount; i++)
		// Update
		segments += (void*) ((UInt8*) getBytePtr() + mInternals->mSegmentByteCount * i);

	return Info(mInternals->mCurrentFrameCount, segments);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioFrames::Info CAudioFrames::getWriteInfo()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNumericArray<void*>	segments;
	for (UInt32 i = 0; i < mInternals->mSegmentCount; i++)
		// Update
		segments +=
				(void*) ((UInt8*) getMutableBytePtr() +
						mInternals->mSegmentByteCount * i +
						mInternals->mCurrentFrameCount * mInternals->mBytesPerFrame);

	return Info(mInternals->mAvailableFrameCount - mInternals->mCurrentFrameCount, segments);
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioFrames::completeWrite(UInt32 frameCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check
	AssertFailIf((mInternals->mCurrentFrameCount + frameCount) > mInternals->mAvailableFrameCount);

	// Store
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
	const	UInt8*	buffer = (const UInt8*) getBytePtr();

	// Update AudioBufferList
	for (UInt32 i = 0; i < mInternals->mSegmentCount; i++) {
		// Update this buffer
		audioBufferList.mBuffers[i].mData = (void*) (buffer + mInternals->mSegmentByteCount * i);
		audioBufferList.mBuffers[i].mDataByteSize = mInternals->mCurrentFrameCount * mInternals->mBytesPerFrame;
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
	UInt8*	buffer = (UInt8*) getMutableBytePtr();
	UInt32	frameCount = mInternals->mAvailableFrameCount - mInternals->mCurrentFrameCount;

	// Update AudioBufferList
	for (UInt32 i = 0; i < mInternals->mSegmentCount; i++) {
		// Setup this buffer
		audioBufferList.mBuffers[i].mData =
				buffer +
						mInternals->mSegmentByteCount * i +
						mInternals->mCurrentFrameCount * mInternals->mBytesPerFrame;
		audioBufferList.mBuffers[i].mDataByteSize = frameCount * mInternals->mBytesPerFrame;
	}

	return frameCount;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioFrames::completeWrite(AudioBufferList& audioBufferList)
//----------------------------------------------------------------------------------------------------------------------
{
	completeWrite(audioBufferList.mBuffers[0].mDataByteSize / mInternals->mBytesPerFrame);
}
#endif

//----------------------------------------------------------------------------------------------------------------------
void CAudioFrames::limit(UInt32 maxFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mCurrentFrameCount = std::min<UInt32>(mInternals->mCurrentFrameCount, maxFrames);
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioFrames::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mCurrentFrameCount = 0;
}
