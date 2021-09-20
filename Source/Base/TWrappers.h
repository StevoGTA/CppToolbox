//----------------------------------------------------------------------------------------------------------------------
//	TWrappers.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CppToolboxAssert.h"
#include "TReferenceTracking.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: I (Instance)

template <typename T> struct I {
		// Lifecycle methods
		I(T* instance) : mInstance(instance), mReferenceCount(new UInt32) { *mReferenceCount = 1; }
		I(const I<T>& other) :
			mInstance(other.mInstance), mReferenceCount(other.mReferenceCount)
			{ (*mReferenceCount)++; }
		~I()
			{
				// One less reference
				if (--(*mReferenceCount) == 0) {
					// All done
					Delete(mInstance);
					Delete(mReferenceCount);
				}
			}

		// Instamce methods
	T&	operator*() const
			{ return *mInstance; }
	T*	operator->() const
			{ return mInstance; }

	// Properties
	private:
		T*		mInstance;
		UInt32*	mReferenceCount;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - OI (Optional Instance)
/*
	Examples:

	OI<CString>	optionalString1
	optionalString1.hasInstance();	// false
	optionalString1.getObject();	// Assert fail

	OI<CString>	optionalString2(new CString(OSSTR("Hello World!")));
	optionalString2.hasInstance();	// true
	optionalString2.getObject();	// CString&
	*optionalString2;				// CString&
	optionalString2->isEmpty();		// false



	OI<CString>	string1;						// No instance
	OI<CString>	string2(CString(OSSTR("abc")));	// Instance

	OI<CString>	string3 = string1;

	OI<CString>	string4 = procThatReturnsString();

	string3.hasInstance();	// true
	string3.getObject();	// CString&
	*string3;				// CString&
	string3->isEmpty();		// false
 */

template <typename T> struct OI {
			// Lifecycle methods
			OI() : mInstance(nil), mReferenceCount(nil) {}
			OI(T* instance) : mInstance(instance), mReferenceCount(new UInt32) { *mReferenceCount = 1; }
			OI(const T& instance) : mInstance(new T(instance)), mReferenceCount(new UInt32) { *mReferenceCount = 1; }
			OI(const OI<T>& other) :
				mInstance(other.mInstance), mReferenceCount(other.mReferenceCount)
				{
					// Check if have reference
					if (mInstance != nil)
						// Additional reference
						(*mReferenceCount)++;
				}
			~OI()
				{
					// Check for instance
					if ((mInstance != nil) && (--(*mReferenceCount) == 0)) {
						// All done
						Delete(mInstance);
						Delete(mReferenceCount);
					}
				}

			// Instamce methods
	bool	hasInstance() const
				{ return mInstance != nil; }

	T&		operator*() const
				{ AssertFailIf(mInstance == nil); return *mInstance; }
	T*		operator->() const
				{ AssertFailIf(mInstance == nil); return mInstance; }

	OI<T>&	operator=(const OI<T>& other)
				{
					// Check for instance
					if ((mInstance != nil) && (--(*mReferenceCount) == 0)) {
						// All done
						Delete(mInstance);
						Delete(mReferenceCount);
					}

					// Copy
					mInstance = other.mInstance;
					mReferenceCount = other.mReferenceCount;

					// Note additional reference if have reference
					if (mInstance != nil)
						(*mReferenceCount)++;

					return *this;
				}

	bool	operator==(const OI<T>& other) const
				{ return (hasInstance() == other.hasInstance()) &&
						(!hasInstance() || (*mInstance == *other.mInstance)); }
	bool	operator!=(const OI<T>& other) const
				{ return (hasInstance() != other.hasInstance()) ||
						(hasInstance() && (*mInstance != *other.mInstance)); }

	// Properties
	private:
		T*		mInstance;
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
	bool	hasProc() const
				{ return mProc != nil; }
	T		getProc() const
				{ AssertFailIf(mProc == nil); return mProc; }

	T		operator*() const
				{ AssertFailIf(mProc == nil); return mProc; }

	// Properties
	private:
		T	mProc;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - R (Reference)

template <typename T> struct R {
		// Lifecycle methods
		R(T& reference) : mReference(&reference) {}
		R(const R<T>& other) : mReference(other.mReference) {}

		// Instamce methods
	T&	operator*() const
			{ return *mReference; }
	T*	operator->() const
			{ return mReference; }

	// Properties
	private:
		T*	mReference;
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
	bool	hasReference() const
				{ return mReference != nil; }
	T&		getReference() const
				{ AssertFailIf(mReference == nil); return *mReference; }

	T&		operator*() const
				{ AssertFailIf(mReference == nil); return *mReference; }
	T*		operator->() const
				{ AssertFailIf(mReference == nil); return mReference; }

	// Properties
	private:
		T*	mReference;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - OV (Optional Value)
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
	bool	hasValue() const
				{ return mHasValue; }
	T		getValue() const
				{ AssertFailIf(!mHasValue); return mValue; }
	T		getValue(T defaultValue) const
				{ return mHasValue ? mValue : defaultValue; }
	void	setValue(T value)
				{ mHasValue = true; mValue = value; }
	void	removeValue()
				{ mHasValue = false; }

	T		operator*() const
				{ AssertFailIf(!mHasValue); return mValue; }

	OV<T>&	operator=(T value)
				{ mHasValue = true; mValue = value; return *this; }

	bool	operator==(const OV<T>& other) const
				{ return (mHasValue == other.mHasValue) && (!mHasValue || (mValue == other.mValue)); }
	bool	operator!=(const OV<T>& other) const
				{ return (mHasValue != other.mHasValue) || (mHasValue && (mValue != other.mValue)); }

	// Properties
	private:
		bool	mHasValue;
		T		mValue;
};
