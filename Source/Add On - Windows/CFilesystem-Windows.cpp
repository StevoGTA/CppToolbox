//----------------------------------------------------------------------------------------------------------------------
//	CFilesystem-Windows.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFilesystem.h"

#undef Delete
#include <Windows.h>
#define Delete(x)		{ delete x; x = nil; }

#include <winrt/Windows.Foundation.Collections.h>

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
TIResult<SFoldersFiles> CFilesystem::getFoldersFiles(const CFolder& folder, bool deep)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return TIResult<SFoldersFiles>(SError::mUnimplemented);
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<TArray<CFolder> > CFilesystem::getFolders(const CFolder& folder, bool deep)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return TIResult<TArray<CFolder> >(SError::mUnimplemented);
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<TArray<CFile> > CFilesystem::getFiles(const CFolder& folder, bool deep)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFilesystemPath	filesystemPath = folder.getFilesystemPath();

	// Find
	WIN32_FIND_DATA	findData;
	HANDLE			findHandle =
							FindFirstFile(
									filesystemPath.appendingComponent(CString(OSSTR("*"))).getString().getOSString(),
									&findData);
	TNArray<CFile>	files;
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
				files += CFile(filesystemPath.appendingComponent(CString(findData.cFileName)));
			}
		} while (FindNextFile(findHandle, &findData) != 0);
	}
	FindClose(findHandle);

	return TIResult<TArray<CFile> >(files);
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

//----------------------------------------------------------------------------------------------------------------------
SFoldersFiles CFilesystem::getFoldersFiles(const IVectorView<IStorageItem>& storageItems)
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose folders and files
	TNArray<CFolder>	folders;
	TNArray<CFile>		files;
	for (auto const& storageItem : storageItems) {
		// Check if directory
		if ((uint32_t) storageItem.Attributes() & (uint32_t) FileAttributes::Directory)
			// Folder
			folders += CFolder(CFilesystemPath(CString(storageItem.Path().data())));
		else
			// File
			files += CFile(CFilesystemPath(CString(storageItem.Path().data())));
	}

	return SFoldersFiles(folders, files);
}
