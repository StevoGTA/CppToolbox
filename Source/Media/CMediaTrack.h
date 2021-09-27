//----------------------------------------------------------------------------------------------------------------------
//	CMediaTrack.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "TimeAndDate.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaTrack

class CMediaTrackInternals;
class CMediaTrack {
	// Info
	public:
		struct Info {

									// Lifecycle methods
									Info(UniversalTimeInterval duration, UInt32 bitrate) :
										mDuration(duration), mBitrate(bitrate)
										{}

									// Methods
			UniversalTimeInterval	getDuration() const
										{ return mDuration; }
			UInt32					getBitrate() const
										{ return mBitrate; }

			// Properties
			private:
				UniversalTimeInterval	mDuration;
				UInt32					mBitrate;
		};

	// Methods
	public:
								// Lifecycle methods
								CMediaTrack(const Info& info);
								CMediaTrack(const CMediaTrack& other);
		virtual					~CMediaTrack();

								// Instance methods
				const	Info&	getInfo() const;

	// Properties
	private:
		CMediaTrackInternals*	mInternals;
};
