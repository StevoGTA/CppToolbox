//----------------------------------------------------------------------------------------------------------------------
//	CFilesystemMacOSImplementation.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFilesystem.h"

#include "CFUtilities.h"

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
					return error;																	\
				}
#define	CFilesystemReportErrorFileFolderX2AndReturnError(error, message, fileFolder1, fileFolder2)	\
				{																					\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);			\
					fileFolder1.logAsError(CString::mSpaceX4);										\
					fileFolder2.logAsError(CString::mSpaceX4);										\
																									\
					return error;																	\
				}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: CFilesystem

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
UError CFilesystem::getFolders(const CFolder& folder, TArray<CFolder>& outFolders)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get URL
	NSURL*	url = (NSURL*) CFBridgingRelease(eFilesystemPathCopyURLRef(folder.getFilesystemPath(), false));

	NSError*			error;
	NSFileManager*		fileManager = [NSFileManager defaultManager];
	NSArray<NSURL*>*	URLs =
								[fileManager contentsOfDirectoryAtURL:url includingPropertiesForKeys:nil options:0
										error:&error];
	if (URLs != nil) {
		// Iterate URLs
		for (NSURL* url in URLs) {
			// Determine if file or folder
			NSNumber*	number;
			if ([url getResourceValue:&number forKey:NSURLIsDirectoryKey error:nil] && number.boolValue)
				// Folder
				outFolders += CFolder(CFilesystemPath(CString((__bridge CFStringRef) url.path)));
		}

		return kNoError;
	} else
		// Error
		CFilesystemReportErrorFileFolderX1AndReturnError(MAKE_UErrorFromNSError(error), "getting folders", folder);
}

//----------------------------------------------------------------------------------------------------------------------
UError CFilesystem::getFiles(const CFolder& folder, TArray<CFile>& outFiles)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get URL
	NSURL*	url = (NSURL*) CFBridgingRelease(eFilesystemPathCopyURLRef(folder.getFilesystemPath(), false));

	NSError*			error;
	NSFileManager*		fileManager = [NSFileManager defaultManager];
	NSArray<NSURL*>*	URLs =
								[fileManager contentsOfDirectoryAtURL:url includingPropertiesForKeys:nil options:0
										error:&error];
	if (URLs != nil) {
		// Iterate URLs
		for (NSURL* url in URLs) {
			// Determine if file or folder
			NSNumber*	number;
			if (![url getResourceValue:&number forKey:NSURLIsDirectoryKey error:nil] && number.boolValue)
				// File
				outFiles += CFile(CFilesystemPath(CString((__bridge CFStringRef) url.path)));
		}

		return kNoError;
	} else
		// Error
		CFilesystemReportErrorFileFolderX1AndReturnError(MAKE_UErrorFromNSError(error), "getting files", folder);
}

//----------------------------------------------------------------------------------------------------------------------
UError CFilesystem::getFoldersFiles(const CFolder& folder, TArray<CFolder>& outFolders, TArray<CFile>& outFiles)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get URL
	NSURL*	url = (NSURL*) CFBridgingRelease(eFilesystemPathCopyURLRef(folder.getFilesystemPath(), false));

	NSError*			error;
	NSFileManager*		fileManager = [NSFileManager defaultManager];
	NSArray<NSURL*>*	URLs =
								[fileManager contentsOfDirectoryAtURL:url includingPropertiesForKeys:nil options:0
										error:&error];
	if (URLs != nil) {
		// Iterate URLs
		for (NSURL* url in URLs) {
			// Determine if file or folder
			NSNumber*	number;
			if ([url getResourceValue:&number forKey:NSURLIsDirectoryKey error:nil] && number.boolValue)
				// Folder
				outFolders += CFolder(CFilesystemPath(CString((__bridge CFStringRef) url.path)));
			else
				// File
				outFiles += CFile(CFilesystemPath(CString((__bridge CFStringRef) url.path)));
		}

		return kNoError;
	} else
		// Error
		CFilesystemReportErrorFileFolderX1AndReturnError(MAKE_UErrorFromNSError(error), "getting folders and files",
				folder);
}

//----------------------------------------------------------------------------------------------------------------------
UError CFilesystem::copy(const CFile& file, const CFolder& destinationFolder)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	if (!file.doesExist())
		CFilesystemReportErrorFileFolderX1AndReturnError(kCFileDoesNotExistError, "checking source file", file);

	// Setup
	CFile	destinationFile(destinationFolder.getFilesystemPath().appendingComponent(file.getName()));
	if (destinationFile.doesExist()) {
		// Destination file already exists
		UError	error = destinationFile.remove();
		if (error != kNoError)
			CFilesystemReportErrorFileFolderX1AndReturnError(error, "removing destination file", destinationFile);
	}

	// Setup
	NSURL*	sourceURL = (NSURL*) CFBridgingRelease(eFilesystemPathCopyURLRef(file.getFilesystemPath(), false));
	NSURL*	destinationURL =
					(NSURL*) CFBridgingRelease(eFilesystemPathCopyURLRef(destinationFile.getFilesystemPath(), false));

	// Do the copy
	NSError*	error;
	if ([[NSFileManager defaultManager] copyItemAtURL:sourceURL toURL:destinationURL error:&error]) {
		// Copy comment
		CString	string = file.getComment();
		if (!string.isEmpty())
			// Read comment, write to dest
			destinationFile.setComment(string);

		return kNoError;
	} else
		// Error
		CFilesystemReportErrorFileFolderX2AndReturnError(MAKE_UErrorFromNSError(error), "copying file", file,
				destinationFolder);
}

