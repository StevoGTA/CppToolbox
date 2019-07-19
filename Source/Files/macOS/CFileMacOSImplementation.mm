//----------------------------------------------------------------------------------------------------------------------
//	CFileMacOSImplementation.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFile.h"

#include "CFUtilities.h"

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

////----------------------------------------------------------------------------------------------------------------------
//CImage CFile::getImage() const
////----------------------------------------------------------------------------------------------------------------------
//{
//	CFStringRef	stringRef = eStringCopyCFStringRef(mInternals->mURL.getFilesystemPath());
//	NSImage*	image = [[NSWorkspace sharedWorkspace] iconForFile:(__bridge NSString*) stringRef];
//	::CFRelease(stringRef);
//
//	return CImage(image);
//}

//----------------------------------------------------------------------------------------------------------------------
bool CFile::isHidden() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get URL
	NSURL*	url = (NSURL*) CFBridgingRelease(eFilesystemPathCopyURLRef(getFilesystemPath(), false));

	// Get info
	NSNumber*	number;

	return [url getResourceValue:&number forKey:NSURLIsHiddenKey error:nil] && number.boolValue;
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTime CFile::getCreationDate() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get URL
	NSURL*	url = (NSURL*) CFBridgingRelease(eFilesystemPathCopyURLRef(getFilesystemPath(), false));

	// Get info
	NSDate*		date;
	NSError*	error;
	if ([url getResourceValue:&date forKey:NSURLCreationDateKey error:&error])
		// Got date
		return date.timeIntervalSinceReferenceDate;
	else
		// Didn't succeed
		CFileReportErrorAndReturnError(MAKE_UErrorFromNSError(error), "getting creation date");
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTime CFile::getModificationDate() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get URL
	NSURL*	url = (NSURL*) CFBridgingRelease(eFilesystemPathCopyURLRef(getFilesystemPath(), false));

	// Get info
	NSDate*		date;
	NSError*	error;
	if ([url getResourceValue:&date forKey:NSURLContentModificationDateKey error:&error])
		// Got date
		return date.timeIntervalSinceReferenceDate;
	else
		// Didn't succeed
		CFileReportErrorAndReturnError(MAKE_UErrorFromNSError(error), "getting modification date");
}

//----------------------------------------------------------------------------------------------------------------------
bool CFile::isAlias() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get URL
	NSURL*	url = (NSURL*) CFBridgingRelease(eFilesystemPathCopyURLRef(getFilesystemPath(), false));

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
CString CFile::getComment() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get URL
	NSURL*	url = (NSURL*) CFBridgingRelease(eFilesystemPathCopyURLRef(getFilesystemPath(), false));

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

			return string;
		} else
			// Must not be a comment
			return CString::mEmpty;
	} else
		// Error
		CFileReportErrorAndReturnValue(kParamError, "getting comment", CString::mEmpty);
}

//----------------------------------------------------------------------------------------------------------------------
UError CFile::setComment(const CString& string) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFStringRef	stringRef = eFilesystemPathCopyStringRef(getFilesystemPath());

	// Compose AppleScript
	NSMutableString*	appleScriptString = [NSMutableString string];
	[appleScriptString appendString:@"TELL APPLICATION \"FINDER\"\n"];
	[appleScriptString appendFormat:@"SET filePath TO \"%@\" AS POSIX FILE\n", (__bridge NSString*) stringRef];
	[appleScriptString appendFormat:@"SET COMMENT OF (filePath AS ALIAS) TO \"%@\"\n", stringRef];
	[appleScriptString appendString:@"END TELL"];

	// Cleanup
	::CFRelease(stringRef);

	// Run it!
	NSAppleScript*	appleScript = [[NSAppleScript alloc] initWithSource:appleScriptString];
	NSDictionary*	errorInfo;
	if ([appleScript executeAndReturnError:&errorInfo] != nil)
		// Success
		return kNoError;
	else {
		// Error
		NSLog(@"Error when setting file comment: %@", errorInfo);

		return kParamError;
	}
}
