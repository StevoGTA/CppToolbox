//----------------------------------------------------------------------------------------------------------------------
//	TOptional.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CppToolboxAssert.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: OV (Optional Value)
/*
	Examples:

	OV<UInt32>	optionalValue1;
	optionalValue1.hasValue();	// false
	optionalValue1.getValue();	// Assert fail
	optionalValue1 = 42;
	optionalValue1.getValue();	// 42
	optionalValue1.removeValue();
	optionalValue1.getValue();	// Assert fail

	OV<UInt32>	optionalValue2(0);
	optionalValue2.hasValue();	// true
	UInt32	result1 = optionalValue2.getValue();
	UInt32	result2 = *optionalValue2;
 */

template <typename T> struct OV {
	public:
				// Lifecycle methods
				OV() : mHasValue(false) {}
				OV(T value) : mHasValue(true), mValue(value) {}
				OV(const OV& other) : mHasValue(other.mHasValue), mValue(other.mValue) {}

				// Instamce methods
		bool	hasValue() const { return mHasValue; }
		T		getValue() const { AssertFailIf(!mHasValue); return mValue; }
		void	setValue(T value) { mHasValue = true; mValue = value; }
		void	removeValue() { mHasValue = false; }

		T		operator *() const { AssertFailIf(!mHasValue); return mValue; }

		OV<T>&	operator=(T value) { mHasValue = true; mValue = value; return *this; }

	// Properties
	private:
		bool	mHasValue;
		T		mValue;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: OR (Optional Reference)
/*
	Examples:

	OR<CString>	optionalString1;
	optionalString1.hasReference();	// false
	optionalString1.getReference();	// Assert fail

	CString	string("Hello World!");
	OR<CString>	optionalString2(string);
	optionalString2.getReference().isEmpty();	// false
	*optionalString;							// CString&
	optionalString2->isEmpty();					// false
 */

template <typename T> struct OR {
	public:
				// Lifecycle methods
				OR() : mReference(nil) {}
				OR(T& reference) : mReference(&reference) {}
				OR(const OR& other) : mReference(other.mReference) {}

				// Instamce methods
		bool	hasReference() const { return mReference != nil; }
		T&		getReference() const { AssertFailIf(mReference == nil); return *mReference; }

		T&		operator *() const { AssertFailIf(mReference == nil); return *mReference; }
		T*		operator ->() const { AssertFailIf(mReference == nil); return mReference; }

	// Properties
	private:
		T*	mReference;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: OP (Optional Proc)
/*
	Examples:

	OP<CArrayApplyProc>	optionalApplyProc1;
	optionalApplyProc1.hasProc();	// false
	optionalApplyProc1.getProc();	// Assert fail

	OP<CArrayApplyProc>	optionalApplyProc2(sArrayApplyProc);
	optionalApplyProc2.getProc()(nil, nil);
	(*optionalApplyProc2)(nil, nil);
 */

template <typename T> struct OP {
	public:
				// Lifecycle methods
				OP() : mProc(nil) {}
				OP(T proc) : mProc(proc) {}
				OP(const OP& other) : mProc(other.mProc) {}

				// Instamce methods
		bool	hasProc() const { return mProc != nil; }
		T		getProc() const { AssertFailIf(mProc == nil); return mProc; }

		T		operator *() const { AssertFailIf(mProc == nil); return mProc; }

	// Properties
	private:
		T	mProc;
};
