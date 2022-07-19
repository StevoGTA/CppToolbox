//----------------------------------------------------------------------------------------------------------------------
//	CAudioFrames.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CArray.h"
#include "CData.h"
#include "TWrappers.h"

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

class CAudioFramesInternals;
class CAudioFrames : private CData {
	// Requirements
	public:
		struct Requirements {
					// Lifecycle methods
					Requirements(UInt32 frameCountInterval, UInt32 frameCountMinimum) :
						mFrameCountInterval(frameCountInterval), mFrameCountMinimum(frameCountMinimum)
						{}
					Requirements(const Requirements& other) :
						mFrameCountInterval(other.mFrameCountInterval), mFrameCountMinimum(other.mFrameCountMinimum)
						{}

					// Instance methods
			UInt32	getFrameCount(UInt32 minimumFrameCount)
						{ return std::max<UInt32>(mFrameCountMinimum,
								((minimumFrameCount - 1) / mFrameCountInterval + 1) * mFrameCountInterval); }

			// Properties
			UInt32	mFrameCountInterval;
			UInt32	mFrameCountMinimum;
		};

	// ReadInfo
	public:
		struct Info {
											// Lifecycle methods
											Info(UInt32 frameCount, const TNumericArray<void*>& segments) :
												mFrameCount(frameCount), mSegments(segments)
												{}
											Info(const Info& other) :
												mFrameCount(other.mFrameCount), mSegments(other.mSegments)
												{}

											// Instance methods
					UInt32					getFrameCount() const
												{ return mFrameCount; }
			const	TNumericArray<void*>&	getSegments() const
												{ return mSegments; }

			// Properties
			private:
				UInt32					mFrameCount;
				TNumericArray<void*>	mSegments;
		};

	// Methods
	public:
				// Lifecycle methods
				CAudioFrames(void* buffer, UInt32 segmentCount, UInt32 segmentByteCount, UInt32 frameCount,
						UInt32 bytesPerFramePerSegment);
				CAudioFrames(UInt32 segmentCount, UInt32 bytesPerFramePerSegment, UInt32 frameCountPerSegment);
				~CAudioFrames();

				// Instance methods
		UInt32	getAvailableFrameCount() const;
		UInt32	getCurrentFrameCount() const;

		Info	getReadInfo() const;

		Info	getWriteInfo();
		void	completeWrite(const CAudioFrames& other)
					{ completeWrite(other.getCurrentFrameCount(), other.getReadInfo().getSegments()); }
		void	completeWrite(UInt32 frameCount);
		void	completeWrite(UInt32 frameCount, const TNumericArray<void*>& sampleBufferPtrs);

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
		CAudioFramesInternals*	mInternals;
};
