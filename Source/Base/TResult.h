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
					TIResult(const OI<T>& value) : mValue(value) {}
					TIResult(const SError& error) : mError(OI<SError>(error)) {}
					TIResult(const TIResult& other) : mValue(other.mValue), mError(other.mError) {}

					// Instance Methods
			bool	hasValue() const
						{ return mValue.hasInstance(); }
	const	T&		getValue() const
						{ AssertFailIf(!mValue.hasInstance()); return *mValue; }

			bool	hasError() const
						{ return mError.hasInstance(); }
	const	SError&	getError() const
						{ AssertFailIf(!mError.hasInstance()); return *mError; }

	const	T&		operator*() const
						{ AssertFailIf(!mValue.hasInstance()); return *mValue; }

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
			bool	hasValue() const
						{ return mValue.hasInstance(); }
	const	T		getValue() const
						{ return *mValue; }

			bool	hasError() const
						{ return mError.hasInstance(); }
	const	SError&	getError() const
						{ AssertFailIf(!mError.hasInstance()); return *mError; }

	const	T&		operator*() const
						{ AssertFailIf(!mValue.hasValue()); return *mValue; }

	private:
		OV<T>		mValue;
		OI<SError>	mError;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Macros

#define ReturnIfResultError(result)				{ if (result.hasError()) return; }
#define ReturnErrorIfResultError(result)		{ if (result.hasError()) return OI<SError>(result.getError()); }
#define ReturnResultIfResultError(result)		{ if (result.hasError()) return result; }
#define	ReturnValueIfResultError(result, value)	{ if (result.hasError()) return value; }

#define LogIfResultError(result, when)																\
			{																						\
				if (result.hasError())																\
					CLogServices::logError(result.getError(), when, __FILE__, __func__, __LINE__);	\
			}

#define	LogIfResultErrorAndReturn(result, when)														\
			{																						\
				if (result.hasError()) {															\
					CLogServices::logError(result.getError(), when, __FILE__, __func__, __LINE__);	\
					return;																			\
				}																					\
			}

#define LogIfResultErrorAndReturnError(result, when)												\
			{																						\
				if (result.hasError()) {															\
					CLogServices::logError(result.getError(), when, __FILE__, __func__, __LINE__);	\
					return error;																	\
				}																					\
			}

#define	LogIfResultErrorAndReturnValue(result, when, value)											\
			{																						\
				if (result.hasError()) {															\
					CLogServices::logError(result.getError(), when, __FILE__, __func__, __LINE__);	\
					return value;																	\
				}																					\
			}
