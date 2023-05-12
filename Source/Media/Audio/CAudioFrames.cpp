//----------------------------------------------------------------------------------------------------------------------
//	CAudioFrames.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioFrames.h"

#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioFrames::Internals

class CAudioFrames::Internals {
	public:
		Internals(UInt32 segmentCount, UInt32 segmentByteCount, UInt32 allocatedFrameCount,
				UInt32 bytesPerFramePerSegment) :
			mSegmentCount(segmentCount), mSegmentByteCount(segmentByteCount), mAllocatedFrameCount(allocatedFrameCount),
					mCurrentFrameCount(0), mBytesPerFramePerSegment(bytesPerFramePerSegment)
			{}

		UInt32	mSegmentCount;
		UInt32	mSegmentByteCount;
		UInt32	mAllocatedFrameCount;
		UInt32	mCurrentFrameCount;
		UInt32	mBytesPerFramePerSegment;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioFrames

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioFrames::CAudioFrames(void* buffer, UInt32 segmentCount, UInt32 segmentByteCount, UInt32 frameCount,
		UInt32 bytesPerFramePerSegment) : CData(buffer, segmentCount * segmentByteCount, false)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(segmentCount, segmentByteCount, frameCount, bytesPerFramePerSegment);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioFrames::CAudioFrames(UInt32 segmentCount, UInt32 bytesPerFramePerSegment, UInt32 frameCountPerSegment) :
		CData((CData::ByteCount) (segmentCount * frameCountPerSegment * bytesPerFramePerSegment))
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals =
			new Internals(segmentCount, frameCountPerSegment * bytesPerFramePerSegment, frameCountPerSegment,
					bytesPerFramePerSegment);
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
	// Setup
	TNumberArray<void*>	segments;
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
	TNumberArray<void*>	segments;
	for (UInt32 i = 0; i < mInternals->mSegmentCount; i++)
		// Update
		segments +=
				(void*) ((UInt8*) getMutableBytePtr() +
						mInternals->mSegmentByteCount * i +
						mInternals->mCurrentFrameCount * mInternals->mBytesPerFramePerSegment);

	return Info(mInternals->mAllocatedFrameCount - mInternals->mCurrentFrameCount, segments);
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
void CAudioFrames::completeWrite(UInt32 frameCount, const TNumberArray<void*>& sampleBufferPtrs)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check
	AssertFailIf((mInternals->mCurrentFrameCount + frameCount) > mInternals->mAllocatedFrameCount);

	// Setup
	CArray::ItemCount	sampleBuffersCount = sampleBufferPtrs.getCount();

