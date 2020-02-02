//----------------------------------------------------------------------------------------------------------------------
//	CFilesystem.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFilesystem.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	CFilesystemReportErrorAndReturnError(error, message, folder)								\
				{																					\
					if (error != kNoError) {														\
						CLogServices::logError(error, message, __FILE__, __func__, __LINE__);		\
						folder.logAsError(CString::mSpaceX4);										\
																									\
						return error;																\
					}																				\
				}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFilesystem

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
UError CFilesystem::copy(const CFolder& sourceFolder, const CFolder& destinationFolder)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	if (!sourceFolder.doesExist())
		CFilesystemReportErrorAndReturnError(kFolderDoesNotExistError, "checking source folder when copying",
				sourceFolder);

	// Internals check
	if (!destinationFolder.doesExist())
		CFilesystemReportErrorAndReturnError(kFolderDoesNotExistError, "checking destination folder when copying",
				destinationFolder);

	// Get contents of source folder
	TNArray<CFolder>	folders;
	TNArray<CFile>		files;
	UError				error;
	error = getFoldersFiles(sourceFolder, folders, files);
	ReturnErrorIfError(error);

	// Create folders in destination folder
	for (CArrayItemIndex i = 0; i < folders.getCount(); i++) {
		// Create folder
		CFolder	folder(destinationFolder.getFilesystemPath().appendingComponent(folders[i].getName()));
		error = folder.create();
		ReturnErrorIfError(error);
	}

	// Copy files
	error = copy(files, destinationFolder);
	ReturnErrorIfError(error);

	return kNoError;
}

//----------------------------------------------------------------------------------------------------------------------
UError CFilesystem::copy(const TArray<CFile> files, const CFolder& destinationFolder)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate files
	for (CArrayItemIndex i = 0; i < files.getCount(); i++) {
		// Copy this file
		UError	error = copy(files[i], destinationFolder);
		ReturnErrorIfError(error);
	}

	return kNoError;
}
