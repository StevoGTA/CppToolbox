//----------------------------------------------------------------------------------------------------------------------
//	CFile-Windows-WinRT.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFile.h"

#include "SError-Windows-WinRT.h"

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.FileProperties.h>

#include <Windows.h>

using namespace winrt::Windows::Storage;

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	CFileReportError(error, message)													\
				{																			\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);	\
					logAsError(CString::mSpaceX4);											\
				}
#define	CFileReportErrorAndReturnError(error, message)										\
				{																			\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);	\
					logAsError(CString::mSpaceX4);											\
																							\
					return OV<SError>(error);												\
				}
#define	CFileReportErrorAndReturnValue(error, message, value)								\
				{																			\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);	\
					logAsError(CString::mSpaceX4);											\
																							\
					return OV<SError>(value);												\
				}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFile

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFile::rename(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CFile::getByteCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Catch errors
	try {
		// Setup
		auto	storageFile = StorageFile::GetFileFromPathAsync(getFilesystemPath().getString().getOSString()).get();
		auto	basicProperties = storageFile.GetBasicPropertiesAsync().get();

		return basicProperties.Size();
	} catch (const hresult_error& exception) {
		// Error
		SError	error = SErrorFromHRESULTError(exception);
		CFileReportError(error, "getting byte count");

		return 0;
	}
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFile::remove() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Catch errors
	try {
		// Setup
		auto	storageFile = StorageFile::GetFileFromPathAsync(getFilesystemPath().getString().getOSString()).get();
		storageFile.DeleteAsync().get();

		return OV<SError>();
	} catch (const hresult_error& exception) {
		// Error
		SError	error = SErrorFromHRESULTError(exception);
		CFileReportError(error, "checking if exsists");

		return OV<SError>(error);
	}
}

//----------------------------------------------------------------------------------------------------------------------
bool CFile::doesExist() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Catch errors
	try {
		// Setup
		auto	storageFile = StorageFile::GetFileFromPathAsync(getFilesystemPath().getString().getOSString()).get();

		return true;
	} catch (const hresult_error& exception) {
		// Check error
		if ((exception.code() == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) || (exception.code() == E_INVALIDARG))
			// File not found
			return false;
		else {
			// Error
			SError	error = SErrorFromHRESULTError(exception);
			CFileReportError(error, "checking if exsists");

			return false;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
bool CFile::isHidden() const
//----------------------------------------------------------------------------------------------------------------------
{
	return getName().hasPrefix(CString(OSSTR(".")));
}

//----------------------------------------------------------------------------------------------------------------------
bool CFile::getLocked() const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return false;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFile::setLocked(bool lockFile) const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return OV<SError>();
}
