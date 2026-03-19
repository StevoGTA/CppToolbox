//----------------------------------------------------------------------------------------------------------------------
//	CFile-macOS.mm			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#import "CFile.h"

#import "CFilesystem.h"
#import "SError-Apple.h"

#import <Foundation/Foundation.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	CFileReportErrorAndReturnError(error, message)															\
				{																								\
					CLogServices::logError(error, message,														\
							CString(__FILE__, sizeof(__FILE__), CString::kEncodingUTF8),						\
							CString(__func__, sizeof(__func__), CString::kEncodingUTF8), __LINE__);				\
					CLogServices::logError(																		\
							CString::mSpaceX4 + CString(OSSTR("File: ")) + getFilesystemPath().getString());	\
																												\
					return error;																				\
				}
#define	CFileReportErrorAndReturnValue(error, message, value)													\
				{																								\
					CLogServices::logError(error, message,														\
							CString(__FILE__, sizeof(__FILE__), CString::kEncodingUTF8),						\
							CString(__func__, sizeof(__func__), CString::kEncodingUTF8), __LINE__);				\
					CLogServices::logError(																		\
							CString::mSpaceX4 + CString(OSSTR("File: ")) + getFilesystemPath().getString());	\
																												\
					return value;																				\
				}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFile

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
bool CFile::isHidden() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get info
	NSNumber*	number;

	return [(__bridge NSURL*) *CFilesystem::getURLRefFor(*this) getResourceValue:&number forKey:NSURLIsHiddenKey
					error:nil] &&
			number.boolValue;
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTime CFile::getCreationUniversalTime() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get info
	NSDate*		date;
	NSError*	error;
	if ([(__bridge NSURL*) *CFilesystem::getURLRefFor(*this) getResourceValue:&date forKey:NSURLCreationDateKey
			error:&error])
		// Got date
		return date.timeIntervalSinceReferenceDate;
	else
		// Didn't succeed
		CFileReportErrorAndReturnValue(SErrorFromNSError(error), CString(OSSTR("getting creation date")), 0.0);
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTime CFile::getModificationUniversalTime() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get info
	NSDate*		date;
	NSError*	error;
	if ([(__bridge NSURL*) *CFilesystem::getURLRefFor(*this) getResourceValue:&date
			forKey:NSURLContentModificationDateKey error:&error])
		// Got date
		return date.timeIntervalSinceReferenceDate;
	else
		// Didn't succeed
		CFileReportErrorAndReturnValue(SErrorFromNSError(error), CString(OSSTR("getting modification date")), 0.0);
}

//----------------------------------------------------------------------------------------------------------------------
bool CFile::isAlias() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get info
	CFBooleanRef	booleanRef;
	bool			isAlias =
							::CFURLCopyResourcePropertyForKey(*CFilesystem::getURLRefFor(*this), kCFURLIsAliasFileKey,
											&booleanRef, nil) &&
									::CFBooleanGetValue(booleanRef);
	if (booleanRef != nil)
		::CFRelease(booleanRef);

	return isAlias;
}

//----------------------------------------------------------------------------------------------------------------------
OV<CString> CFile::getComments() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get info
	MDItemRef	itemRef = ::MDItemCreateWithURL(kCFAllocatorDefault, *CFilesystem::getURLRefFor(*this));
	if (itemRef != nil) {
		// Got itemRef
		CFStringRef	stringRef = (CFStringRef) ::MDItemCopyAttribute(itemRef, kMDItemFinderComment);
		::CFRelease(itemRef);

		if (stringRef != nil) {
			// Got string
			CString		string(stringRef);
			::CFRelease(stringRef);

			return OV<CString>(string);
		} else
			// Must not be a comment
			return OV<CString>();
	} else
		// Could not get MDItemRef
		return OV<CString>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFile::setComments(const CString& string) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFStringRef	stringRef = string.getOSString();

	// Compose AppleScript
	NSMutableString*	appleScriptString = [NSMutableString string];
	[appleScriptString appendString:@"TELL APPLICATION \"FINDER\"\n"];
	[appleScriptString appendFormat:@"SET filePath TO \"%@\" AS POSIX FILE\n",
			(__bridge NSString*) getFilesystemPath().getString().getOSString()];
	[appleScriptString appendFormat:@"SET COMMENT OF (filePath AS ALIAS) TO \"%@\"\n", stringRef];
	[appleScriptString appendString:@"END TELL"];

	// Run it!
	NSAppleScript*	appleScript = [[NSAppleScript alloc] initWithSource:appleScriptString];
	NSDictionary*	errorInfo;
	if ([appleScript executeAndReturnError:&errorInfo] != nil)
		// Success
		return OV<SError>();
	else {
		// Error
		NSLog(@"Error when setting file comment: %@", errorInfo);

		return OV<SError>(
				SError(CString(OSSTR("NSAppleScript")),
						((NSNumber*) errorInfo[NSAppleScriptErrorNumber]).intValue,
						CString((__bridge CFStringRef) errorInfo[NSAppleScriptErrorMessage])));
	}
}
