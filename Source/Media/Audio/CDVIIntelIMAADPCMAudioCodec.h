//----------------------------------------------------------------------------------------------------------------------
//	CDVIIntelIMAADPCMAudioCodec.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CCodecRegistry.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDVIIntelIMAADPCMAudioCodec

class CDVIIntelIMAADPCMAudioCodecInternals;
class CDVIIntelIMAADPCMAudioCodec : public CDecodeOnlyAudioCodec {
	// Methods
	public:
										// Lifecycle methods
										CDVIIntelIMAADPCMAudioCodec();
										~CDVIIntelIMAADPCMAudioCodec();

										// CAudioCodec methods - Decoding
				void					setupForDecode(const SAudioProcessingFormat& audioProcessingFormat,
												const I<CMediaReader>& mediaReader,
												const I<CCodec::DecodeInfo>& decodeInfo);
				OI<SError>				decode(CAudioFrames& audioFrames);

										// Class methods
		static	I<CCodec::DecodeInfo>	composeDecodeInfo(UInt64 dataStartOffset, UInt64 dataSize,
												const SAudioStorageFormat& audioStorageFormat);

	// Properties
	public:
		static	OSType									mID;

	private:
				CDVIIntelIMAADPCMAudioCodecInternals*	mInternals;
};
