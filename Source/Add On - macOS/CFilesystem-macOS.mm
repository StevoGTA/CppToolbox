//----------------------------------------------------------------------------------------------------------------------
//	CFilesystem-macOS.mm			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#import "CFilesystem.h"

#import "CLogServices.h"
#import "SError-Apple.h"

#import <AppKit/AppKit.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	CFilesystemReportFolderError(error, message, folder)										\
				{																					\
					CLogServices::logError(error, message,											\
							CString(__FILE__, sizeof(__FILE__), CString::kEncodingUTF8),			\
							CString(__func__, sizeof(__func__), CString::kEncodingUTF8), __LINE__);	\
					CLogServices::logError(															\
							CString::mSpaceX4 + CString(OSSTR("Folder: ")) +						\
									folder.getFilesystemPath().getString());						\
				}
#define	CFilesystemReportFileError(error, message, file)															\
				{																									\
					CLogServices::logError(error, message,															\
							CString(__FILE__, sizeof(__FILE__), CString::kEncodingUTF8),							\
							CString(__func__, sizeof(__func__), CString::kEncodingUTF8), __LINE__);					\
					CLogServices::logError(																			\
							CString::mSpaceX4 + CString(OSSTR("File: ")) + file.getFilesystemPath().getString());	\
				}
#define	CFilesystemReportFileFolderErrorAndReturnError(error, message, file, folder)								\
				{																									\
					CLogServices::logError(error, message,															\
							CString(__FILE__, sizeof(__FILE__), CString::kEncodingUTF8),							\
							CString(__func__, sizeof(__func__), CString::kEncodingUTF8), __LINE__);					\
					CLogServices::logError(																			\
							CString::mSpaceX4 + CString(OSSTR("File: ")) + file.getFilesystemPath().getString());	\
					CLogServices::logError(																			\
							CString::mSpaceX4 + CString(OSSTR("Folder: ")) +										\
									folder.getFilesystemPath().getString());										\
																													\
					return OV<SError>(error);																		\
				}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFilesystem

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFilesystem::copy(const CFile& file, const CFolder& destinationFolder)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	if (!file.doesExist())
		CFilesystemReportFileError(CFile::mDoesNotExistError, CString(OSSTR("checking source file")), file);

	// Setup
	CFile	destinationFile(destinationFolder.getFilesystemPath().appendingComponent(file.getName()));
	if (destinationFile.doesExist()) {
		// Destination file already exists
		OV<SError>	error = destinationFile.remove();
		if (error.hasValue())
			CFilesystemReportFileError(*error, CString(OSSTR("removing destination file")), destinationFile);
	}

	// Setup
	CCoreFoundation::O<CFURLRef>	sourceURLRef = getURLRefFor(file);
	CCoreFoundation::O<CFURLRef>	destinationURLRef = getURLRefFor(destinationFolder);

	// Do the copy
	NSError*	error;
	if ([[NSFileManager defaultManager] copyItemAtURL:(__bridge NSURL*) *sourceURLRef
			toURL:(__bridge NSURL*) *destinationURLRef error:&error]) {
		// Copy comments
		OV<CString>	comments = file.getComments();
		if (comments.hasValue())
			// Write to dest
			destinationFile.setComments(*comments);

		return OV<SError>();
	} else
		// Error
		CFilesystemReportFileFolderErrorAndReturnError(SErrorFromNSError(error), CString(OSSTR("copying file")), file,
				destinationFolder);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFilesystem::open(const TArray<CFile>& files, CFURLRef applicationURLRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	NSMutableArray<NSURL*>*	urls = [NSMutableArray array];
	for (TArray<CFile>::Iterator iterator = files.getIterator(); iterator; iterator++)
		// Add URL
		[urls addObject: (__bridge NSURL*) *getURLRefFor(*iterator)];

	// Open
			dispatch_semaphore_t	semaphore = dispatch_semaphore_create(0);
	__block	NSError*				openURLsError = nil;
	[[NSWorkspace sharedWorkspace] openURLs:urls withApplicationAtURL:(__bridge NSURL*) applicationURLRef
			configuration:[NSWorkspaceOpenConfiguration configuration]
			completionHandler:^(NSRunningApplication* runningApplication, NSError* error) {
		// Store error
		openURLsError = error;

		// Done
		dispatch_semaphore_signal(semaphore);
	}];
	while (dispatch_semaphore_wait(semaphore, DISPATCH_TIME_NOW))
		// Run current RunLoop
		[[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0]];

	// Handle results
	if (openURLsError == nil)
		// Success
		return OV<SError>();
	else {
		// Error
		OV<SError>	error(SErrorFromNSError(openURLsError));
		LogError(*error, CString(OSSTR("opening files with")));

		return error;
	}
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CFilesystem::FileResult> CFilesystem::moveToTrash(const TArray<CFile>& files)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNArray<FileResult>	fileResults;

	// Iterate files
	for (TArray<CFile>::Iterator iterator = files.getIterator(); iterator; iterator++) {
		// Move this file to the trash
		NSError*	error;
		if (![[NSFileManager defaultManager]
				trashItemAtURL:(__bridge NSURL*) *getURLRefFor(*iterator)
				resultingItemURL:nil error:&error]) {
			// Failed
			SError	sError = SErrorFromNSError(error);
			CFilesystemReportFileError(sError, CString(OSSTR("moving file to trash")), (*iterator));

			// Add entry
			fileResults += FileResult(*iterator, sError);
		}
	}

	return fileResults;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFilesystem::revealInFinder(const CFolder& folder)
//----------------------------------------------------------------------------------------------------------------------
{
	// Reveal in Finder
	if ([[NSWorkspace sharedWorkspace] openURL:(__bridge NSURL*) *getURLRefFor(folder)])
		// Success
		return OV<SError>();
	else {
		// Error
		CFilesystemReportFolderError(CFile::mUnableToRevealInFinderError,
				CString(OSSTR("revealing in Finder")), folder);

		return CFile::mUnableToRevealInFinderError;
	}
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFilesystem::revealInFinder(const TArray<CFile>& files)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	NSMutableArray<NSURL*>*	urls = [NSMutableArray array];
	for (TArray<CFile>::Iterator iterator = files.getIterator(); iterator; iterator++)
		// Add URL
		[urls addObject:(__bridge NSURL*) *getURLRefFor(*iterator)];

	// Reveal in Finder
	[[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:urls];

	return OV<SError>();
}

