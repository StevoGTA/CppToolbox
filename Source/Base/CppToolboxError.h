//----------------------------------------------------------------------------------------------------------------------
//	CppToolboxError.h			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "PlatformDefinitions.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Errors

typedef	UInt64	UError;
typedef	UInt32	UErrorDomain;
typedef	SInt32	UErrorError;
#define	MAKE_UError(domain, error)	(((UInt64) domain << 32) | ((UInt64) error & 0x00000000FFFFFFFF))
#define	GET_UErrorDomain(error)		(UErrorDomain) (error >> 32)
#define GET_UErrorError(error)		(UErrorError) (error)

const	UError			kNoError				= 0;
const	UError			kGeneralFailureError	= ~0;

const	UErrorDomain	kCoreErrorDomain	= MAKE_OSTYPE('C', 'o', 'r', 'e');
const	UError			kMemFullError		= MAKE_UError(kCoreErrorDomain, MAKE_OSTYPE('M', 'e', 'M', 'F'));
const	UError			kParamError			= MAKE_UError(kCoreErrorDomain, MAKE_OSTYPE('P', 'a', 'r', 'm'));
const	UError			kAssertFailedError	= MAKE_UError(kCoreErrorDomain, MAKE_OSTYPE('A', 'S', 'R', 'T'));
const	UError			kNilValueError		= MAKE_UError(kCoreErrorDomain, MAKE_OSTYPE('n', 'i', 'l', '!'));
const	UError			kNonNilValueError	= MAKE_UError(kCoreErrorDomain, MAKE_OSTYPE('!', 'n', 'i', 'l'));
const	UError			kUnimplementedError	= MAKE_UError(kCoreErrorDomain, MAKE_OSTYPE('!', 'i', 'm', 'p'));

#if TARGET_OS_MACOS || TARGET_OS_IOS || TARGET_OS_LINUX
	const	UErrorDomain	kPOSIXErrorDomain	= MAKE_OSTYPE('P', 'o', 's', 'x');
	#define MAKE_UErrorFromPOSIXError()	MAKE_UError(kPOSIXErrorDomain, errno)
#endif

#if TARGET_OS_MACOS || TARGET_OS_IOS
	const	UErrorDomain	kCoreFoundationErrorDomain = 'CoFo';
	#define MAKE_UErrorFromCFErrorRef(errorRef)														\
					((errorRef != nil) ?															\
							MAKE_UError(kCoreFoundationErrorDomain, ::CFErrorGetCode(errorRef)) :	\
							kNoError)

	const	UErrorDomain	kMachErrorDomain = 'Mach';
	#define MAKE_UErrorFromMachError(error)	((error != 0) ? MAKE_UError(kMachErrorDomain, error) : kNoError)

	const	UErrorDomain	kOSStatusErrorDomain = 'OSSt';
	#define MAKE_UErrorFromOSStatus(status)	((status != noErr) ? MAKE_UError(kOSStatusErrorDomain, status) : kNoError)

	const	UErrorDomain	kNSErrorErrorDomain = 'NSEr';
	#define MAKE_UErrorFromNSError(nsError)	\
					((nsError.code != 0) ? MAKE_UError(kNSErrorErrorDomain, nsError.code) : kNoError)
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Macros

#define ReturnIfError(error)				{ if (error != kNoError) return; }
#define ReturnErrorIfError(error)			{ if (error != kNoError) return error; }
#define	ReturnValueIfError(error, value)	{ if (error != kNoError) return value; }

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CErrorRegistry

class CString;

class CErrorRegistry {
	// Methods
	public:
		static	void	registerError(UError error, const CString& string);
		static	CString	getStringForError(UError error);
};

//----------------------------------------------------------------------------------------------------------------------
// Futures
/*
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SError

struct SError {

			// Lifecycle methods
			SError(const CString& domain, UInt32 code, const CString& defaultLocalizedDescription) :
				mDomain(domain), mCode(code), mDefaultLocalizedDescription(defaultLocalizedDescription)
				{}
			SError(const SError& other) :
				mDomain(other.mDomain), mCode(other.mCode),
						mDefaultLocalizedDescription(other.mDefaultLocalizedDescription)
				{}

			// Instance methods
	CString	getLocalizationKey()
				{ return mDomain + CString(OSSTR("/")) + CString(mCode); }

	// Properties
	CString	mDomain;
	UInt32	mCode;

	CString	mDefaultLocalizedDescription;
};

	UError becomes OV<SError>
*/
