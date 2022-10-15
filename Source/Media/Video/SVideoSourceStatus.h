//----------------------------------------------------------------------------------------------------------------------
//	SVideoSourceStatus.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SVideoSourceStatus

struct SVideoSourceStatus {
										// Lifecycle methods
										SVideoSourceStatus(UniversalTimeInterval timeInterval) :
												mTimeInterval(timeInterval)
												{}
										SVideoSourceStatus(const SError& error) : mError(OV<SError>(error)) {}
										SVideoSourceStatus(const SVideoSourceStatus& other) :
											mTimeInterval(other.mTimeInterval), mError(other.mError)
											{}

										// Instance methods
				bool					isSuccess() const
											{ return !mError.hasValue(); }
				UniversalTimeInterval	getTimeInterval() const
											{ return *mTimeInterval; }
		const	SError&					getError() const
											{ return *mError; }

	// Properties
	private:
		OV<UniversalTimeInterval>	mTimeInterval;
		OV<SError>					mError;
};
