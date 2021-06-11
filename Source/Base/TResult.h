//----------------------------------------------------------------------------------------------------------------------
//	TResult.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: TIResult (Instance)

template <typename T> struct TIResult {
						// Lifecycle Methods
						TIResult(const T& value) : mValue(OI<T>(value)) {}
						TIResult(const SError& error) : mError(OI<SError>(error)) {}
						TIResult(const TIResult& other) : mValue(other.mValue), mError(other.mError) {}

						// Instance Methods
	const	OI<T>&		getValue() const
							{ return mValue; }
	const	OI<SError>&	getError() const
							{ return mError; }

	private:
		OI<T>		mValue;
		OI<SError>	mError;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TVResult (Value)

template <typename T> struct TVResult {
						// Lifecycle Methods
						TVResult(const T value) : mValue(OV<T>(value)) {}
						TVResult(const SError& error) : mError(OI<SError>(error)) {}
						TVResult(const TVResult& other) : mValue(other.mValue), mError(other.mError) {}

						// Instance Methods
	const	OV<T>&		getValue() const
							{ return mValue; }
	const	OI<SError>&	getError() const
							{ return mError; }

	private:
		OV<T>		mValue;
		OI<SError>	mError;
};
