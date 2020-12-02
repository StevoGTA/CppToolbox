//----------------------------------------------------------------------------------------------------------------------
//	CAudioData.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioData.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioDataInternals

class CAudioDataInternals {
	public:
		CAudioDataInternals(UInt32 bufferCount, UInt32 bytesPerBuffer, UInt32 availableFrameCount,
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
// MARK: - CAudioData

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioData::CAudioData(void* buffer, UInt32 bufferCount, UInt32 bufferTotalFrameCount, UInt32 bufferAvailableFrameCount,
		UInt32 bytesPerFrame) : CData(buffer, bufferCount * bufferTotalFrameCount * bytesPerFrame, false)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals =
			new CAudioDataInternals(bufferCount, bufferTotalFrameCount * bytesPerFrame, bufferAvailableFrameCount,
					bytesPerFrame);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioData::CAudioData(UInt32 bufferCount, UInt32 bytesPerFrame, UInt32 frameCountPerBuffer) :
		CData(bufferCount * frameCountPerBuffer * bytesPerFrame)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals =
			new CAudioDataInternals(bufferCount, frameCountPerBuffer * bytesPerFrame, frameCountPerBuffer,
					bytesPerFrame);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioData::~CAudioData()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UInt32 CAudioData::getAvailableFrameCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mAvailableFrameCount;
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CAudioData::getCurrentFrameCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mCurrentFrameCount;
}

//----------------------------------------------------------------------------------------------------------------------
I<TBuffer<void*> > CAudioData::getBuffers() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TBuffer<void*>*	buffer = new TBuffer<void*>(mInternals->mBufferCount);
	for (UInt32 i = 0; i < mInternals->mBufferCount; i++)
		// Update
		(**buffer)[i] = (void*) ((UInt8*) getBytePtr() + mInternals->mBytesPerBuffer * i);

	return I<TBuffer<void*> >(buffer);
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioData::completeWrite(UInt32 frameCount)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mCurrentFrameCount = frameCount;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioData::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mCurrentFrameCount = 0;
}

#if TARGET_OS_IOS || TARGET_OS_MACOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS

//----------------------------------------------------------------------------------------------------------------------
void CAudioData::getAsRead(AudioBufferList& audioBufferList) const
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
void CAudioData::getAsWrite(AudioBufferList& audioBufferList)
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
