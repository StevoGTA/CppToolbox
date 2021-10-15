//----------------------------------------------------------------------------------------------------------------------
//	SAudioSourceStatus.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "TimeAndDate.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SAudioSourceStatus

struct SAudioSourceStatus {
										// Lifecycle methods
										SAudioSourceStatus(UniversalTimeInterval timeInterval) :
												mTimeInterval(timeInterval)
												{}
										SAudioSourceStatus(const SError& error) : mError(OI<SError>(error)) {}
										SAudioSourceStatus(const SAudioSourceStatus& other) :
											mTimeInterval(other.mTimeInterval), mError(other.mError)
											{}

										// Instance methods
				bool					isSuccess() const
											{ return !mError.hasInstance(); }
				UniversalTimeInterval	getTimeInterval() const
											{ return *mTimeInterval; }
		const	SError&					getError() const
											{ return *mError; }

	// Properties
	private:
		OV<UniversalTimeInterval>	mTimeInterval;
		OI<SError>					mError;
};
