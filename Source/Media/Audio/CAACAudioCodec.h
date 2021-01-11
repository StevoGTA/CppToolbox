//----------------------------------------------------------------------------------------------------------------------
//	CAACAudioCodec.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioCodecRegistry.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAACAudioCodec

class CAACAudioCodecInternals;
class CAACAudioCodec : public CDecodeOnlyAudioCodec {
	// Methods
	public:
							// Lifecycle methods
							CAACAudioCodec(OSType codecID);
							~CAACAudioCodec();

							// CAudioCodec methods - Decoding
		void				setupForDecode(const SAudioProcessingFormat& audioProcessingFormat,
									CByteParceller& byteParceller, const I<CAudioCodec::DecodeInfo>& decodeInfo);
		SAudioReadStatus	decode(const SMediaPosition& mediaPosition, CAudioData& audioData);

	// Properties
	public:
		static	OSType						mAACLCID;
		static	OSType						mAACLDID;

	private:
				CAACAudioCodecInternals*	mInternals;
};
