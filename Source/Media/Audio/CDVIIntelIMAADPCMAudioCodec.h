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
		void				setupForDecode(const SAudioProcessingFormat& audioProcessingFormat,
									const I<CDataSource>& dataSource, const I<CCodec::DecodeInfo>& decodeInfo);
		SAudioReadStatus	decode(const SMediaPosition& mediaPosition, CAudioFrames& audioFrames);

	// Properties
	public:
		static	OSType									mID;

	private:
				CDVIIntelIMAADPCMAudioCodecInternals*	mInternals;
};
