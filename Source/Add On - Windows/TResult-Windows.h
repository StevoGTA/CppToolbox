//----------------------------------------------------------------------------------------------------------------------
//	TResult-Windows.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SError.h"
#include "TWrappers-Windows.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: TCIResult (COM Instance)

template <typename T> struct TCIResult {
					// Lifecycle Methods
					TCIResult(T* instance) : mInstance(OCI<T>(instance)) {}
					TCIResult(const OCI<T>& instance) : mInstance(instance) {}
					TCIResult(const SError& error) : mError(OV<SError>(error)) {}
					TCIResult(const TCIResult& other) : mInstance(other.mInstance), mError(other.mError) {}

					// Instance Methods
			bool	hasInstance() const
						{ return mInstance.hasInstance(); }
	const	OCI<T>&	getInstance() const
						{ return mInstance; }

			bool	hasError() const
						{ return mError.hasValue(); }
	const	SError&	getError() const
						{ return *mError; }

	const	T&		operator*() const
						{ AssertFailIf(!mInstance.hasInstance()); return *mInstance; }
	const	T*		operator->() const
						{ AssertFailIf(!mInstance.hasInstance()); return &(*mInstance); }

	private:
		OCI<T>		mInstance;
		OV<SError>	mError;
};
