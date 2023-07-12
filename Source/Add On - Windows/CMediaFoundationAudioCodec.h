//----------------------------------------------------------------------------------------------------------------------
//	CMediaFoundationAudioCodec.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioCodec.h"
#include "CMediaPacketSource.h"

#undef Delete

#include <mfapi.h>

#define Delete(x)	{ delete x; x = nil; }

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaFoundationDecodeAudioCodec

class CMediaFoundationDecodeAudioCodecInternals;
class CMediaFoundationDecodeAudioCodec : public CDecodeAudioCodec {
	// Methods
	public:
								// Lifecycle methods
								~CMediaFoundationDecodeAudioCodec();

								// CDecodeAudioCodec methods
				OV<SError>		setup(const SAudio::ProcessingFormat& audioProcessingFormat);
				void			seek(UniversalTimeInterval timeInterval);
				OV<SError>		decodeInto(CAudioFrames& audioFrames);

	protected:
								// Lifecycle methods
								CMediaFoundationDecodeAudioCodec(OSType codecID,
										const I<CMediaPacketSource>& mediaPacketSource);

								// Subclass methods
		virtual	OR<const GUID>	getGUID(OSType codecID) const = 0;
		virtual	OV<CData>		getUserData() const
									{ return OV<CData>(); }

	// Properties
	private:
		CMediaFoundationDecodeAudioCodecInternals*	mInternals;
};
