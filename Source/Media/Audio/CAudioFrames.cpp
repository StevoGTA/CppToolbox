//----------------------------------------------------------------------------------------------------------------------
//	CAudioFrames.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioFrames.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioFramesInternals

class CAudioFramesInternals {
	public:
		CAudioFramesInternals(UInt32 bufferCount, UInt32 bytesPerBuffer, UInt32 availableFrameCount,
				UInt32 bytesPerFrame) :
			mBufferCount(bufferCount), mBytesPerBuffer(bytesPerBuffer), mAvailableFrameCount(availableFrameCount),
					mCurrentFrameCount(0), mBytesPerFrame(bytesPerFrame)
			{}

		UInt32	mBufferCount;
		UInt32	mBytesPerBuffer;
		UInt32	mAvailableFrameCount;
		UInt32	mCurrentFrameCount;
		UInt32	mBytesPerFrame;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioFrames

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioFrames::CAudioFrames(void* buffers, UInt32 bufferCount, UInt32 bufferTotalFrameCount,
		UInt32 bufferAvailableFrameCount, UInt32 bytesPerFrame) :
		CData(buffers, bufferCount * bufferTotalFrameCount * bytesPerFrame, false)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals =
			new CAudioFramesInternals(bufferCount, bufferTotalFrameCount * bytesPerFrame, bufferAvailableFrameCount,
					bytesPerFrame);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioFrames::CAudioFrames(UInt32 bufferCount, UInt32 bytesPerFrame, UInt32 frameCountPerBuffer) :
		CData((CData::Size) (bufferCount * frameCountPerBuffer * bytesPerFrame))
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals =
			new CAudioFramesInternals(bufferCount, frameCountPerBuffer * bytesPerFrame, frameCountPerBuffer,
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
TNumericArray<const void*> CAudioFrames::getBuffersAsRead() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNumericArray<const void*>	bufferPtrs;
	for (UInt32 i = 0; i < mInternals->mBufferCount; i++)
		// Update
		bufferPtrs += (void*) ((UInt8*) getBytePtr() + mInternals->mBytesPerBuffer * i);

	return bufferPtrs;
}

//----------------------------------------------------------------------------------------------------------------------
TNumericArray<void*> CAudioFrames::getBuffersAsWrite()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNumericArray<void*>	bufferPtrs;
	for (UInt32 i = 0; i < mInternals->mBufferCount; i++)
		// Update
		bufferPtrs += (void*) ((UInt8*) getMutableBytePtr() + mInternals->mBytesPerBuffer * i);

	return bufferPtrs;
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
	AssertFailIf(audioBufferList.mNumberBuffers != mInternals->mBufferCount);

	// Setup
	const	UInt8*	buffer = (const UInt8*) getBytePtr();

	// Update AudioBufferList
	for (UInt32 i = 0; i < mInternals->mBufferCount; i++) {
		// Update this buffer
		audioBufferList.mBuffers[i].mData = (void*) (buffer + mInternals->mBytesPerBuffer * i);
		audioBufferList.mBuffers[i].mDataByteSize = mInternals->mCurrentFrameCount * mInternals->mBytesPerFrame;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioFrames::getAsWrite(AudioBufferList& audioBufferList)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf(audioBufferList.mNumberBuffers != mInternals->mBufferCount);

	// Setup
	UInt8*	buffer = (UInt8*) getMutableBytePtr();

	// Update AudioBufferList
	for (UInt32 i = 0; i < mInternals->mBufferCount; i++) {
		// Update this buffer
		audioBufferList.mBuffers[i].mData = buffer + mInternals->mBytesPerBuffer * i;
		audioBufferList.mBuffers[i].mDataByteSize = mInternals->mAvailableFrameCount * mInternals->mBytesPerFrame;
	}
}

#endif
