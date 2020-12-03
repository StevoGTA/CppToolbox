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
					return OI<SError>(error);														\
				}
#define	CFilesystemReportErrorFileFolderX2AndReturnError(error, message, fileFolder1, fileFolder2)	\
				{																					\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);			\
					fileFolder1.logAsError(CString::mSpaceX4);										\
					fileFolder2.logAsError(CString::mSpaceX4);										\
																									\
					return OI<SError>(error);														\
				}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFilesystem

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFilesystem::copy(const CFile& file, const CFolder& destinationFolder)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	if (!file.doesExist())
		CFilesystemReportErrorFileFolderX1AndReturnError(CFile::mDoesNotExistError, "checking source file", file);

	// Setup
	CFile	destinationFile(destinationFolder.getFilesystemPath().appendingComponent(file.getName()));
	if (destinationFile.doesExist()) {
		// Destination file already exists
		OI<SError>	error = destinationFile.remove();
		if (error.hasInstance())
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
		// Copy comment
		CString	string = file.getComment();
		if (!string.isEmpty())
			// Read comment, write to dest
			destinationFile.setComment(string);

		return OI<SError>();
	} else
		// Error
		CFilesystemReportErrorFileFolderX2AndReturnError(SErrorFromNSError(error), "copying file", file,
				destinationFolder);
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFilesystem::open(const TArray<CFile> files, const CApplicationObject& applicationObject)
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
							CCoreFoundation::createURLRefFrom(applicationObject.getFilesystemPath(), true));
	// Open
	NSError*	error;
	if ([[NSWorkspace sharedWorkspace] openURLs:urls withApplicationAtURL:applicationURL options:0 configuration:@{}
			error:&error])
		// Success
		return OI<SError>();
	else
		// Error
		CFilesystemReportErrorFileFolderX1AndReturnError(SErrorFromNSError(error), "opening files with",
				applicationObject);
}

//----------------------------------------------------------------------------------------------------------------------
void CFilesystem::moveToTrash(const TArray<CFile> files, TArray<CFile>& outUntrashedFiles)
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
OI<SError> CFilesystem::moveToTrash(const TArray<CFile> files)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	OI<SError>	sError;

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
OI<SError> CFilesystem::revealInFinder(const CFolder& folder)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get URL
	NSURL*	url = (NSURL*) CFBridgingRelease(CCoreFoundation::createURLRefFrom(folder.getFilesystemPath(), false));

	// Reveal in Finder
	if ([[NSWorkspace sharedWorkspace] openURL:url])
		return OI<SError>();
	else
		CFilesystemReportErrorFileFolderX1AndReturnError(CFile::mUnableToRevealInFinderError, "revealing in Finder",
				folder);
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFilesystem::revealInFinder(const TArray<CFile> files)
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

	return OI<SError>();
}
