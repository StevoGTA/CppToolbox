//----------------------------------------------------------------------------------------------------------------------
//	CppToolboxError.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CppToolboxError.h"

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	TOwningKeyConvertibleDictionary<UError, CString>*	sErrorMap = nil;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: CErrorRegistry

//----------------------------------------------------------------------------------------------------------------------
void CErrorRegistry::registerError(UError error, const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	if (sErrorMap == nil)
		// Create map
		sErrorMap = new TOwningKeyConvertibleDictionary<UError, CString>();

	// Add
	sErrorMap->set(error, string);
}

//----------------------------------------------------------------------------------------------------------------------
CString CErrorRegistry::getStringForError(UError error)
//----------------------------------------------------------------------------------------------------------------------
{
//	// Check for no error
//	if (error == kNoError)
//		return CString(OSSTR("No Error"));
//
	// Check for registered error
	CString*	string = (sErrorMap != nil) ? (*sErrorMap)[error] : nil;
	if (string != nil)
		return *string;

	// Get system error info
	UErrorDomain	errorDomain = GET_UErrorDomain(error);
	UErrorError		errorError = GET_UErrorError(error);
//
//	if (errorDomain == kCoreErrorDomain) {
//		switch (error) {
//			case kMemFullError:			return CString(OSSTR("Memory is full"));
//			case kParamError:			return CString(OSSTR("Parameter is invalid"));
//			case kAssertFailedError:	return CString(OSSTR("Assertion Failed"));
//			case kNilValueError:		return CString(OSSTR("nil value"));
//			case kNonNilValueError:		return CString(OSSTR("non nil value"));
//			case kUnimplementedError:	return CString(OSSTR("Unimplemented"));
//		}
//	}
//
//#if TARGET_OS_MACOS
//	if (errorDomain == kOSStatusErrorDomain) {
//		// OSStatus
//		CString		string;
//		OSStatus	status = errorError;
//
//		// Get error string
//		string = CString(::GetMacOSStatusCommentString(status), kCStringDefaultMaxLength, kStringEncodingMacRoman);
//		if (!string.isEmpty())
//			return string;
//
//		string = CString(::GetMacOSStatusErrorString(status), kCStringDefaultMaxLength, kStringEncodingMacRoman);
//		if (!string.isEmpty())
//			return CString(OSSTR("\"")) + string + CString(OSSTR("\""));
//
//		// Fallback
//		switch (status) {
//			// OS Errors
//			case paramErr:			return CString(OSSTR("Parameter Error"));
//			case fnfErr:			return CString(OSSTR("File Not Found"));
//			case eofErr:			return CString(OSSTR("End of File"));
//			case fnOpnErr:			return CString(OSSTR("File Not Open"));
//			case ioErr:				return CString(OSSTR("I/O Error"));
//			case afpAccessDenied:	return CString(OSSTR("Insufficient Access Privileges"));
//			case dskFulErr:			return CString(OSSTR("Disk is Full"));
//
//			// Quicktime Errors
//			case invalidDuration:	return CString(OSSTR("Invalid Duration"));
//			case noMovieFound:		return CString(OSSTR("No Movie Found"));
//
//			default:				return CString(OSSTR("(OSStatus) ")) + CString(status);
//		}
//	}
//#endif
//#if TARGET_OS_MACOS || TARGET_OS_IOS || TARGET_OS_LINUX
//	if (errorDomain == kPOSIXErrorDomain)
//		// POSIX
//		return CString(strerror(errorError));
//#endif
//
//	// Return generic
	return (errorError < 0) ?
		CString(OSSTR("Domain: ")) + CString(errorDomain, true, true) + CString(OSSTR(", Error: ")) +
				CString(errorError) :
		CString(OSSTR("Domain: ")) + CString(errorDomain, true, true) + CString(OSSTR(", Error: ")) +
				CString((OSType) errorError, true, true);
}
