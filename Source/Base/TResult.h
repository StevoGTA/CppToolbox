//----------------------------------------------------------------------------------------------------------------------
//	TResult.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: TIResult (Instance)

template <typename T> struct TIResult {
					// Lifecycle Methods
					TIResult(const T& instance) : mInstance(OI<T>(instance)) {}
					TIResult(const OI<T>& instance) : mInstance(instance) {}
					TIResult(const SError& error) : mError(OV<SError>(error)) {}
					TIResult(const TIResult& other) : mInstance(other.mInstance), mError(other.mError) {}

					// Instance Methods
			bool	hasInstance() const
						{ return mInstance.hasInstance(); }
	const	OI<T>	getOptionalInstance() const
						{ return mInstance; }
	const	T&		getInstance() const
						{ AssertFailIf(!mInstance.hasInstance()); return *mInstance; }

			bool	hasError() const
						{ return mError.hasValue(); }
	const	SError&	getError() const
						{ AssertFailIf(!mError.hasValue()); return *mError; }

	const	T&		operator*() const
						{ AssertFailIf(!mInstance.hasInstance()); return *mInstance; }
	const	T*		operator->() const
						{ AssertFailIf(!mInstance.hasInstance()); return &(*mInstance); }

	private:
		OI<T>		mInstance;
		OV<SError>	mError;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TVResult (Value)

template <typename T> struct TVResult {
					// Lifecycle Methods
					TVResult(const T value) : mValue(OV<T>(value)) {}
					TVResult(const SError& error) : mError(OV<SError>(error)) {}
					TVResult(const TVResult& other) : mValue(other.mValue), mError(other.mError) {}

					// Instance Methods
			bool	hasValue() const
						{ return mValue.hasValue(); }
	const	T		getValue() const
						{ return *mValue; }

			bool	hasError() const
						{ return mError.hasValue(); }
	const	SError&	getError() const
						{ AssertFailIf(!mError.hasValue()); return *mError; }

	const	T&		operator*() const
						{ AssertFailIf(!mValue.hasValue()); return *mValue; }
	const	T*		operator->() const
						{ AssertFailIf(!mValue.hasValue()); return &(*mValue); }

	private:
		OV<T>		mValue;
		OV<SError>	mError;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Macros

#define ReturnIfResultError(result)				{ if (result.hasError()) return; }
#define ReturnErrorIfResultError(result)		{ if (result.hasError()) return OV<SError>(result.getError()); }
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
