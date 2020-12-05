//----------------------------------------------------------------------------------------------------------------------
//	CAudioData.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CData.h"
#include "TBuffer.h"
#include "TInstance.h"

#if TARGET_OS_IOS || TARGET_OS_MACOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
	#include <CoreAudioTypes/CoreAudioTypes.h>
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioData

class CAudioDataInternals;
class CAudioData : public CData {
	// Methods
	public:
							// Lifecycle methods
							CAudioData(void* buffer, UInt32 bufferCount, UInt32 bufferTotalFrameCount,
									UInt32 bufferAvailableFrameCount, UInt32 bytesPerFrame);
							CAudioData(UInt32 bufferCount, UInt32 bytesPerFrame, UInt32 frameCountPerBuffer = 4096);
							~CAudioData();

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
		CAudioDataInternals*	mInternals;
};
