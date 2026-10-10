//----------------------------------------------------------------------------------------------------------------------
//	CFilesystem-Windows-Win32.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFilesystem.h"

#include "CLogServices.h"
#include "SError-Windows.h"

#include <shellapi.h>
#include <shlobj_core.h>

#pragma comment(lib, "shell32")

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	CFilesystemReportFolderErrorAndReturnValue(error, message, folder, value)					\
				{																					\
					CLogServices::logError(error, message,											\
							CString(__FILE__, sizeof(__FILE__), CString::kEncodingUTF8),			\
							CString(__func__, sizeof(__func__), CString::kEncodingUTF8), __LINE__);	\
					CLogServices::logError(															\
							CString::mSpaceX4 + CString(OSSTR("Folder: ")) +						\
									folder.getFilesystemPath().getString());						\
																									\
					return value;																	\
				}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local procs

//----------------------------------------------------------------------------------------------------------------------
static TVResult<CFilesystem::VolumeInfo> sGetVolumeInfo(const CFilesystemPath& filesystemPath)
//----------------------------------------------------------------------------------------------------------------------
{
	// GetVolumeInformation() wants the volume's mount point rather than an arbitrary path below it, so resolve
	//	that first.  This is also what makes a UNC path work, where the mount point is "\\server\share\".
	WCHAR	volumePath[MAX_PATH];
	if (!::GetVolumePathNameW(filesystemPath.getString().getOSString(), volumePath, MAX_PATH))
		// Error
		return TVResult<CFilesystem::VolumeInfo>(SErrorFromWindowsGetLastError());

	// Get the filesystem name ("NTFS", "exFAT", "FAT32", ...) and the flags carrying read-only
	WCHAR	filesystemName[MAX_PATH];
	DWORD	filesystemFlags;
	if (!::GetVolumeInformationW(volumePath, NULL, 0, NULL, NULL, &filesystemFlags, filesystemName, MAX_PATH))
		// Error
		return TVResult<CFilesystem::VolumeInfo>(SErrorFromWindowsGetLastError());

	return TVResult<CFilesystem::VolumeInfo>(
			CFilesystem::VolumeInfo(CString(filesystemName), ::GetDriveTypeW(volumePath) != DRIVE_REMOTE,
					(filesystemFlags & FILE_READ_ONLY_VOLUME) != 0));
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
		CFilesystemReportFolderErrorAndReturnValue(error, CString(OSSTR("calling FindFirstFile()")), folder,
				TVResult<SFoldersFiles>(error));
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
		CFilesystemReportFolderErrorAndReturnValue(error, CString(OSSTR("calling FindFirstFile()")), folder,
				TVResult<TArray<CFile>>(error));
	}
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CFilesystem::VolumeInfo> CFilesystem::getVolumeInfo(const CFile& file)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get Volume Info
	return sGetVolumeInfo(file.getFilesystemPath());
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CFilesystem::VolumeInfo> CFilesystem::getVolumeInfo(const CFolder& folder)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get Volume Info
	return sGetVolumeInfo(folder.getFilesystemPath());
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
	// Group files by folder - each folder gets one File Explorer window with its files selected
	TNArrayDictionary<CFile>	filesByFolderPath;
	for (TArray<CFile>::Iterator iterator = files.getIterator(); iterator; iterator++)
		// Add file
		filesByFolderPath.add(iterator->getFolder().getFilesystemPath().getString(), *iterator);

	// Iterate folders
	OV<SError>	error;
	for (TNArrayDictionary<CFile>::Iterator iterator = filesByFolderPath.getIterator(); iterator; iterator++) {
		// Setup
		const	CString&			folderPath = iterator.getKey();
		const	TNArray<CFile>&		folderFiles = iterator.getValue();
				PIDLIST_ABSOLUTE	folderPIDL = ::ILCreateFromPathW(folderPath.getOSString());
				PIDLIST_ABSOLUTE*	filePIDLs = new PIDLIST_ABSOLUTE[folderFiles.getCount()];
				PCUITEMID_CHILD*	childPIDLs = new PCUITEMID_CHILD[folderFiles.getCount()];
				UINT				count = 0;

		// Collect the files that can still be found - one that has gone away cannot be selected
		if (folderPIDL != NULL)
			for (TArray<CFile>::Iterator fileIterator = folderFiles.getIterator(); fileIterator; fileIterator++) {
				// Get PIDL
				PIDLIST_ABSOLUTE	filePIDL =
											::ILCreateFromPathW(
													fileIterator->getFilesystemPath().getString().getOSString());
				if (filePIDL != NULL) {
					// Add
					filePIDLs[count] = filePIDL;
					childPIDLs[count++] = (PCUITEMID_CHILD) ::ILFindLastID(filePIDL);
				}
			}

		// Check situation
		if (count > 0) {
			// Open folder and select files
			HRESULT	result = ::SHOpenFolderAndSelectItems(folderPIDL, count, childPIDLs, 0);
			if (FAILED(result))
				// Error
				error.setValue(SErrorFromHRESULT(result));
		} else {
			// Nothing to select, so just open the folder
			HINSTANCE	result = ::ShellExecuteW(NULL, L"explore", folderPath.getOSString(), NULL, NULL, SW_SHOW);
			if (((INT_PTR) result) < 32)
				// Error
				error.setValue(SErrorFromWindowsGetLastError());
		}

		// Cleanup
		for (UINT i = 0; i < count; i++)
			::ILFree(filePIDLs[i]);
		delete[] filePIDLs;
		delete[] childPIDLs;
		::ILFree(folderPIDL);
	}

	return error;
}
#endif
