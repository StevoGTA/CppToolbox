//----------------------------------------------------------------------------------------------------------------------
//	CMediaFoundationAudioCodec.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioCodec.h"

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
				OI<SError>		setup(const SAudioProcessingFormat& audioProcessingFormat);
				void			seek(UniversalTimeInterval timeInterval);
				OI<SError>		decodeInto(CAudioFrames& audioFrames);

	protected:
								// Lifecycle methods
								CMediaFoundationDecodeAudioCodec(OSType codecID,
										const I<CMediaPacketSource>& mediaPacketSource);

								// Subclass methods
		virtual	OR<const GUID>	getGUID(OSType codecID) const = 0;
		virtual	OI<CData>		getUserData() const
									{ return OI<CData>(); }

	// Properties
	private:
		CMediaFoundationDecodeAudioCodecInternals*	mInternals;
};
