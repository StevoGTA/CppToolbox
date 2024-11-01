//----------------------------------------------------------------------------------------------------------------------
//	CFilesystem-macOS.mm			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFilesystem.h"

#include "CCoreFoundation.h"
#include "SError-Apple.h"

#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	CFilesystemReportErrorFileFolderX1(error, message, fileFolder)								\
				{																					\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);			\
					fileFolder.logAsError(CString::mSpaceX4);										\
				}
#define	CFilesystemReportErrorFileFolderX1AndReturnError(error, message, fileFolder)				\
				{																					\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);			\
					fileFolder.logAsError(CString::mSpaceX4);										\
																									\
					return OV<SError>(error);														\
				}
#define	CFilesystemReportErrorFileFolderX2AndReturnError(error, message, fileFolder1, fileFolder2)	\
				{																					\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);			\
					fileFolder1.logAsError(CString::mSpaceX4);										\
					fileFolder2.logAsError(CString::mSpaceX4);										\
																									\
					return OV<SError>(error);														\
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
		CFilesystemReportErrorFileFolderX1AndReturnError(CFile::mDoesNotExistError, "checking source file", file);

	// Setup
	CFile	destinationFile(destinationFolder.getFilesystemPath().appendingComponent(file.getName()));
	if (destinationFile.doesExist()) {
		// Destination file already exists
		OV<SError>	error = destinationFile.remove();
		if (error.hasValue())
			CFilesystemReportErrorFileFolderX1AndReturnError(*error, "removing destination file", destinationFile);
	}

	// Setup
	NSURL*	sourceURL = (NSURL*) CFBridgingRelease(CCoreFoundation::createURLRefFrom(file.getFilesystemPath(), false));
	NSURL*	destinationURL =
					(NSURL*) CFBridgingRelease(
							CCoreFoundation::createURLRefFrom(destinationFile.getFilesystemPath(), false));

	// Do the copy
	NSError*	error;
	if ([[NSFileManager defaultManager] copyItemAtURL:sourceURL toURL:destinationURL error:&error]) {
		// Copy comments
		OV<CString>	comments = file.getComments();
		if (comments.hasValue())
			// Write to dest
			destinationFile.setComments(*comments);

		return OV<SError>();
	} else
		// Error
		CFilesystemReportErrorFileFolderX2AndReturnError(SErrorFromNSError(error), "copying file", file,
				destinationFolder);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFilesystem::open(const TArray<CFile>& files, const Application& application)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	NSMutableArray<NSURL*>*	urls = [NSMutableArray array];
	for (CArray::ItemIndex i = 0; i < files.getCount(); i++)
		[urls
				addObject:
						(NSURL*) CFBridgingRelease(
								CCoreFoundation::createURLRefFrom(files[i].getFilesystemPath(), false))];

	NSURL*	applicationURL =
					(NSURL*) CFBridgingRelease(
							CCoreFoundation::createURLRefFrom(application.getFilesystemPath(), true));
	// Open
			dispatch_semaphore_t	semaphore = dispatch_semaphore_create(0);
	__block	NSError*				openURLsError = nil;
	[[NSWorkspace sharedWorkspace] openURLs:urls withApplicationAtURL:applicationURL
			configuration:[NSWorkspaceOpenConfiguration configuration]
			completionHandler:^(NSRunningApplication* runningApplication, NSError* error) {
		// Store error
		openURLsError = error;

		// Done
		dispatch_semaphore_signal(semaphore);
	}];
	while (dispatch_semaphore_wait(semaphore, DISPATCH_TIME_NOW))
		[[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0]];

	// Handle results
	if (openURLsError == nil)
		// Success
		return OV<SError>();
	else
		// Error
		CFilesystemReportErrorFileFolderX1AndReturnError(SErrorFromNSError(openURLsError), "opening files with",
				application);
}

//----------------------------------------------------------------------------------------------------------------------
void CFilesystem::moveToTrash(const TArray<CFile>& files, TMArray<CFile>& outUntrashedFiles)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate files
	for (CArray::ItemIndex i = 0; i < files.getCount(); i++) {
		// Move this file to the trash
		NSURL*		url =
							(NSURL*) CFBridgingRelease(
									CCoreFoundation::createURLRefFrom(files[i].getFilesystemPath(), false));
		NSError*	error;
		if (![[NSFileManager defaultManager] trashItemAtURL:url resultingItemURL:nil error:&error]) {
			// Failed
			CFilesystemReportErrorFileFolderX1(SErrorFromNSError(error), "moving file to trash", files[i]);

			// Add to out array
			outUntrashedFiles += files[i];
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFilesystem::moveToTrash(const TArray<CFile>& files)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	OV<SError>	sError;

	// Iterate files
	for (CArray::ItemIndex i = 0; i < files.getCount(); i++) {
		// Move this file to the trash
		NSURL*		url =
							(NSURL*) CFBridgingRelease(
									CCoreFoundation::createURLRefFrom(files[i].getFilesystemPath(), false));
		NSError*	error;
		if (![[NSFileManager defaultManager] trashItemAtURL:url resultingItemURL:nil error:&error]) {
			// Failed
			sError = SErrorFromNSError(error);
			CFilesystemReportErrorFileFolderX1(*sError, "moving file to trash", files[i]);
		}
	}

	return sError;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFilesystem::revealInFinder(const CFolder& folder)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get URL
	NSURL*	url = (NSURL*) CFBridgingRelease(CCoreFoundation::createURLRefFrom(folder.getFilesystemPath(), false));

	// Reveal in Finder
	if ([[NSWorkspace sharedWorkspace] openURL:url])
		return OV<SError>();
	else
		CFilesystemReportErrorFileFolderX1AndReturnError(CFile::mUnableToRevealInFinderError, "revealing in Finder",
				folder);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFilesystem::revealInFinder(const TArray<CFile>& files)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	NSMutableArray<NSURL*>*	urls = [NSMutableArray array];
	for (CArray::ItemIndex i = 0; i < files.getCount(); i++)
		[urls
				addObject:
						(NSURL*) CFBridgingRelease(
								CCoreFoundation::createURLRefFrom(files[i].getFilesystemPath(), false))];

	// Reveal in Finder
	[[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:urls];

	return OV<SError>();
}
