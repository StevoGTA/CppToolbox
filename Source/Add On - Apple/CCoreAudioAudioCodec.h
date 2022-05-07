//----------------------------------------------------------------------------------------------------------------------
//	CCoreAudioAudioCodec.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioCodec.h"

#include <AudioToolbox/AudioToolbox.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreAudioDecodeAudioCodec

class CCoreAudioDecodeAudioCodecInternals;
class CCoreAudioDecodeAudioCodec : public CDecodeAudioCodec {
	// Methods
	public:
											// Lifecycle methods
											~CCoreAudioDecodeAudioCodec();

											// CDecodeAudioCodec methods
				OI<SError>					setup(const SAudioProcessingFormat& audioProcessingFormat);
				void						seek(UniversalTimeInterval timeInterval);
				OI<SError>					decodeInto(CAudioFrames& audioFrames);

	protected:
											// Lifecycle methods
											CCoreAudioDecodeAudioCodec(OSType codecID,
													const I<CMediaPacketSource>& mediaPacketSource);

											// Subclass methods
		virtual	AudioStreamBasicDescription	getSourceASBD(OSType codecID,
													const SAudioProcessingFormat& audioProcessingFormat) = 0;
		virtual	OI<SError>					setMagicCookie(AudioConverterRef audioConverterRef)
												{ return OI<SError>(); }

	// Properties
	private:
		CCoreAudioDecodeAudioCodecInternals*	mInternals;
};