//----------------------------------------------------------------------------------------------------------------------
UError CFilesystem::replace(const CFile& sourceFile, const CFile& destinationFile)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	NSURL*	sourceURL = (NSURL*) CFBridgingRelease(eFilesystemPathCopyURLRef(sourceFile.getFilesystemPath(), false));
	NSURL*	destinationURL =
					(NSURL*) CFBridgingRelease(eFilesystemPathCopyURLRef(destinationFile.getFilesystemPath(), false));

	// Replace contents
	NSError*	error;
	if ([[NSFileManager defaultManager] replaceItemAtURL:destinationURL withItemAtURL:sourceURL backupItemName:nil
			options:0 resultingItemURL:nil error:&error])
		// Success
		return kNoError;
	else
		// Error
		CFilesystemReportErrorFileFolderX2AndReturnError(MAKE_UErrorFromNSError(error), "replacing file", sourceFile,
				destinationFile);
}

//----------------------------------------------------------------------------------------------------------------------
UError CFilesystem::open(const TArray<CFile> files, const CApplicationObject& applicationObject)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	NSMutableArray<NSURL*>*	urls = [NSMutableArray array];
	for (CArrayItemIndex i = 0; i < files.getCount(); i++)
		[urls addObject:(NSURL*) CFBridgingRelease(eFilesystemPathCopyURLRef(files[i].getFilesystemPath(), false))];

	NSURL*	applicationURL =
					(NSURL*) CFBridgingRelease(eFilesystemPathCopyURLRef(applicationObject.getFilesystemPath(), true));
	// Open
	NSError*	error;
	if ([[NSWorkspace sharedWorkspace] openURLs:urls withApplicationAtURL:applicationURL options:0 configuration:@{}
			error:&error])
		// Success
		return kNoError;
	else
		// Error
		CFilesystemReportErrorFileFolderX1AndReturnError(MAKE_UErrorFromNSError(error), "opening files with",
				applicationObject);
}

//----------------------------------------------------------------------------------------------------------------------
void CFilesystem::moveToTrash(const TArray<CFile> files, TArray<CFile>& outUntrashedFiles)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate files
	for (CArrayItemIndex i = 0; i < files.getCount(); i++) {
		// Move this file to the trash
		NSURL*		url = (NSURL*) CFBridgingRelease(eFilesystemPathCopyURLRef(files[i].getFilesystemPath(), false));
		NSError*	error;
		if (![[NSFileManager defaultManager] trashItemAtURL:url resultingItemURL:nil error:&error]) {
			// Failed
			CFilesystemReportErrorFileFolderX1(MAKE_UErrorFromNSError(error), "moving file to trash", files[i]);

			// Add to out array
			outUntrashedFiles += files[i];
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
UError CFilesystem::moveToTrash(const TArray<CFile> files)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UError	uError = kNoError;

	// Iterate files
	for (CArrayItemIndex i = 0; i < files.getCount(); i++) {
		// Move this file to the trash
		NSURL*		url = (NSURL*) CFBridgingRelease(eFilesystemPathCopyURLRef(files[i].getFilesystemPath(), false));
		NSError*	error;
		if (![[NSFileManager defaultManager] trashItemAtURL:url resultingItemURL:nil error:&error]) {
			// Failed
			uError = MAKE_UErrorFromNSError(error);
			CFilesystemReportErrorFileFolderX1(uError, "moving file to trash", files[i]);
		}
	}

	return uError;
}

//----------------------------------------------------------------------------------------------------------------------
UError CFilesystem::revealInFinder(const CFolder& folder)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get URL
	NSURL*	url = (NSURL*) CFBridgingRelease(eFilesystemPathCopyURLRef(folder.getFilesystemPath(), false));

	// Reveal in Finder
	if ([[NSWorkspace sharedWorkspace] openURL:url])
		return kNoError;
	else
		CFilesystemReportErrorFileFolderX1AndReturnError(kCFileUnableToRevealInFinderError, "revealing in Finder",
				folder);
}

//----------------------------------------------------------------------------------------------------------------------
UError CFilesystem::revealInFinder(const TArray<CFile> files)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	NSMutableArray<NSURL*>*	urls = [NSMutableArray array];
	for (CArrayItemIndex i = 0; i < files.getCount(); i++)
		[urls addObject:(NSURL*) CFBridgingRelease(eFilesystemPathCopyURLRef(files[i].getFilesystemPath(), false))];

	// Reveal in Finder
	[[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:urls];

	return kNoError;
}
