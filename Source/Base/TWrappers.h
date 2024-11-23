//----------------------------------------------------------------------------------------------------------------------
//	TWrappers.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CppToolboxAssert.h"
#include "CHashable.h"

/*
	// Top-level definitions

	An "Instance" is a classic C++ instance - something created with "new".  The significance here is that the instance
		needs to be maintained as it is passed around.  IOW it is likely a subclass that cannot be re-created as it is
		not passed around how to do so.

	A "Value" is any POD or object that can be exactly duplicated via new(*other).

	A "Reference" is typically used to pass a "reference" to an object - "Instance" or "Value" where it is desired to
		access the original object without re-creating or copying it.

	For objects that are reference counted, it is up to the implementation as to whether these should be "Instances" or
		"Values".  The concept is likely driven by "reference" vs "value" semantics.  If it is desired to pass around an
		object such that multiple holders can affect its state, use "Instance" for "reference" semantics.  However, if
		it is desired to pass around an immutable object, feel free to use "Value" for "value" semantics.
*/

//----------------------------------------------------------------------------------------------------------------------
// MARK: I (Instance)
//	An Instance is something that needs to be maintained as it is passed around.  It cannot be copied as it is not
//	possible to know exactly how it is to be copied

template <typename T> class I : public CHashable {
	// Methods
	public:
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

						// CEquatable methods
				bool	operator==(const CEquatable& other) const
							{ return mInstance == ((const I&) other).mInstance; }

						// CHashable methods
				void	hashInto(CHashable::HashCollector& hashableHashCollector) const
							{ hashableHashCollector.add((const UInt8*) mInstance, sizeof(T*)); }

						// Instance methods
				T&		operator*() const
							{ return *mInstance; }
				T*		operator->() const
							{ return mInstance; }

				I<T>&	operator=(const I<T>& other)
							{
								// Check for instance
								if (--(*mReferenceCount) == 0) {
									// All done
									Delete(mInstance);
									Delete(mReferenceCount);
								}

								// Copy
								mInstance = other.mInstance;
								mReferenceCount = other.mReferenceCount;

								// Additional reference
								(*mReferenceCount)++;

								return *this;
							}

		static	bool	doesInstanceMatch(const I<T>& reference, T* instance)
							{ return reference.mInstance == instance; }

	// Properties
	private:
		T*		mInstance;
		UInt32*	mReferenceCount;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - OI (Optional Instance)
/*
	Examples:

	OV<CString>	optionalString1
	optionalString1.hasInstance();	// false
	optionalString1.getObject();	// Assert fail

	OV<CString>	optionalString2(CString(OSSTR("Hello World!")));
	optionalString2.hasInstance();	// true
	optionalString2.getObject();	// CString&
	*optionalString2;				// CString&
	optionalString2->isEmpty();		// false



	OV<CString>	string1;						// No instance
	OV<CString>	string2(CString(OSSTR("abc")));	// Instance

	OV<CString>	string3 = string1;

	OV<CString>	string4 = procThatReturnsString();

	string3.hasInstance();	// true
	string3.getObject();	// CString&
	*string3;				// CString&
	string3->isEmpty();		// false
 */

template <typename T> struct OI {
	// Methods
	public:
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
		void	setInstance(T* instance = nil)
					{
						// Check for instance
						if ((mInstance != nil) && (--(*mReferenceCount) == 0)) {
							// All done
							Delete(mInstance);
							Delete(mReferenceCount);
						}

						// Store
						mInstance = instance;

						// Finish setup
						if (mInstance != nil) {
							// Have instance
							mReferenceCount = new UInt32;
							*mReferenceCount = 1;
						} else {
							// Don't have instance
							Delete(mReferenceCount);
						}
					}

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
							(hasInstance() && !(*mInstance == *other.mInstance)); }

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
	// Methods
	public:
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
	// Methods
	public:
				// Lifecycle methods
				R(T& reference) : mReference(&reference) {}
				R(const R<T>& other) : mReference(other.mReference) {}

				// Instamce methods
		T&		operator*() const
					{ return *mReference; }
		T*		operator->() const
					{ return mReference; }

		bool	operator==(const R<T>& other) const
					{ return mReference == other.mReference; }

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
	// Methods
	public:
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

		bool	operator==(const OR<T>& other) const
					{ return (hasReference() == other.hasReference()) &&
							(!hasReference() || (*mReference == *other.mReference)); }
		bool	operator!=(const OR<T>& other) const
					{ return (hasReference() != other.hasReference()) ||
							(hasReference() && !(*mReference == *other.mReference)); }

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
	// Methods
	public:
				// Lifecycle methods
				OV() : mValue(nil) {}
				OV(T value) : mValue(new T(value)) {}
				OV(const OV& other) : mValue((other.mValue != nil) ? new T(*other.mValue) : nil) {}
				~OV() { Delete(mValue); }

				// Instamce methods
		bool	hasValue() const
					{ return mValue != nil; }
		T&		getValue() const
					{ AssertFailIf(mValue == nil); return *mValue; }
		T		getValue(T defaultValue) const
					{ return (mValue != nil) ? *mValue : defaultValue; }
		void	setValue(T value)
					{ if (mValue != nil) *mValue = value; else mValue = new T(value); }
		void	setValue(const OV<T>& value)
					{
						// Check situation
						if ((mValue != nil) && value.hasValue())
							// Update value
							*mValue = *value;
						else if (mValue != nil) {
							// No longer have a value
							Delete(mValue);
						} else if (value.hasValue())
							// Now have a value
							mValue = new T(*value);
					}
		void	removeValue()
					{ Delete(mValue); }

		T&		operator*() const
					{ AssertFailIf(mValue == nil); return *mValue; }
		T*		operator->() const
					{ AssertFailIf(mValue == nil); return mValue; }

		OV<T>&	operator=(T value)
					{ setValue(value); return *this; }
		OV<T>&	operator=(const OV<T>& value)
					{ setValue(value); return *this; }

		bool	operator==(const OV<T>& other) const
					{ return (hasValue() == other.hasValue()) && (!hasValue() || (*mValue == *other.mValue)); }
		bool	operator!=(const OV<T>& other) const
					{ return (hasValue() != other.hasValue()) || (hasValue() && !(*mValue == *other.mValue)); }

	// Properties
	private:
		T*	mValue;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Macros

#define	ReturnValueIfNoValue(instance, value)	{ if (!instance.hasValue()) return value; }
