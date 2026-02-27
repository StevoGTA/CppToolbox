//----------------------------------------------------------------------------------------------------------------------
//	CFilesystem-Windows-Win32.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFilesystem.h"

#include "SError-Windows.h"

#include <shellapi.h>

#pragma comment(lib, "shell32")

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
TVResult<SFoldersFiles> CFilesystem::getFoldersFiles(const CFolder& folder, bool deep)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFilesystemPath	filesystemPath = folder.getFilesystemPath();

	// Find
	WIN32_FIND_DATA	findData;
	HANDLE			findHandle =
							::FindFirstFile(
									filesystemPath.appendingComponent(CString(OSSTR("*"))).getString().getOSString(),
									&findData);
	if (findHandle != INVALID_HANDLE_VALUE) {
		// Setup
		TNArray<CFolder>	folders;
		TNArray<CFile>		files;

		// Iterate all entries
		do {
			// Setup
			CString	name(findData.cFileName);

			// Check file attributes
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				// Folder
				if ((name != CString(OSSTR("."))) && (name != CString(OSSTR("..")))) {
					// Process folder
					CFolder	childFolder(filesystemPath.appendingComponent(name));
					folders += childFolder;

					if (deep) {
						// Get files for this folder
						auto	result = getFoldersFiles(childFolder);
						if (result.hasValue()) {
							// Success
							folders += result->getFolders();
							files += result->getFiles();
						} else
							// Error
							return result;
					}
				}
			} else {
				// File
				files += CFile(filesystemPath.appendingComponent(name));
			}
		} while (::FindNextFile(findHandle, &findData) != 0);

		// Cleanup
		::FindClose(findHandle);

		return TVResult<SFoldersFiles>(SFoldersFiles(folders, files));
	} else {
		// Error
		SError	error = SErrorFromWindowsGetLastError();
		CFilesystemReportErrorFileFolderX1(error, "calling FindFirstFile()", folder);

		return TVResult<SFoldersFiles>(error);
	}
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<TArray<CFolder> > CFilesystem::getFolders(const CFolder& folder, bool deep)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return TVResult<TArray<CFolder> >(SError::mUnimplemented);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<TArray<CFile> > CFilesystem::getFiles(const CFolder& folder, bool deep)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFilesystemPath	filesystemPath = folder.getFilesystemPath();

	// Find
	WIN32_FIND_DATA	findData;
	HANDLE			findHandle =
							::FindFirstFile(
									filesystemPath.appendingComponent(CString(OSSTR("*"))).getString().getOSString(),
									&findData);
	if (findHandle != INVALID_HANDLE_VALUE) {
		// Setup
		TNArray<CFile>	files;

		// Iterate all files
		do {
			// Check file attributes
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				// Folder
				if (deep) {
					// Get files for this folder
					auto	result = getFiles(CFolder(filesystemPath.appendingComponent(CString(findData.cFileName))));
					if (result.hasValue())
						// Success
						files += *result;
					else
						// Error
						return result;
				}
			} else {
				// File
				files += CFile(filesystemPath.appendingComponent(CString(findData.cFileName)));
			}
		} while (::FindNextFile(findHandle, &findData) != 0);

		// Cleanup
		::FindClose(findHandle);

		return TVResult<TArray<CFile> >(files);
	} else {
		// Error
		SError	error = SErrorFromWindowsGetLastError();
		CFilesystemReportErrorFileFolderX1(error, "calling FindFirstFile()", folder);

		return TVResult<TArray<CFile>>(error);
	}
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFilesystem::copy(const CFile& file, const CFolder& destinationFolder)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFilesystem::replace(const CFile& sourceFile, const CFile& destinationFile)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return OV<SError>();
}

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFilesystem::revealInFileExplorer(const TArray<CFile>& files)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate files
	OV<SError>	error;
	for (TArray<CFile>::Iterator iterator = files.getIterator(); iterator; iterator++) {
		// Explore file
		HINSTANCE	result =
							ShellExecuteW(NULL, L"explore",
									iterator->getFolder().getFilesystemPath().getString().getOSString(), NULL, NULL,
									SW_SHOW);
		if (((INT_PTR) result) < 32)
			// Error
			error.setValue(SErrorFromWindowsGetLastError());
	}

	return error;
}
#endif
