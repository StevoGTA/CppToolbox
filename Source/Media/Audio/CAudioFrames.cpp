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
		CData((CData::Size) (segmentCount * frameCountPerSegment * bytesPerFrame))
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
	return mInternals->mAvailableFrameCount;
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CAudioFrames::getCurrentFrameCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mCurrentFrameCount;
}

//----------------------------------------------------------------------------------------------------------------------
TNumericArray<const void*> CAudioFrames::getSegmentsAsRead() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNumericArray<const void*>	segmentPtrs;
	for (UInt32 i = 0; i < mInternals->mSegmentCount; i++)
		// Update
		segmentPtrs += (void*) ((UInt8*) getBytePtr() + mInternals->mSegmentByteCount * i);

	return segmentPtrs;
}

//----------------------------------------------------------------------------------------------------------------------
TNumericArray<void*> CAudioFrames::getSegmentsAsWrite()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNumericArray<void*>	segmentPtrs;
	for (UInt32 i = 0; i < mInternals->mSegmentCount; i++)
		// Update
		segmentPtrs += (void*) ((UInt8*) getMutableBytePtr() + mInternals->mSegmentByteCount * i);

	return segmentPtrs;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioFrames::completeWrite(UInt32 frameCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check
	AssertFailIf(frameCount > mInternals->mAvailableFrameCount);

	// Store
	mInternals->mCurrentFrameCount = frameCount;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioFrames::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mCurrentFrameCount = 0;
}

#if TARGET_OS_IOS || TARGET_OS_MACOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS

//----------------------------------------------------------------------------------------------------------------------
void CAudioFrames::getAsRead(AudioBufferList& audioBufferList) const
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
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioFrames::getAsWrite(AudioBufferList& audioBufferList)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf(audioBufferList.mNumberBuffers != mInternals->mSegmentCount);

	// Setup
	UInt8*	buffer = (UInt8*) getMutableBytePtr();

	// Update AudioBufferList
	for (UInt32 i = 0; i < mInternals->mSegmentCount; i++) {
		// Update this buffer
		audioBufferList.mBuffers[i].mData = buffer + mInternals->mSegmentByteCount * i;
		audioBufferList.mBuffers[i].mDataByteSize = mInternals->mAvailableFrameCount * mInternals->mBytesPerFrame;
	}
}

#endif
