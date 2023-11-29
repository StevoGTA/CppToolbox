//----------------------------------------------------------------------------------------------------------------------
//	SError.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SError

struct SError {
									// Lifecycle methods
									SError(const CString& domain, SInt32 code, const CDictionary& localizationInfo,
											const CString& defaultLocalizedDescription) :
										mDomain(domain), mCode(code), mLocalizationInfo(localizationInfo),
												mDefaultLocalizedDescription(defaultLocalizedDescription)
										{}
									SError(const CString& domain, SInt32 code,
											const CString& defaultLocalizedDescription) :
										mDomain(domain), mCode(code),
												mDefaultLocalizedDescription(defaultLocalizedDescription)
										{}
									SError(const SError& other) :
										mDomain(other.mDomain), mCode(other.mCode),
												mLocalizationInfo(other.mLocalizationInfo),
												mDefaultLocalizedDescription(other.mDefaultLocalizedDescription)
										{}

									// Instance methods
			const	CString&		getDomain() const
										{ return mDomain; }
					SInt32			getCode() const
										{ return mCode; }
					CString			getDescription() const
										{ return mDomain + CString(OSSTR("/")) + CString(mCode) + CString(OSSTR(" (")) +
												mDefaultLocalizedDescription + CString(OSSTR(")")); }
			const	CString&		getDefaultLocalizedDescription() const
										{ return mDefaultLocalizedDescription; }
					CString			getLocalizedDescription() const
										{ return CString(mDomain, CString(mCode), mLocalizationInfo); }

					bool			operator==(const SError& other) const
										{ return (mDomain == other.mDomain) && (mCode == other.mCode); }
					bool			operator!=(const SError& other) const
										{ return (mDomain != other.mDomain) || (mCode != other.mCode); }

	// Properties
	public:
		static	const	SError	mCancelled;
		static	const	SError	mEndOfData;
		static	const	SError	mUnimplemented;

	private:
						CString		mDomain;
						SInt32		mCode;
						CDictionary	mLocalizationInfo;
						CString		mDefaultLocalizedDescription;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Macros

#define ReturnIfError(error)				{ if (error.hasValue()) return; }
#define ReturnErrorIfError(error)			{ if (error.hasValue()) return error; }
#define	ReturnValueIfError(error, value)	{ if (error.hasValue()) return value; }
