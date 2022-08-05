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
												CAudioTrack(const Info& info,
														const SAudioStorageFormat& audioStorageFormat);
												CAudioTrack(UInt32 index, const Info& info,
														const SAudioStorageFormat& audioStorageFormat);
												CAudioTrack(const CAudioTrack& other);
												~CAudioTrack();

												// Instance methods
				const	SAudioStorageFormat&	getAudioStorageFormat() const;

												// Class methods
		static			Info					composeInfo(UniversalTimeInterval duration,
														const SAudioStorageFormat& audioStorageFormat,
														UInt32 bytesPerFrame);
		static			Info					composeInfo(UniversalTimeInterval duration,
														const SAudioStorageFormat& audioStorageFormat,
														UInt32 framesPerPacket, UInt32 bytesPerPacket);
		static			Info					composeInfo(const SAudioStorageFormat& audioStorageFormat,
														UInt64 frameCount, UInt64 byteCount);
		static			Float32					getDBFromValue(Float32 value)
													{ return (Float32) (20.0 * log10(value)); }
		static			Float32					getValueFromDB(Float32 db)
													{ return powf(10.0, (Float32) (db / 20.0)); }
//		static			Float32					getPercentFromValue(Float32 value)
//													{ return value / 100.0; }

		static			CString					getStringFromDB(Float32 db, Float32 muteDB);

	// Properties
	private:
		CAudioTrackInternals*	mInternals;
};
