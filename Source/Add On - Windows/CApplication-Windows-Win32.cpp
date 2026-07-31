//----------------------------------------------------------------------------------------------------------------------
//	CApplication-Windows-Win32.cpp			©2025 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CApplication.h"

#include "SVersionInfo.h"
#include "SError-Windows.h"

#include <appmodel.h>
#include <shellapi.h>

#pragma comment(lib, "shell32")
#pragma comment(lib, "version")

//----------------------------------------------------------------------------------------------------------------------
// MARK: CApplication::Internals

class CApplication::Internals {
	public:
						Internals(const CFilesystemPath& filesystemPath, const CString& displayName,
								const CString& version, const OV<CString>& copyright) :
							mFilesystemPath(filesystemPath), mDisplayName(displayName), mVersion(version),
									mCopyright(copyright)
							{}

		static	CString	getVersionResourceString(const CString& filesystemPathString, const wchar_t* key)
							{
								// Get version info size
								DWORD	versionInfoHandle = 0;
								DWORD	versionInfoSize =
												::GetFileVersionInfoSizeW(filesystemPathString.getOSString(),
														&versionInfoHandle);
								if (versionInfoSize == 0)
									// No version info
									return CString::mEmpty;

								// Load version info
								TBuffer<UInt8>	versionInfo((UInt64) versionInfoSize);
								if (!::GetFileVersionInfoW(filesystemPathString.getOSString(), 0, versionInfoSize,
										*versionInfo))
									// Failed
									return CString::mEmpty;

								// Query the translation table so we read the value in the executable's own language
								struct LANGANDCODEPAGE { WORD wLanguage; WORD wCodePage; };
								LANGANDCODEPAGE*	translations = nil;
								UINT				translationsByteCount = 0;
								if (!::VerQueryValueW(*versionInfo, L"\\VarFileInfo\\Translation",
												(LPVOID*) &translations, &translationsByteCount) ||
										(translationsByteCount < sizeof(LANGANDCODEPAGE)))
									// No translation
									return CString::mEmpty;

								// Compose the sub-block path for the requested key using the first translation
								wchar_t	subBlock[128];
								::swprintf_s(subBlock, L"\\StringFileInfo\\%04x%04x\\%s", translations[0].wLanguage,
										translations[0].wCodePage, key);

								// Query the value
								LPWSTR	value = nil;
								UINT	valueLength = 0;
								if (::VerQueryValueW(*versionInfo, subBlock, (LPVOID*) &value, &valueLength) &&
										(valueLength > 1))
									// Found
									return CString(value);

								return CString::mEmpty;
							}

		CFilesystemPath	mFilesystemPath;
		CString			mDisplayName;
		CString			mVersion;
		OV<CString>		mCopyright;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CApplication

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CApplication::CApplication(const CFilesystemPath& filesystemPath, const CString& displayName, const CString& version,
		const OV<CString>& copyright)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(filesystemPath, displayName, version, copyright);
}

//----------------------------------------------------------------------------------------------------------------------
CApplication::CApplication(const CApplication& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals =
			new Internals(other.mInternals->mFilesystemPath, other.mInternals->mDisplayName, other.mInternals->mVersion,
					other.mInternals->mCopyright);
}

//----------------------------------------------------------------------------------------------------------------------
CApplication::~CApplication()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CString CApplication::getDisplayName() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mDisplayName;
}

//----------------------------------------------------------------------------------------------------------------------
CString CApplication::getVersion() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mVersion;
}

//----------------------------------------------------------------------------------------------------------------------
CString CApplication::getDisplayNameAndVersion() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mVersion.isEmpty() ?
			mInternals->mDisplayName : (mInternals->mDisplayName + CString::mSpace + mInternals->mVersion);
}

//----------------------------------------------------------------------------------------------------------------------
OV<CString> CApplication::getCopyright() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mCopyright;
}

