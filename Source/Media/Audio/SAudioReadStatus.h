//----------------------------------------------------------------------------------------------------------------------
//	SAudioReadStatus.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioData.h"
#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK:  SAudioReadStatus

struct SAudioReadStatus {
							// Lifecycle methods
							SAudioReadStatus(const SError& error) : mError(OI<SError>(error)) {}
							SAudioReadStatus(Float32 sourceProcessed) : mSourceProcessed(sourceProcessed) {}

							// Instance methods
				bool		isSuccess() const
								{ return !mError.hasInstance(); }
		const	OV<Float32>	getSourceProcessed() const
								{ return mSourceProcessed; }
		const	OI<SError>&	getError() const
								{ return mError; }

	// Properties
	private:
		OV<Float32>	mSourceProcessed;
		OI<SError>	mError;
};
