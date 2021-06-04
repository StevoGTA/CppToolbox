//----------------------------------------------------------------------------------------------------------------------
//	SAudioSourceStatus.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SAudioSourceStatus

struct SAudioSourceStatus {
							// Lifecycle methods
							SAudioSourceStatus(const SError& error) : mError(OI<SError>(error)) {}
							SAudioSourceStatus(Float32 percentConsumed) : mPercentConsumed(percentConsumed) {}

							// Instance methods
				bool		isSuccess() const
								{ return !mError.hasInstance(); }
		const	OV<Float32>	getPercentConsumed() const
								{ return mPercentConsumed; }
		const	OI<SError>&	getError() const
								{ return mError; }

	// Properties
	private:
		OV<Float32>	mPercentConsumed;
		OI<SError>	mError;
};
