//----------------------------------------------------------------------------------------------------------------------
//	CDVIIntelIMAADPCMAudioCodec.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioCodecRegistry.h"

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
		TArray<SAudioProcessingSetup>	getDecodeAudioProcessingSetups(
												const SAudioStorageFormat& storedAudioSampleFormat) const;
		void							setupForDecode(const SAudioProcessingFormat& audioProcessingFormat,
												CByteParceller& byteParceller,
												const I<CAudioCodec::DecodeInfo>& decodeInfo);
		SAudioReadStatus				decode(const SMediaPosition& mediaPosition, CAudioData& audioData);

	// Properties
	public:
		static	OSType									mID;

	private:
				CDVIIntelIMAADPCMAudioCodecInternals*	mInternals;
};
