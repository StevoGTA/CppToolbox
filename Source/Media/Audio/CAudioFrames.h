//----------------------------------------------------------------------------------------------------------------------
//	CAudioFrames.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CData.h"
#include "TBuffer.h"
#include "TWrappers.h"

#if TARGET_OS_IOS || TARGET_OS_MACOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
	#include <CoreAudioTypes/CoreAudioTypes.h>
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioFrames

/*
	Terms:
		"sample" is an individual LPCM sample of typically between 8 and 64 bits.
		"frame" is the collection of samples for all channels that are active at the same time slice.
		"bytesPerFrame" is how many bytes are required to store a single frame of audio data.

		"packet" is opaque data comprising a certain number of samples (or frames) compressed into a certain number of
				bytes.

		Frames can be together in a single buffer (interleaved) or in separate buffers (non-interleaved).  For
			simplicity and performance, CAudioFrames always uses a single data buffer in all cases.  For interleaved,
			the buffer contains all the frames together.  For non-interleaved, the buffer is subdivided into separate
			regions for each collection of samples.
*/

class CAudioFramesInternals;
class CAudioFrames : public CData {
	// Methods
	public:
							// Lifecycle methods
							CAudioFrames(void* buffers, UInt32 bufferCount, UInt32 bufferTotalFrameCount,
									UInt32 bufferAvailableFrameCount, UInt32 bytesPerFrame);
							CAudioFrames(UInt32 bufferCount, UInt32 bytesPerFrame, UInt32 frameCountPerBuffer = 4096);
							~CAudioFrames();

							// Instance methods
		UInt32				getAvailableFrameCount() const;
		UInt32				getCurrentFrameCount() const;

		I<TBuffer<void*> >	getBuffers() const;
		void				completeWrite(UInt32 frameCount);

		void				reset();

#if TARGET_OS_IOS || TARGET_OS_MACOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
							// Apple methods
		void				getAsRead(AudioBufferList& audioBufferList) const;
		void				getAsWrite(AudioBufferList& audioBufferList);
#endif

	// Properties
	private:
		CAudioFramesInternals*	mInternals;
};
