//----------------------------------------------------------------------------------------------------------------------
//	CFilesystem-Windows.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFilesystem.h"

#undef Delete
#include <Windows.h>
#define Delete(x)		{ delete x; x = nil; }

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
OI<SError> CFilesystem::getFolders(const CFolder& folder, TArray<CFolder>& outFolders)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFilesystem::getFiles(const CFolder& folder, TArray<CFile>& outFiles)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFilesystemPath	filesystemPath = folder.getFilesystemPath();

	WIN32_FIND_DATA	findData;
	HANDLE			findHandle =
							FindFirstFile(
									filesystemPath.appendingComponent(CString(OSSTR("*"))).getString().getOSString(),
									&findData);
	if (findHandle != INVALID_HANDLE_VALUE) {
		//
		do {
			//
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				// Directory
				//_stprintf_s(directory, MAX_PATH, TEXT("    %s    <DIR>\n"), findData.cFileName);
				//OutputDebugString(directory);
			} else {
				// File
				//LARGE_INTEGER	fileSize;
				//fileSize.LowPart = findData.nFileSizeLow;
				//fileSize.HighPart = findData.nFileSizeHigh;
				//_stprintf_s(directory, MAX_PATH, TEXT("    %s    %lld bytes\n"), findData.cFileName, fileSize.QuadPart);
				//OutputDebugString(directory);
				outFiles += CFile(filesystemPath.appendingComponent(CString(findData.cFileName)));
			}
		} while (FindNextFile(findHandle, &findData) != 0);
	}
	FindClose(findHandle);

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFilesystem::getFoldersFiles(const CFolder& folder, TArray<CFolder>& outFolders, TArray<CFile>& outFiles)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFilesystem::copy(const CFile& file, const CFolder& destinationFolder)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFilesystem::replace(const CFile& sourceFile, const CFile& destinationFile)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return OI<SError>();
}
