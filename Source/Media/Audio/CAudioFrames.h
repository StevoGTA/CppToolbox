//----------------------------------------------------------------------------------------------------------------------
//	CAudioFrames.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "TBuffer.h"
#include "TimeAndDate.h"

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
	#include <CoreAudioTypes/CoreAudioTypes.h>
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioFrames

/*
	Terms:
		"sample" is an individual LPCM sample of typically between 8 and 64 bits.
		"frame" is the collection of samples for all channels for the same time slice.
		"bytesPerFrame" is how many bytes are required to store a single frame of audio data.

		"packet" is opaque data comprising a certain number of samples (or frames) compressed into a certain number of
				bytes.

		Frames can be together in a single buffer (interleaved) or in separate buffers (non-interleaved).  For
			simplicity and performance, CAudioFrames always uses a single data buffer in all cases.  For interleaved,
			the buffer contains all the frames together.  For non-interleaved, the buffer is subdivided into separate
			segments for each collection of samples.
*/

class CAudioFrames {
	// Requirements
	public:
	// Info
	public:
		struct Info {
			// Methods
			public:
										// Lifecycle methods
										Info(void* buffer, UInt32 segmentCount, UInt32 segmentByteCount,
												UInt32 frameCount) :
											mBuffer(buffer), mSegmentCount(segmentCount),
													mSegmentByteCount(segmentByteCount), mFrameCount(frameCount)
											{}
										Info(const Info& other) :
											mBuffer(other.mBuffer), mSegmentCount(other.mSegmentCount),
													mSegmentByteCount(other.mSegmentByteCount),
													mFrameCount(other.mFrameCount)
											{}

										// Instance methods
				UInt32					getFrameCount() const
											{ return mFrameCount; }
				UniversalTimeInterval	getDurationTimeInterval(Float32 sampleRate) const
											{ return (UniversalTimeInterval) mFrameCount /
													(UniversalTimeInterval) sampleRate; }
				UInt32					getSegmentCount() const
											{ return mSegmentCount; }
				UInt32					getSegmentByteCount() const
											{ return mSegmentByteCount; }
				void*					getSegment(UInt32 index) const
											{ return (UInt8*) mBuffer + (UInt64) mSegmentByteCount * index; }

			// Properties
			private:
				void*	mBuffer;
				UInt32	mSegmentCount;
				UInt32	mSegmentByteCount;
				UInt32	mFrameCount;
		};

		struct Requirements {
			// Methods
			public:
						// Lifecycle methods
						Requirements(UInt32 frameCountInterval, UInt32 frameCountMinimum) :
							mFrameCountInterval(frameCountInterval), mFrameCountMinimum(frameCountMinimum)
							{}
						Requirements(const Requirements& other) :
							mFrameCountInterval(other.mFrameCountInterval), mFrameCountMinimum(other.mFrameCountMinimum)
							{}

						// Instance methods
				UInt32	getFrameCountInterval() const
							{ return mFrameCountInterval; }
				UInt32	getFrameCount(UInt32 minimumFrameCount)
							{ return std::max<UInt32>(mFrameCountMinimum,
									((minimumFrameCount - 1) / mFrameCountInterval + 1) * mFrameCountInterval); }

			// Properties
			private:
				UInt32	mFrameCountInterval;
				UInt32	mFrameCountMinimum;
		};

	// SourceQueue
	public:
		class SourceQueue {
			// Classes
			private:
				class Internals;

			// Methods
			public:
									// Lifecycle methods
									SourceQueue(UInt32 channelCount);
									SourceQueue(const SourceQueue& other);
									~SourceQueue();

									// Instance methods
				UInt64				getStartFrameIndex() const;
				UInt64				getEndFrameIndex() const;
				UInt32				getFrameCount() const;
				TBuffer<Float32>	getFrames() const;

				void				add(const CAudioFrames& audioFrames);
				void				consumeInto(CAudioFrames& audioFrames, UInt32 frameCount);
				void				noteConsumedBefore(UInt64 frameIndex);
				void				noteIgnoredAfter(UInt64 frameIndex);

				SourceQueue&		operator=(const SourceQueue& other);

			// Properties
			private:
				Internals*	mInternals;
		};

	// Classes
	private:
		class Internals;

	// Methods
	public:
				// Lifecycle methods
				CAudioFrames(UInt32 bytesPerFrame, UInt32 frameCount);
				CAudioFrames(UInt32 segmentCount, UInt32 bytesPerFramePerSegment, UInt32 frameCountPerSegment);
				CAudioFrames(void* buffer, UInt32 segmentCount, UInt32 segmentByteCount, UInt32 frameCount,
						UInt32 bytesPerFramePerSegment);
				CAudioFrames(const Info& info, UInt32 segmentIndex, bool isRead = false);
				~CAudioFrames();

				// Instance methods
		UInt32	getAllocatedFrameCount() const;
		UInt32	getCurrentFrameCount() const;

		Info	getReadInfo() const;

		Info	getWriteInfo();
		void	completeWrite(const CAudioFrames& other)
					{ completeWrite(other.getCurrentFrameCount(), other.getReadInfo()); }
		void	completeWrite(UInt32 frameCount);
		void	completeWrite(UInt32 frameCount, const Info& info);
		void	completeWrite(UInt32 frameCount, void* const* sampleBufferPtrs, UInt32 sampleBufferCount);

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
				// Apple methods
		UInt32	getAsRead(AudioBufferList& audioBufferList) const;

		UInt32	getAsWrite(AudioBufferList& audioBufferList);
		void	completeWrite(AudioBufferList& audioBufferList);
#endif

		void	limit(UInt32 maxFrames);

		void	toggleEndianness(UInt8 bits);
		void	toggle8BitSignedUnsigned();

		void	reset();

	// Properties
	private:
		Internals*	mInternals;
};
