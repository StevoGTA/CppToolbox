//----------------------------------------------------------------------------------------------------------------------
//	SMediaPosition.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "TimeAndDate.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK:  SMediaPosition

struct SMediaPosition {
	// Types
	enum Mode {
		kFromStart,		// Uses mTimeInterval
		kFromCurrent,	// Ignores mTimeInterval
	};

									// Lifecycle methods
									SMediaPosition(const SMediaPosition& other) :
											mMode(other.mMode), mTimeInterval(other.mTimeInterval)
											{}

									// Instance methods
			Mode					getMode() const
										{ return mMode; }
			UniversalTimeInterval	getTimeInterval() const
										{ return mTimeInterval; }
			UInt64					getFrameIndex(Float32 sampleRate) const
										{ return (UInt64) (mTimeInterval * sampleRate); }

									// Class methods
	static	SMediaPosition			fromStart(UniversalTimeInterval timeInterval)
										{ return SMediaPosition(kFromStart, timeInterval); }
	static	SMediaPosition			fromCurrent()
										{ return SMediaPosition(kFromCurrent, 0.0); }

	private:
									// Lifecycle methods
									SMediaPosition(Mode mode, UniversalTimeInterval timeInterval) :
										mMode(mode), mTimeInterval(timeInterval)
										{}

	// Properties
	private:
		Mode					mMode;
		UniversalTimeInterval	mTimeInterval;
};