//----------------------------------------------------------------------------------------------------------------------
CFolder CApplication::getFolder() const
//----------------------------------------------------------------------------------------------------------------------
{
	return CFolder(mInternals->mFilesystemPath.deletingLastComponent());
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CApplication::open(const TArray<CFile>& files) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate files - launch this application once per file (reliable for both single-file and multi-file apps)
	OV<SError>	error;
	for (TArray<CFile>::Iterator iterator = files.getIterator(); iterator; iterator++) {
		// Pass the file path as a quoted parameter so paths with spaces arrive as a single argument
		CString		parametersString =
							CString(OSSTR("\"")) + iterator->getFilesystemPath().getString() + CString(OSSTR("\""));
		HINSTANCE	result =
							::ShellExecuteW(NULL, NULL, mInternals->mFilesystemPath.getString().getOSString(),
									parametersString.getOSString(), NULL, SW_SHOWNORMAL);
		if (((INT_PTR) result) < 32)
			// Error
			error.setValue(SErrorFromWindowsGetLastError());
	}

	return error;
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> CApplication::getStorageData() const
//----------------------------------------------------------------------------------------------------------------------
{
	return TVResult<CData>(*mInternals->mFilesystemPath.getString().getData(CString::kEncodingUTF8));
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CApplication CApplication::getCurrent()
//----------------------------------------------------------------------------------------------------------------------
{
	// Get the running executable path
	wchar_t	modulePath[MAX_PATH];
	::GetModuleFileNameW(NULL, modulePath, MAX_PATH);

	CString			modulePathString(modulePath);
	CFilesystemPath	filesystemPath(modulePathString);

	// Compose display name from the executable filename
	OV<CString>	lastComponent = filesystemPath.getLastComponentDeletingExtension();
	CString		displayName = lastComponent.hasValue() ? *lastComponent : CString::mEmpty;

	// Compose version from the package identity (when running with package identity)
	CString	version;
	UINT32	packageIDByteCount = 0;
	if (::GetCurrentPackageId(&packageIDByteCount, nil) == ERROR_INSUFFICIENT_BUFFER) {
		// Get package id
		TBuffer<UInt8>	packageIDBuffer((UInt64) packageIDByteCount);
		if (::GetCurrentPackageId(&packageIDByteCount, *packageIDBuffer) == ERROR_SUCCESS) {
			// Compose version
			PACKAGE_ID*	packageID = (PACKAGE_ID*) *packageIDBuffer;
			version =
					SVersionInfo((UInt8) packageID->version.Major, (UInt8) packageID->version.Minor,
									(UInt8) packageID->version.Build)
							.getString();
		}
	}
	if (version.isEmpty())
		// Fall back to the executable's version resource when not running with package identity
		version = Internals::getVersionResourceString(modulePathString, L"ProductVersion");

	return CApplication(filesystemPath, displayName, version, OV<CString>());
}

//----------------------------------------------------------------------------------------------------------------------
OV<CApplication> CApplication::getFor(const CFilesystemPath& filesystemPath)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CString	filesystemPathString = filesystemPath.getString();

	// Compose display name from FileDescription, falling back to the filename
	CString		fileDescription = Internals::getVersionResourceString(filesystemPathString, L"FileDescription");
	OV<CString>	lastComponent = filesystemPath.getLastComponentDeletingExtension();
	CString		displayName =
						!fileDescription.isEmpty() ?
								fileDescription : (lastComponent.hasValue() ? *lastComponent : CString::mEmpty);

	// Compose version and copyright from the version resource
	CString	version = Internals::getVersionResourceString(filesystemPathString, L"ProductVersion");
	CString	copyright = Internals::getVersionResourceString(filesystemPathString, L"LegalCopyright");

	return OV<CApplication>(
			CApplication(filesystemPath, displayName, version,
					!copyright.isEmpty() ? OV<CString>(copyright) : OV<CString>()));
}

//----------------------------------------------------------------------------------------------------------------------
OV<CApplication> CApplication::getFrom(const CData& storageData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Reconstruct the executable path from UTF-8 storage data
	CString	filesystemPathString(storageData, CString::kEncodingUTF8);
	if (filesystemPathString.isEmpty())
		// No path
		return OV<CApplication>();

	// Check that the application is still present
	CFilesystemPath	filesystemPath(filesystemPathString);
	if (!CFile(filesystemPath).doesExist())
		// The application is no longer present
		return OV<CApplication>();

	return getFor(filesystemPath);
}
