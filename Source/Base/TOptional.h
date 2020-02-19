//----------------------------------------------------------------------------------------------------------------------
//	TOptional.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CppToolboxAssert.h"
#include "TReferenceTracking.h"

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
			// Lifecycle methods
			OV() : mHasValue(false) {}
			OV(T value) : mHasValue(true), mValue(value) {}
			OV(const OV& other) : mHasValue(other.mHasValue), mValue(other.mValue) {}

			// Instamce methods
	bool	hasValue() const { return mHasValue; }
	T		getValue() const { AssertFailIf(!mHasValue); return mValue; }
	T		getValue(T defaultValue) const { return mHasValue ? mValue : defaultValue; }
	void	setValue(T value) { mHasValue = true; mValue = value; }
	void	removeValue() { mHasValue = false; }

	T		operator*() const { AssertFailIf(!mHasValue); return mValue; }

	OV<T>&	operator=(T value) { mHasValue = true; mValue = value; return *this; }

	// Properties
	private:
		bool	mHasValue;
		T		mValue;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - OR (Optional Reference)
/*
	Examples:

	OR<CString>	optionalString1;
	optionalString1.hasReference();	// false
	optionalString1.getReference();	// Assert fail

	CString	string(OSSTR("Hello World!"));
	OR<CString>	optionalString2(string);
	optionalString2.getReference().isEmpty();	// false
	*optionalString;							// CString&
	optionalString2->isEmpty();					// false
 */

template <typename T> struct OR {
			// Lifecycle methods
			OR() : mReference(nil) {}
			OR(T& reference) : mReference(&reference) {}
			OR(const OR& other) : mReference(other.mReference) {}

			// Instamce methods
	bool	hasReference() const { return mReference != nil; }
	T&		getReference() const { AssertFailIf(mReference == nil); return *mReference; }

	T&		operator*() const { AssertFailIf(mReference == nil); return *mReference; }
	T*		operator->() const { AssertFailIf(mReference == nil); return mReference; }

	// Properties
	private:
		T*	mReference;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - OO (Optional Object)
/*
	Examples:

	OO<CString>	optionalString1
	optionalString1.hasObject();	// false
	optionalString1.getObject();	// Assert fail

	OO<CString>	optionalString2(new CString(OSSTR("Hello World!")));
	optionalString2.hasObject();	// true
	optionalString2.getObject();	// CString&
	*optionalString2;				// CString&
	optionalString2->isEmpty();		// false



	OO<CString>	string1;						// No object
	OO<CString>	string2(CString(OSSTR("abc")));	// Object

	OO<CString>	string3 = string1;

	OO<CString>	string4 = procThatReturnsString();

	string3.hasObject();	// true
	string3.getObject();	// CString&
	*string3;				// CString&
	string3->isEmpty();		// false


 */

template <typename T> struct OO {
			// Lifecycle methods
			OO() : mObject(nil), mReferenceCount(nil) {}
			OO(T* object) : mObject(object), mReferenceCount(new UInt32) { *mReferenceCount = 1; }
			OO(const T& object) : mObject(new T(object)), mReferenceCount(new UInt32) { *mReferenceCount = 1; }
			OO(const OO<T>& other) :
				mObject(other.mObject), mReferenceCount(other.mReferenceCount)
				{
					// Check if have reference
					if (mObject != nil)
						// Additional reference
						(*mReferenceCount)++;
				}
			~OO()
				{
					// Check for object
					if ((mObject != nil) && (--(*mReferenceCount) == 0))
						// All done
						DisposeOf(mObject);
				}

	OO<T>&	operator=(const OO<T>& other)
				{
					// Check for object
					if ((mObject != nil) && (--(*mReferenceCount) == 0))
						// All done
						DisposeOf(mObject);

					// Copy
					mObject = other.mObject;
					mReferenceCount = other.mReferenceCount;

					// Note additional reference if have reference
					if (mObject != nil)
						(*mReferenceCount)++;

					return *this;
				}

			// Instamce methods
	bool	hasObject() const
				{ return mObject != nil; }
	T&		getObject() const
				{ AssertFailIf(mObject == nil); return *mObject; }

	T&		operator*() const
				{ AssertFailIf(mObject == nil); return *mObject; }
	T*		operator->() const
				{ AssertFailIf(mObject == nil); return mObject; }

	// Properties
	private:
		T*		mObject;
		UInt32*	mReferenceCount;
 };

//----------------------------------------------------------------------------------------------------------------------
// MARK: - OP (Optional Proc)
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
			// Lifecycle methods
			OP() : mProc(nil) {}
			OP(T proc) : mProc(proc) {}
			OP(const OP& other) : mProc(other.mProc) {}

			// Instamce methods
	bool	hasProc() const { return mProc != nil; }
	T		getProc() const { AssertFailIf(mProc == nil); return mProc; }

	T		operator*() const { AssertFailIf(mProc == nil); return mProc; }

	// Properties
	private:
		T	mProc;
};
