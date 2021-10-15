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
										CVideoTrack(const Info& info, const SVideoStorageFormat& videoStorageFormat);
										CVideoTrack(UInt32 index, const Info& info,
												const SVideoStorageFormat& videoStorageFormat);
										CVideoTrack(const CVideoTrack& other);
										~CVideoTrack();

										// Instance methods
		const	SVideoStorageFormat&	getVideoStorageFormat() const;

	// Properties
	private:
		CVideoTrackInternals*	mInternals;
};
