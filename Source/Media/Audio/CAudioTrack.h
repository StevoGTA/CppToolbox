//----------------------------------------------------------------------------------------------------------------------
//	CAudioTrack.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioCodec.h"
#include "CMediaTrack.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioTrack

class CAudioTrackInternals;
class CAudioTrack : public CMediaTrack {
	// Methods
	public:
												// Lifecycle methods
												CAudioTrack(const SAudioStorageFormat& audioStorageFormat,
														const I<CCodec::DecodeInfo>& decodeInfo);
												CAudioTrack(UInt32 index, const SAudioStorageFormat& audioStorageFormat,
														const I<CCodec::DecodeInfo>& decodeInfo);
												CAudioTrack(const CAudioTrack& other);
												~CAudioTrack();

												// Instance methods
				const	SAudioStorageFormat&	getAudioStorageFormat() const;
				const	I<CCodec::DecodeInfo>&	getDecodeInfo() const;

		static			Float32					getDBFromValue(Float32 value)
													{ return 20.0 * log10(value); }
		static			Float32					getValueFromDB(Float32 db)
													{ return powf(10.0, db / 20.0); }
//		static			Float32					getPercentFromValue(Float32 value)
//													{ return value / 100.0; }

		static			CString					getStringFromDB(Float32 db, Float32 muteDB);

	// Properties
	private:
		CAudioTrackInternals*	mInternals;
};
