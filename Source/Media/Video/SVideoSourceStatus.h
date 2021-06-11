//----------------------------------------------------------------------------------------------------------------------
//	SVideoSourceStatus.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SVideoSourceStatus

struct SVideoSourceStatus {
							// Lifecycle methods
							SVideoSourceStatus(const SError& error) : mError(OI<SError>(error)) {}
							SVideoSourceStatus(Float32 percentConsumed) : mPercentConsumed(percentConsumed) {}

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
