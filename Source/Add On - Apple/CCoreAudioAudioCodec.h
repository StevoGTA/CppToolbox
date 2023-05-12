//----------------------------------------------------------------------------------------------------------------------
//	CCoreAudioAudioCodec.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioCodec.h"

#include <AudioToolbox/AudioToolbox.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreAudioDecodeAudioCodec

class CCoreAudioDecodeAudioCodec : public CDecodeAudioCodec {
	// Classes
	private:
		class Internals;

	// Methods
	public:
											// Lifecycle methods
											~CCoreAudioDecodeAudioCodec();

											// CDecodeAudioCodec methods
				OV<SError>					setup(const SAudioProcessingFormat& audioProcessingFormat);
				CAudioFrames::Requirements	getRequirements() const
												{ return CAudioFrames::Requirements(1024, 1024 * 2); }
				void						seek(UniversalTimeInterval timeInterval);
				OV<SError>					decodeInto(CAudioFrames& audioFrames);

	protected:
											// Lifecycle methods
											CCoreAudioDecodeAudioCodec(OSType codecID,
													const I<CMediaPacketSource>& mediaPacketSource);

											// Subclass methods
		virtual	AudioStreamBasicDescription	getSourceASBD(OSType codecID,
													const SAudioProcessingFormat& audioProcessingFormat) = 0;
		virtual	OV<SError>					setMagicCookie(AudioConverterRef audioConverterRef)
												{ return OV<SError>(); }

	// Properties
	private:
		Internals*	mInternals;
};
