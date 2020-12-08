//----------------------------------------------------------------------------------------------------------------------
//	SError.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SError

struct SError {
					// Lifecycle methods
					SError(const CString& domain, SInt32 code, const CString& defaultLocalizedDescription) :
						mDomain(domain), mCode(code), mDefaultLocalizedDescription(defaultLocalizedDescription)
						{}
					SError(const SError& other) :
						mDomain(other.mDomain), mCode(other.mCode),
								mDefaultLocalizedDescription(other.mDefaultLocalizedDescription)
						{}

					// Instance methods
			SInt32	getCode() const
						{ return mCode; }
			CString	getDescription() const
						{ return mDomain + CString(OSSTR("/")) + CString(mCode) + CString(OSSTR(" (")) +
								mDefaultLocalizedDescription + CString(OSSTR(")")); }

			bool	operator==(const SError& other) const
						{ return (mDomain == other.mDomain) && (mCode == other.mCode); }
			bool	operator!=(const SError& other) const
						{ return (mDomain != other.mDomain) || (mCode != other.mCode); }

	// Properties
	public:
		static	SError	mUnimplemented;
		static	SError	mEndOfData;

	private:
				CString	mDomain;
				SInt32	mCode;

				CString	mDefaultLocalizedDescription;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Macros

#define ReturnIfError(error)				{ if (error.hasInstance()) return; }
#define ReturnErrorIfError(error)			{ if (error.hasInstance()) return error; }
#define	ReturnValueIfError(error, value)	{ if (error.hasInstance()) return value; }
