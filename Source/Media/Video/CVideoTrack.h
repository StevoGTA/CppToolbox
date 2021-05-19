//----------------------------------------------------------------------------------------------------------------------
//	CVideoTrack.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CVideoCodec.h"
#include "SVideoFormats.h"
#include "CMediaTrack.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CVideoTrack

class CVideoTrackInternals;
class CVideoTrack : public CMediaTrack {
	// Methods
	public:
										// Lifecycle methods
										CVideoTrack(const SVideoStorageFormat& videoStorageFormat,
												const I<CCodec::DecodeInfo>& decodeInfo);
										CVideoTrack(UInt32 index, const SVideoStorageFormat& videoStorageFormat,
												const I<CCodec::DecodeInfo>& decodeInfo);
										CVideoTrack(const CVideoTrack& other);
										~CVideoTrack();

										// Instance methods
		const	SVideoStorageFormat&	getVideoStorageFormat() const;
		const	I<CCodec::DecodeInfo>&	getDecodeInfo() const;

	// Properties
	private:
		CVideoTrackInternals*	mInternals;
};
