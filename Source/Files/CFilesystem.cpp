//----------------------------------------------------------------------------------------------------------------------
//	CFilesystem.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFilesystem.h"

#include "CDotUnderscoreReader.h"
#include "CFileDataSource.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	CFilesystemReportErrorAndReturnError(error, message, folder)									\
				{																						\
					CLogServices::logError(error, message,												\
							CString(__FILE__, sizeof(__FILE__), CString::kEncodingUTF8),				\
							CString(__func__, sizeof(__func__), CString::kEncodingUTF8), __LINE__);		\
					folder.logAsError(CString::mSpaceX4);												\
																										\
					return error;																		\
				}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFilesystem

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CFile CFilesystem::getDotUnderscoreFile(const CFile& file)
//----------------------------------------------------------------------------------------------------------------------
{
	// Try {file}../._{filename}
	const	CFilesystemPath&	filesystemPath = file.getFilesystemPath();
	CFile	dotUnderscoreFile(
					filesystemPath
							.deletingLastComponent()
							.appendingComponent(CString(OSSTR("._")) + *filesystemPath.getLastComponent()));

	return dotUnderscoreFile;
}

#if defined(TARGET_OS_MACOS)
//----------------------------------------------------------------------------------------------------------------------
CFile CFilesystem::getResourceFork(const CFile& file)
//----------------------------------------------------------------------------------------------------------------------
{
	// Try {file}/..namedfork/rsrc
	return CFile(
			file.getFilesystemPath()
					.appendingComponent(CString(OSSTR("..namedfork")))
					.appendingComponent(CString(OSSTR("rsrc"))));
}
#endif

//----------------------------------------------------------------------------------------------------------------------
OI<I<CRandomAccessDataSource> > CFilesystem::getResourceDataSource(const CFile& file)
//----------------------------------------------------------------------------------------------------------------------
{
#if defined(TARGET_OS_MACOS)
	// Try resource file
	CFile	resourceFile = getResourceFork(file);
	if (resourceFile.doesExist() && (resourceFile.getByteCount() > 0))
		// Success
		return OI<I<CRandomAccessDataSource> >(I<CRandomAccessDataSource>(new CMappedFileDataSource(resourceFile)));
#endif

	// Try ._ file
	CFile	dotUnderscoreFile = getDotUnderscoreFile(file);
	if (dotUnderscoreFile.doesExist()) {
		// Was able to load ._ file
		TIResult<CDotUnderscoreReader>	dotUnderscoreReader =
												CDotUnderscoreReader::from(
														I<CRandomAccessDataSource>(
																new CMappedFileDataSource(dotUnderscoreFile)));
		if (dotUnderscoreReader.hasInstance()) {
			// Get resource fork
			OR<CData>	resourceFork = dotUnderscoreReader->getResourceFork();
			if (resourceFork.hasReference())
				// Success
				return OI<I<CRandomAccessDataSource> >(I<CRandomAccessDataSource>(new CDataDataSource(*resourceFork)));
		}
	}

	return OI<I<CRandomAccessDataSource> >();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFilesystem::copy(const CFolder& sourceFolder, const CFolder& destinationFolder)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	if (!sourceFolder.doesExist())
		CFilesystemReportErrorAndReturnError(CFolder::mDoesNotExistError,
				CString(OSSTR("checking source folder when copying")), sourceFolder);

	// Internals check
	if (!destinationFolder.doesExist())
		CFilesystemReportErrorAndReturnError(CFolder::mDoesNotExistError,
				CString(OSSTR("checking destination folder when copying")), destinationFolder);

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
