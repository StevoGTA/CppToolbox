//----------------------------------------------------------------------------------------------------------------------
//	CFilesystem.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFilesystem.h"

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
OI<CFile> CFilesystem::getDotUnderscoreFile(const CFile& file)
//----------------------------------------------------------------------------------------------------------------------
{
#if defined(TARGET_OS_MACOS)
	// macOS doesn't need to access the ._ file
	return OI<CFile>();
#else
	// Try {file}../._{filename}
	const	CFilesystemPath&	filesystemPath = file.getFilesystemPath();
	CFile	dotUnderscoreFile(
					filesystemPath
							.deletingLastComponent()
							.appendingComponent(CString(OSSTR("._")) + filesystemPath.getLastComponent()));

	return dotUnderscoreFile.doesExist() ? OI<CFile>(dotUnderscoreFile) : OI<CFile>();
#endif
}

//----------------------------------------------------------------------------------------------------------------------
OI<CFile> CFilesystem::getResourceFork(const CFile& file)
//----------------------------------------------------------------------------------------------------------------------
{
#if defined(TARGET_OS_MACOS)
	// Try {file}/..namedfork/rsrc
	const	CFilesystemPath&	filesystemPath = file.getFilesystemPath();
	CFile	resourceFork(
					filesystemPath
							.appendingComponent(CString(OSSTR("..namedfork")))
							.appendingComponent(CString(OSSTR("rsrc"))));

	return resourceFork.doesExist() ? OI<CFile>(resourceFork) : OI<CFile>();
#else
	// Unsupported
	return OI<CFile>();
#endif
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFilesystem::copy(const CFolder& sourceFolder, const CFolder& destinationFolder)
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
	TIResult<SFoldersFiles>	foldersFilesResult = getFoldersFiles(sourceFolder, true);
	ReturnValueIfResultError(foldersFilesResult, OI<SError>(foldersFilesResult.getError()));

	// Create folders in destination folder
	const	SFoldersFiles&	foldersFiles = *foldersFilesResult;
	const	TArray<CFolder>	folders = foldersFiles.getFolders();
	for (CArray::ItemIndex i = 0; i < folders.getCount(); i++) {
		// Create folder
		CFolder		folder(destinationFolder.getFilesystemPath().appendingComponent(folders[i].getName()));
		OI<SError>	error = folder.create();
		ReturnErrorIfError(error);
	}

	// Copy files
	OI<SError>	error = copy(foldersFiles.getFiles(), destinationFolder);
	ReturnErrorIfError(error);

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFilesystem::copy(const TArray<CFile> files, const CFolder& destinationFolder)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate files
	for (CArray::ItemIndex i = 0; i < files.getCount(); i++) {
		// Copy this file
		OI<SError>	error = copy(files[i], destinationFolder);
		ReturnErrorIfError(error);
	}

	return OI<SError>();
}
