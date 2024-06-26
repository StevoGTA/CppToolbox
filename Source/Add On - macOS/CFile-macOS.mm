//----------------------------------------------------------------------------------------------------------------------
//	CFile-macOS.mm			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFile.h"

#include "CCoreFoundation.h"
#include "SError-Apple.h"

#include <Foundation/Foundation.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	CFileReportErrorAndReturnError(error, message)										\
				{																			\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);	\
					logAsError(CString::mSpaceX4);											\
																							\
					return error;															\
				}
#define	CFileReportErrorAndReturnValue(error, message, value)								\
				{																			\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);	\
					logAsError(CString::mSpaceX4);											\
																							\
					return value;															\
				}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFile

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
bool CFile::isHidden() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get URL
	NSURL*	url = (NSURL*) CFBridgingRelease(CCoreFoundation::createURLRefFrom(getFilesystemPath(), false));

	// Get info
	NSNumber*	number;

	return [url getResourceValue:&number forKey:NSURLIsHiddenKey error:nil] && number.boolValue;
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTime CFile::getCreationUniversalTime() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get URL
	NSURL*	url = (NSURL*) CFBridgingRelease(CCoreFoundation::createURLRefFrom(getFilesystemPath(), false));

	// Get info
	NSDate*		date;
	NSError*	error;
	if ([url getResourceValue:&date forKey:NSURLCreationDateKey error:&error])
		// Got date
		return date.timeIntervalSinceReferenceDate;
	else
		// Didn't succeed
		CFileReportErrorAndReturnValue(SErrorFromNSError(error), "getting creation date", 0.0);
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTime CFile::getModificationUniversalTime() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get URL
	NSURL*	url = (NSURL*) CFBridgingRelease(CCoreFoundation::createURLRefFrom(getFilesystemPath(), false));

	// Get info
	NSDate*		date;
	NSError*	error;
	if ([url getResourceValue:&date forKey:NSURLContentModificationDateKey error:&error])
		// Got date
		return date.timeIntervalSinceReferenceDate;
	else
		// Didn't succeed
		CFileReportErrorAndReturnValue(SErrorFromNSError(error), "getting modification date", 0.0);
}

//----------------------------------------------------------------------------------------------------------------------
bool CFile::isAlias() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get URL
	NSURL*	url = (NSURL*) CFBridgingRelease(CCoreFoundation::createURLRefFrom(getFilesystemPath(), false));

	// Get info
	CFBooleanRef	booleanRef;
	bool			isAlias =
							::CFURLCopyResourcePropertyForKey((__bridge CFURLRef) url, kCFURLIsAliasFileKey,
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
	// Get URL
	NSURL*	url = (NSURL*) CFBridgingRelease(CCoreFoundation::createURLRefFrom(getFilesystemPath(), false));

	// Get info
	MDItemRef	itemRef = ::MDItemCreateWithURL(kCFAllocatorDefault, (__bridge CFURLRef) url);
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
	[appleScriptString appendFormat:@"SET filePath TO \"%@\" AS POSIX FILE\n", (__bridge NSString*) stringRef];
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
