//----------------------------------------------------------------------------------------------------------------------
//	CFilesystem-Apple.mm			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#import "CFilesystem.h"

#import "CCoreFoundation.h"
#import "NSURL+C++.h"
#import "SError-Apple.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	CFilesystemReportErrorFileFolderX1(error, message, fileFolder)								\
				{																					\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);			\
					fileFolder.logAsError(CString::mSpaceX4);										\
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
TVResult<SFoldersFiles> CFilesystem::getFoldersFiles(const CFolder& folder, bool deep)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	NSURL*							folderURL =
											(NSURL*) CFBridgingRelease(
													CCoreFoundation::createURLRefFrom(folder.getFilesystemPath(),
															false));
	NSDirectoryEnumerationOptions	directoryEnumerationOptions =
											deep ? 0 : NSDirectoryEnumerationSkipsSubdirectoryDescendants;

	// Retrieve urls
	__block	NSError*						nsError = nil;
			NSDirectoryEnumerator<NSURL*>*	directoryEnumerator =
													[NSFileManager.defaultManager enumeratorAtURL:folderURL
															includingPropertiesForKeys:[NSArray array]
															options:directoryEnumerationOptions
															errorHandler:^BOOL(NSURL* url, NSError* error) {
																// Store
																nsError = error;

																// Stop
																return NO;
															}];
	NSMutableArray<NSURL*>*	urls = [NSMutableArray array];
	for (NSURL* url in directoryEnumerator) [urls addObject:url];

	// Check results
	if (nsError == nil)
		// Have urls
		return TVResult<SFoldersFiles>([NSURL foldersFilesFor:urls]);
	else {
		// Error
		SError	sError = SErrorFromNSError(nsError);
		CFilesystemReportErrorFileFolderX1(sError, "getting folders and files", folder);

		return TVResult<SFoldersFiles>(sError);
	}
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<TArray<CFolder> > CFilesystem::getFolders(const CFolder& folder, bool deep)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	NSURL*							folderURL =
											(NSURL*) CFBridgingRelease(
													CCoreFoundation::createURLRefFrom(folder.getFilesystemPath(),
															false));
	NSDirectoryEnumerationOptions	directoryEnumerationOptions =
											deep ? 0 : NSDirectoryEnumerationSkipsSubdirectoryDescendants;

	// Retrieve urls
	__block	NSError*						nsError = nil;
			NSDirectoryEnumerator<NSURL*>*	directoryEnumerator =
													[NSFileManager.defaultManager enumeratorAtURL:folderURL
															includingPropertiesForKeys:[NSArray array]
															options:directoryEnumerationOptions
															errorHandler:^BOOL(NSURL* url, NSError* error) {
																// Store
																nsError = error;

																// Stop
																return NO;
															}];
	NSMutableArray<NSURL*>*	urls = [NSMutableArray array];
	for (NSURL* url in directoryEnumerator) [urls addObject:url];

	// Check results
	if (nsError == nil)
		// Have urls
		return TVResult<TArray<CFolder> >([NSURL foldersFor:urls]);
	else {
		// Error
		SError	sError = SErrorFromNSError(nsError);
		CFilesystemReportErrorFileFolderX1(sError, "getting folders", folder);

		return TVResult<TArray<CFolder> >(sError);
	}
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<TArray<CFile> > CFilesystem::getFiles(const CFolder& folder, bool deep)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	NSURL*							folderURL =
											(NSURL*) CFBridgingRelease(
													CCoreFoundation::createURLRefFrom(folder.getFilesystemPath(),
															false));
	NSDirectoryEnumerationOptions	directoryEnumerationOptions =
											deep ? 0 : NSDirectoryEnumerationSkipsSubdirectoryDescendants;

	// Retrieve urls
	__block	NSError*						nsError = nil;
			NSDirectoryEnumerator<NSURL*>*	directoryEnumerator =
													[NSFileManager.defaultManager enumeratorAtURL:folderURL
															includingPropertiesForKeys:[NSArray array]
															options:directoryEnumerationOptions
															errorHandler:^BOOL(NSURL* url, NSError* error) {
																// Store
																nsError = error;

																// Stop
																return NO;
															}];
	NSMutableArray<NSURL*>*	urls = [NSMutableArray array];
	for (NSURL* url in directoryEnumerator) [urls addObject:url];

	// Check results
	if (nsError == nil)
		// Have urls
		return TVResult<TArray<CFile> >([NSURL filesFor:urls]);
	else {
		// Error
		SError	sError = SErrorFromNSError(nsError);
		CFilesystemReportErrorFileFolderX1(sError, "getting files", folder);

		return TVResult<TArray<CFile> >(sError);
	}
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFilesystem::replace(const CFile& sourceFile, const CFile& destinationFile)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	NSURL*	sourceURL =
					(NSURL*) CFBridgingRelease(
							CCoreFoundation::createURLRefFrom(sourceFile.getFilesystemPath(), false));
	NSURL*	destinationURL =
					(NSURL*) CFBridgingRelease(
							CCoreFoundation::createURLRefFrom(destinationFile.getFilesystemPath(), false));

	// Replace contents
	NSError*	error;
	if ([[NSFileManager defaultManager] replaceItemAtURL:destinationURL withItemAtURL:sourceURL backupItemName:nil
			options:0 resultingItemURL:nil error:&error])
		// Success
		return OV<SError>();
	else
		// Error
		CFilesystemReportErrorFileFolderX2AndReturnError(SErrorFromNSError(error), "replacing file", sourceFile,
				destinationFile);
}