	// Check what to do
	if ((mInternals->mSegmentCount == 1) && (sampleBuffersCount == 1))
		// Interleaved and Interleaved coming in
		::memcpy((UInt8*) getMutableBytePtr() + mInternals->mCurrentFrameCount * mInternals->mBytesPerFramePerSegment,
				sampleBufferPtrs[0], frameCount * mInternals->mBytesPerFramePerSegment);
	else if (mInternals->mSegmentCount == 1) {
		// Interleaved and Non-interleaved coming in
		UInt8*	destinationStartPtr =
						(UInt8*) getMutableBytePtr() +
								mInternals->mCurrentFrameCount * mInternals->mBytesPerFramePerSegment;
		for (UInt32 sampleBufferIndex = 0; sampleBufferIndex < sampleBuffersCount; sampleBufferIndex++) {
			// Check bytes per sample
			switch (mInternals->mBytesPerFramePerSegment / sampleBuffersCount) {
				case 8: {
					// 8 bytes per sample
					const	UInt64*	sourcePtr = (UInt64*) sampleBufferPtrs[sampleBufferIndex];
							UInt64*	destinationPtr = (UInt64*) destinationStartPtr + sampleBufferIndex;
					for (UInt32 frameIndex = 0; frameIndex < frameCount; frameIndex++,
							destinationPtr += sampleBuffersCount)
						// Copy sample
						*destinationPtr = (*sourcePtr++);
					} break;

				case 4: {
					// 4 bytes per sample
					const	UInt32*	sourcePtr = (UInt32*) sampleBufferPtrs[sampleBufferIndex];
							UInt32*	destinationPtr = (UInt32*) destinationStartPtr + sampleBufferIndex;
					for (UInt32 frameIndex = 0; frameIndex < frameCount; frameIndex++,
							destinationPtr += sampleBuffersCount)
						// Copy sample
						*destinationPtr = (*sourcePtr++);
					} break;

				case 3: {
					// 3 bytes per sample
					const	UInt8*	sourcePtr = (UInt8*) sampleBufferPtrs[sampleBufferIndex];
							UInt8*	destinationPtr = destinationStartPtr + sampleBufferIndex;
					for (UInt32 frameIndex = 0; frameIndex < frameCount; frameIndex++,
							destinationPtr += sampleBuffersCount * 3) {
						// Copy sample
						*destinationPtr = (*sourcePtr++);
						*(destinationPtr + 1) = (*sourcePtr++);
						*(destinationPtr + 2) = (*sourcePtr++);
					} } break;

				case 2: {
					// 2 bytes per sample
					const	UInt16*	sourcePtr = (UInt16*) sampleBufferPtrs[sampleBufferIndex];
							UInt16*	destinationPtr = (UInt16*) destinationStartPtr + sampleBufferIndex;
					for (UInt32 frameIndex = 0; frameIndex < frameCount; frameIndex++,
							destinationPtr += sampleBuffersCount)
						// Copy sample
						*destinationPtr = (*sourcePtr++);
					} break;

				case 1: {
					// 1 byte per sample
					const	UInt8*	sourcePtr = (UInt8*) sampleBufferPtrs[sampleBufferIndex];
							UInt8*	destinationPtr = destinationStartPtr + sampleBufferIndex;
					for (UInt32 frameIndex = 0; frameIndex < frameCount; frameIndex++,
							destinationPtr += sampleBuffersCount)
						// Copy sample
						*destinationPtr = (*sourcePtr++);
					} break;
			}
		}
	} else if (sampleBuffersCount == 1) {
		// Non-interleaved and Interleaved coming in
		AssertFailUnimplemented();
	} else {
		// Non-interleaved and Non-interleaved coming in
		AssertFailIf(mInternals->mSegmentCount != sampleBuffersCount);

		for (UInt32 sampleBufferIndex = 0; sampleBufferIndex < sampleBuffersCount; sampleBufferIndex++)
			// Copy samples
			::memcpy(
					(UInt8*) getMutableBytePtr() + mInternals->mSegmentByteCount * sampleBufferIndex +
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
	const	UInt8*	buffer = (const UInt8*) getBytePtr();

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
	UInt8*	buffer = (UInt8*) getMutableBytePtr();
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
	CData::ByteCount	byteCount = getByteCount();

	// Check bits
	switch (bits) {
		case 16: {
			// 16 bits
			UInt16*	buffer = (UInt16*) getMutableBytePtr();
			for (UInt32 i = 0; i < byteCount / sizeof(UInt16); i++, buffer++)
				// Swap
				*buffer = Endian16_Swap(*buffer);
			} break;

		case 24: {
			// 24 bits
			UInt8*	buffer = (UInt8*) getMutableBytePtr();
			for (UInt32 i = 0; i < byteCount / 3; i++, buffer += 3) {
				// Swap 24 bits
				UInt8	temp = *buffer;
				*buffer = *(buffer + 2);
				*(buffer + 2) = temp;
			}
			} break;

		case 32: {
			// 32 bits
			UInt32*	buffer = (UInt32*) getMutableBytePtr();
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
	UInt8*				buffer = (UInt8*) getMutableBytePtr();
	CData::ByteCount	byteCount = getByteCount();

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
