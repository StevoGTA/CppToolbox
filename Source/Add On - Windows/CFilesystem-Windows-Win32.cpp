//----------------------------------------------------------------------------------------------------------------------
//	CFilesystem-Windows-Win32.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFilesystem.h"

#include "CLogServices.h"
#include "CUUID.h"
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
#define	CFilesystemReportFilesErrorAndReturnError(error, message, file1, file2)										\
				{																									\
					CLogServices::logError(error, message,															\
							CString(__FILE__, sizeof(__FILE__), CString::kEncodingUTF8),							\
							CString(__func__, sizeof(__func__), CString::kEncodingUTF8), __LINE__);					\
					CLogServices::logError(																			\
							CString::mSpaceX4 + CString(OSSTR("File: ")) + file1.getFilesystemPath().getString());	\
					CLogServices::logError(																			\
							CString::mSpaceX4 + CString(OSSTR("File: ")) + file2.getFilesystemPath().getString());	\
																													\
					return OV<SError>(error);																		\
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

	// Get the filesystem name ("NTFS", "exFAT", "FAT32", ...), the serial number, and the flags carrying read-only,
	//	hard link support and alternate data stream (extended attribute) support
	WCHAR	filesystemName[MAX_PATH];
	DWORD	serialNumber, filesystemFlags;
	if (!::GetVolumeInformationW(volumePath, NULL, 0, &serialNumber, NULL, &filesystemFlags, filesystemName,
			MAX_PATH))
		// Error
		return TVResult<CFilesystem::VolumeInfo>(SErrorFromWindowsGetLastError());

	// Get the cluster size
	DWORD	sectorsPerCluster, bytesPerSector;
	if (!::GetDiskFreeSpaceW(volumePath, &sectorsPerCluster, &bytesPerSector, NULL, NULL))
		// Error
		return TVResult<CFilesystem::VolumeInfo>(SErrorFromWindowsGetLastError());

	return TVResult<CFilesystem::VolumeInfo>(
			CFilesystem::VolumeInfo(CString(filesystemName), serialNumber,
					CFolder(CFilesystemPath(CString(volumePath))), sectorsPerCluster * bytesPerSector,
					::GetDriveTypeW(volumePath) != DRIVE_REMOTE, (filesystemFlags & FILE_READ_ONLY_VOLUME) != 0,
					(filesystemFlags & FILE_SUPPORTS_HARD_LINKS) != 0, (filesystemFlags & FILE_NAMED_STREAMS) != 0));
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
						auto	result = getFoldersFiles(childFolder, deep);
						if (result.hasValue()) {
							// Success
							folders += result->getFolders();
							files += result->getFiles();
						} else {
							// Error
							::FindClose(findHandle);

							return result;
						}
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
			// Setup
			CString	name(findData.cFileName);

			// Check file attributes
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				// Folder
				if (deep && (name != CString(OSSTR("."))) && (name != CString(OSSTR("..")))) {
					// Get files for this folder
					auto	result = getFiles(CFolder(filesystemPath.appendingComponent(name)), deep);
					if (result.hasValue())
						// Success
						files += *result;
					else {
						// Error
						::FindClose(findHandle);

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
TVResult<CFolder> CFilesystem::createTempFolder(const VolumeInfo& volumeInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get the volume of the user's temporary folder
	TVResult<VolumeInfo>	tempVolumeInfo = getVolumeInfo(CFolder::temp());
	ReturnValueIfResultError(tempVolumeInfo, TVResult<CFolder>(tempVolumeInfo.getError()));

	// Windows has no temporary folder on each volume.  Use the user's temporary folder when it is on the volume, and a
	//	folder at the root of the volume otherwise.
	CFolder		tempFolder =
						((tempVolumeInfo->getIdentifier() == volumeInfo.getIdentifier()) ?
										CFolder::temp() :
										volumeInfo.getRootFolder().getChildFolder(CString(OSSTR(".TemporaryItems"))))
								.getChildFolder(CUUID().getHexString());
	OV<SError>	error = tempFolder.create(true);
	ReturnValueIfError(error, TVResult<CFolder>(*error));

	return TVResult<CFolder>(tempFolder);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFilesystem::copy(const CFile& file, const CFolder& destinationFolder)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFile	destinationFile(destinationFolder.getFilesystemPath().appendingComponent(file.getName()));

	// Copy
	if (::CopyFileW(file.getFilesystemPath().getString().getOSString(),
			destinationFile.getFilesystemPath().getString().getOSString(), FALSE))
		// Success
		return OV<SError>();
	else
		// Error
		CFilesystemReportFilesErrorAndReturnError(SErrorFromWindowsGetLastError(), CString(OSSTR("copying file")),
				file, destinationFile);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFilesystem::replace(const CFile& sourceFile, const CFile& destinationFile)
//----------------------------------------------------------------------------------------------------------------------
{
	// Replace
	if (::ReplaceFileW(destinationFile.getFilesystemPath().getString().getOSString(),
			sourceFile.getFilesystemPath().getString().getOSString(), NULL, REPLACEFILE_IGNORE_MERGE_ERRORS, NULL,
			NULL))
		// Success
		return OV<SError>();
	else
		// Error
		CFilesystemReportFilesErrorAndReturnError(SErrorFromWindowsGetLastError(), CString(OSSTR("replacing file")),
				sourceFile, destinationFile);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFilesystem::createHardLink(const CFile& file, const CFolder& destinationFolder)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFile	destinationFile(destinationFolder.getFilesystemPath().appendingComponent(file.getName()));

	// Create hard link
	if (::CreateHardLinkW(destinationFile.getFilesystemPath().getString().getOSString(),
			file.getFilesystemPath().getString().getOSString(), NULL))
		// Success
		return OV<SError>();
	else
		// Error
		CFilesystemReportFilesErrorAndReturnError(SErrorFromWindowsGetLastError(),
				CString(OSSTR("creating hard link")), file, destinationFile);
}

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFilesystem::revealInFileExplorer(const TArray<CFilesystemPath>& filesystemPaths)
//----------------------------------------------------------------------------------------------------------------------
{
	// Group items by folder - each folder gets one File Explorer window with its items selected
	TNArrayDictionary<CFilesystemPath>	filesystemPathsByFolderPath;
	for (TArray<CFilesystemPath>::Iterator iterator = filesystemPaths.getIterator(); iterator; iterator++)
		// Add item
		filesystemPathsByFolderPath.add(iterator->deletingLastComponent().getString(), *iterator);

	// Iterate folders
	OV<SError>	error;
	for (TNArrayDictionary<CFilesystemPath>::Iterator iterator = filesystemPathsByFolderPath.getIterator(); iterator;
			iterator++) {
		// Setup
		const	CString&					folderPath = iterator.getKey();
		const	TNArray<CFilesystemPath>&	folderFilesystemPaths = iterator.getValue();
				PIDLIST_ABSOLUTE			folderPIDL = ::ILCreateFromPathW(folderPath.getOSString());
				PIDLIST_ABSOLUTE*			filePIDLs = new PIDLIST_ABSOLUTE[folderFilesystemPaths.getCount()];
				PCUITEMID_CHILD*			childPIDLs = new PCUITEMID_CHILD[folderFilesystemPaths.getCount()];
				UINT						count = 0;

		// Collect the items that can still be found
		if (folderPIDL != NULL)
			for (TArray<CFilesystemPath>::Iterator filesystemPathIterator = folderFilesystemPaths.getIterator();
					filesystemPathIterator; filesystemPathIterator++) {
				// Get PIDL
				PIDLIST_ABSOLUTE	filePIDL = ::ILCreateFromPathW(filesystemPathIterator->getString().getOSString());
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
