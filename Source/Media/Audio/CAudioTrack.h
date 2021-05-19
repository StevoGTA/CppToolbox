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

	// Properties
	private:
		CAudioTrackInternals*	mInternals;
};
