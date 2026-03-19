//----------------------------------------------------------------------------------------------------------------------
//	SError.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SError

struct SError {
	// Methods
	public:
							// Lifecycle methods
							SError(const CString& domain, SInt32 code, const CDictionary& localizationInfo,
									const CString& internalDescription) :
								mDomain(domain), mCode(code), mLocalizationInfo(localizationInfo),
										mInternalDescription(internalDescription)
								{}
							SError(const CString& domain, SInt32 code,
									const CDictionary& localizationInfo = CDictionary::mEmpty) :
								mDomain(domain), mCode(code), mLocalizationInfo(localizationInfo)
								{}
							SError(const CString& domain, SInt32 code, const CString& internalDescription) :
								mDomain(domain), mCode(code),
										mInternalDescription(internalDescription)
								{}
							SError(const SError& other) :
								mDomain(other.mDomain), mCode(other.mCode),
										mLocalizationInfo(other.mLocalizationInfo),
										mInternalDescription(other.mInternalDescription)
								{}

							// Instance methods
		const	CString&	getDomain() const
								{ return mDomain; }
				SInt32		getCode() const
								{ return mCode; }
				CString		getLocalizedDescription() const
								{ return CString::hasLocalizationFor(mDomain, CString(mCode)) ?
										CString(mDomain, CString(mCode), mLocalizationInfo) : *mInternalDescription; }

				CString		getInternalDescription() const
								{ return mDomain + CString(OSSTR("/")) + CString(mCode) + CString(OSSTR(" (")) +
										(mInternalDescription.hasValue() ?
												*mInternalDescription :
												CString(mDomain, CString(mCode), mLocalizationInfo)) +
										CString(OSSTR(")")); }

				bool		operator==(const SError& other) const
								{ return (mDomain == other.mDomain) && (mCode == other.mCode); }
				bool		operator!=(const SError& other) const
								{ return (mDomain != other.mDomain) || (mCode != other.mCode); }

	// Properties
	public:
		static	const	SError		mCancelled;
		static	const	SError		mEndOfData;
		static	const	SError		mUnimplemented;

	private:
						CString		mDomain;
						SInt32		mCode;
						CDictionary	mLocalizationInfo;

						OV<CString>	mInternalDescription;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Macros

#define ReturnIfError(error)				{ if (error.hasValue()) return; }
#define ReturnErrorIfError(error)			{ if (error.hasValue()) return error; }
#define	ReturnValueIfError(error, value)	{ if (error.hasValue()) return value; }
