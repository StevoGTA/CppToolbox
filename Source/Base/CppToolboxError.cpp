//----------------------------------------------------------------------------------------------------------------------
//	CppToolboxError.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CppToolboxError.h"

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	TKeyConvertibleDictionary<UError, CString*>*	sErrorMap = nil;

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
		sErrorMap = new TKeyConvertibleDictionary<UError, CString*>();

	// Add
	sErrorMap->set(error, new CString(string));
}

//----------------------------------------------------------------------------------------------------------------------
CString CErrorRegistry::getStringForError(UError error)
//----------------------------------------------------------------------------------------------------------------------
{
//	// Check for no error
//	if (error == kNoError)
//		return CString("No Error");
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
//			case kMemFullError:			return CString("Memory is full");
//			case kParamError:			return CString("Parameter is invalid");
//			case kAssertFailedError:	return CString("Assertion Failed");
//			case kNilValueError:		return CString("nil value");
//			case kNonNilValueError:		return CString("non nil value");
//			case kUnimplementedError:	return CString("Unimplemented");
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
//			return CString("\"") + string + CString("\"");
//
//		// Fallback
//		switch (status) {
//			// OS Errors
//			case paramErr:			return CString("Parameter Error");
//			case fnfErr:			return CString("File Not Found");
//			case eofErr:			return CString("End of File");
//			case fnOpnErr:			return CString("File Not Open");
//			case ioErr:				return CString("I/O Error");
//			case afpAccessDenied:	return CString("Insufficient Access Privileges");
//			case dskFulErr:			return CString("Disk is Full");
//
//			// Quicktime Errors
//			case invalidDuration:	return CString("Invalid Duration");
//			case noMovieFound:		return CString("No Movie Found");
//
//			default:				return CString("(OSStatus) ") + CString(status);
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
		CString("Domain: ") + CString(errorDomain, true, true) + CString(", Error: ") + CString(errorError) :
		CString("Domain: ") + CString(errorDomain, true, true) + CString(", Error: ") +
				CString((OSType) errorError, true, true);
}
