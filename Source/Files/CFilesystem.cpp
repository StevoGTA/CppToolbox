//----------------------------------------------------------------------------------------------------------------------
//	CFilesystem.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFilesystem.h"

#include "CDotUnderscoreReader.h"
#include "CFileDataSource.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	CFilesystemReportErrorAndReturnError(error, message, folder)							\
				{																				\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);		\
					folder.logAsError(CString::mSpaceX4);										\
																								\
					return error;																\
				}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFilesystem

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
OV<CFile> CFilesystem::getDotUnderscoreFile(const CFile& file)
//----------------------------------------------------------------------------------------------------------------------
{
#if defined(TARGET_OS_MACOS)
	// macOS doesn't need to access the ._ file
	return OV<CFile>();
#else
	// Try {file}../._{filename}
	const	CFilesystemPath&	filesystemPath = file.getFilesystemPath();
	CFile	dotUnderscoreFile(
					filesystemPath
							.deletingLastComponent()
							.appendingComponent(CString(OSSTR("._")) + filesystemPath.getLastComponent()));

	return dotUnderscoreFile.doesExist() ? OV<CFile>(dotUnderscoreFile) : OV<CFile>();
#endif
}

//----------------------------------------------------------------------------------------------------------------------
OV<CFile> CFilesystem::getResourceFork(const CFile& file)
//----------------------------------------------------------------------------------------------------------------------
{
#if defined(TARGET_OS_MACOS)
	// Try {file}/..namedfork/rsrc
	const	CFilesystemPath&	filesystemPath = file.getFilesystemPath();
	CFile	resourceFork(
					filesystemPath
							.appendingComponent(CString(OSSTR("..namedfork")))
							.appendingComponent(CString(OSSTR("rsrc"))));

	return resourceFork.doesExist() ? OV<CFile>(resourceFork) : OV<CFile>();
#else
	// Unsupported
	return OV<CFile>();
#endif
}

//----------------------------------------------------------------------------------------------------------------------
OI<CAppleResourceManager> CFilesystem::getAppleResourceManager(const CFile& file)
//----------------------------------------------------------------------------------------------------------------------
{
	// Try resource file
	OV<CFile>	resourceFile = getResourceFork(file);
	if (resourceFile.hasValue() && (resourceFile->getByteCount() > 0)) {
		// Try creating Apple Resource Manager
		TIResult<CAppleResourceManager>	appleResourceManager =
												CAppleResourceManager::from(
														I<CRandomAccessDataSource>(
																new CMappedFileDataSource(*resourceFile)));
		if (appleResourceManager.hasInstance())
			// Success
			return OI<CAppleResourceManager>(*appleResourceManager);
	}

	// Try ._ file
	OV<CFile>	dotUnderscoreFile = getDotUnderscoreFile(file);
	if (dotUnderscoreFile.hasValue()) {
		// Was able to load ._ file
		TIResult<CDotUnderscoreReader>	dotUnderscoreReader =
												CDotUnderscoreReader::from(
														I<CRandomAccessDataSource>(
																new CMappedFileDataSource(*dotUnderscoreFile)));
		if (dotUnderscoreReader.hasInstance()) {
			// Get resource fork
			OR<CData>	resourceFork = dotUnderscoreReader->getResourceFork();
			if (resourceFork.hasReference()) {
				// Try creating Apple Resource Manager
				TIResult<CAppleResourceManager>	appleResourceManager =
														CAppleResourceManager::from(
																I<CRandomAccessDataSource>(
																		new CDataDataSource(*resourceFork)));
				if (appleResourceManager.hasInstance())
					// Success
					return OI<CAppleResourceManager>(*appleResourceManager);
			}
		}
	}

	return OI<CAppleResourceManager>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFilesystem::copy(const CFolder& sourceFolder, const CFolder& destinationFolder)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	if (!sourceFolder.doesExist())
		CFilesystemReportErrorAndReturnError(CFolder::mDoesNotExistError, "checking source folder when copying",
				sourceFolder);

	// Internals check
	if (!destinationFolder.doesExist())
		CFilesystemReportErrorAndReturnError(CFolder::mDoesNotExistError, "checking destination folder when copying",
				destinationFolder);

	// Get contents of source folder
	TVResult<SFoldersFiles>	foldersFilesResult = getFoldersFiles(sourceFolder, true);
	ReturnValueIfResultError(foldersFilesResult, OV<SError>(foldersFilesResult.getError()));

	// Create folders in destination folder
	const	SFoldersFiles&	foldersFiles = *foldersFilesResult;
	const	TArray<CFolder>	folders = foldersFiles.getFolders();
	for (CArray::ItemIndex i = 0; i < folders.getCount(); i++) {
		// Create folder
		CFolder		folder(destinationFolder.getFilesystemPath().appendingComponent(folders[i].getName()));
		OV<SError>	error = folder.create();
		ReturnErrorIfError(error);
	}

	// Copy files
	OV<SError>	error = copy(foldersFiles.getFiles(), destinationFolder);
	ReturnErrorIfError(error);

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFilesystem::copy(const TArray<CFile> files, const CFolder& destinationFolder)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate files
	for (CArray::ItemIndex i = 0; i < files.getCount(); i++) {
		// Copy this file
		OV<SError>	error = copy(files[i], destinationFolder);
		ReturnErrorIfError(error);
	}

	return OV<SError>();
}
