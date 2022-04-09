//----------------------------------------------------------------------------------------------------------------------
//	CFilesystem-Windows-WinRT.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFilesystem.h"

#include "SError-Windows-WinRT.h"

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.h>

using namespace winrt::Windows::Storage;

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
	// Catch errors
	try {
		// Setup
		auto	storageFolder =
						StorageFolder::GetFolderFromPathAsync(folder.getFilesystemPath().getString().getOSString())
								.get();

		// Compose files
		TNArray<CFile>	files;

		// Check deep
		if (deep) {
			// Iterate folders
			for (auto const& childStorageFolder : storageFolder.GetFoldersAsync().get()) {
				// Get files for this folder
				auto	result = getFiles(CFolder(CFilesystemPath(CString(childStorageFolder.Path().data()))));
				if (result.hasValue())
					// Success
					files += *result;
				else
					// Error
					return TIResult<TArray<CFile> >(result.getError());
			}
		}

		// Iterate files
		for (auto const& storageFile : storageFolder.GetFilesAsync().get())
			// Add file
			files += CFile(CFilesystemPath(storageFile.Path().data()));

		return TIResult<TArray<CFile> >(files);
	} catch (const hresult_error& exception) {
		// Error
		SError	error = SErrorFromHRESULTError(exception);
		CFilesystemReportErrorFileFolderX1(error, "getting files", folder);

		return TIResult<TArray<CFile>>(error);
	}
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
