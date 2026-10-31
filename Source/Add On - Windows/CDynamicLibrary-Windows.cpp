//----------------------------------------------------------------------------------------------------------------------
//	CDynamicLibrary-Windows.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDynamicLibrary.h"

#include "CFileDataSource.h"
#include "CReferenceCountable.h"
#include "SError-Windows.h"

#pragma comment(lib, "Version.lib")

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDynamicLibrary::Internals

class CDynamicLibrary::Internals : public TReferenceCountableAutoDelete<Internals> {
	public:
									Internals(const CFile& file, CDynamicLibrary::Handle handle) :
										TReferenceCountableAutoDelete(),
												mFile(file), mHandle(handle)
										{}
									~Internals()
										{ ::FreeLibrary(mHandle); }

		static	OV<TBuffer<UInt8> >	getVersionInfo(const CFilesystemPath& filesystemPath)
										{
											// Get byte count
											DWORD	byteCount =
															::GetFileVersionInfoSize(
																	filesystemPath.getString().getOSString(),
																	NULL);
											if (byteCount == 0)
												// No version info
												return OV<TBuffer<UInt8> >();

											// Get version info
											TBuffer<UInt8>	buffer(byteCount);

											return ::GetFileVersionInfo(
															filesystemPath.getString().getOSString(), 0,
															byteCount, *buffer) ?
													OV<TBuffer<UInt8> >(buffer) : OV<TBuffer<UInt8> >();
										}

		CFile					mFile;
		CDynamicLibrary::Handle	mHandle;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDynamicLibrary

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CDynamicLibrary::CDynamicLibrary(const CDynamicLibrary& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CDynamicLibrary::CDynamicLibrary(const CFile& file, Handle handle)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(file, handle);
}

//----------------------------------------------------------------------------------------------------------------------
CDynamicLibrary::~CDynamicLibrary()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CFile& CDynamicLibrary::getFile() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFile;
}

//----------------------------------------------------------------------------------------------------------------------
OV<void*> CDynamicLibrary::getSymbolAddress(const CString& name) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Look up
	FARPROC	address = ::GetProcAddress(mInternals->mHandle, *name.getUTF8String());

	return (address != NULL) ? OV<void*>((void*) address) : OV<void*>();
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TVResult<CDynamicLibrary> CDynamicLibrary::load(const CFile& file)
//----------------------------------------------------------------------------------------------------------------------
{
	// Load, resolving the library's own dependencies from its folder first
	Handle	handle =
					::LoadLibraryEx(file.getFilesystemPath().getString().getOSString(), NULL,
							LOAD_WITH_ALTERED_SEARCH_PATH);
	if (handle == NULL)
		// Error
		return TVResult<CDynamicLibrary>(SErrorFromWindowsGetLastError());

	return TVResult<CDynamicLibrary>(CDynamicLibrary(file, handle));
}

//----------------------------------------------------------------------------------------------------------------------
OV<SVersionInfo> CDynamicLibrary::getVersionInfo(const CFilesystemPath& filesystemPath)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get version info
	OV<TBuffer<UInt8> >	buffer = Internals::getVersionInfo(filesystemPath);
	if (!buffer.hasValue())
		// No version info
		return OV<SVersionInfo>();

	// Get fixed file info
	VS_FIXEDFILEINFO*	fixedFileInfo;
	UINT				length;
	if (!::VerQueryValue(**buffer, TEXT("\\"), (LPVOID*) &fixedFileInfo, &length) || (length == 0))
		// No fixed file info
		return OV<SVersionInfo>();

	return OV<SVersionInfo>(
			SVersionInfo((UInt8) HIWORD(fixedFileInfo->dwFileVersionMS), (UInt8) LOWORD(fixedFileInfo->dwFileVersionMS),
					(UInt8) HIWORD(fixedFileInfo->dwFileVersionLS),
					CString((UInt32) HIWORD(fixedFileInfo->dwFileVersionMS)) + CString(OSSTR(".")) +
							CString((UInt32) LOWORD(fixedFileInfo->dwFileVersionMS)) + CString(OSSTR(".")) +
							CString((UInt32) HIWORD(fixedFileInfo->dwFileVersionLS)) + CString(OSSTR(".")) +
							CString((UInt32) LOWORD(fixedFileInfo->dwFileVersionLS))));
}

//----------------------------------------------------------------------------------------------------------------------
OV<CString> CDynamicLibrary::getManufacturer(const CFilesystemPath& filesystemPath)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get version info
	OV<TBuffer<UInt8> >	buffer = Internals::getVersionInfo(filesystemPath);
	if (!buffer.hasValue())
		// No version info
		return OV<CString>();

	// Get the first language and code page
	WORD*	translation;
	UINT	length;
	if (!::VerQueryValue(**buffer, TEXT("\\VarFileInfo\\Translation"), (LPVOID*) &translation, &length) ||
			(length < (2 * sizeof(WORD))))
		// No translation
		return OV<CString>();

	// Get company name
	LPCWSTR	companyName;
	if (!::VerQueryValue(**buffer,
					CString::make(OSSTR("\\StringFileInfo\\%04x%04x\\CompanyName"), translation[0], translation[1])
							.getOSString(),
					(LPVOID*) &companyName, &length) ||
			(length <= 1))
		// No company name
		return OV<CString>();

	return OV<CString>(CString(companyName));
}

//----------------------------------------------------------------------------------------------------------------------
CDynamicLibrary::Loadability CDynamicLibrary::getLoadability(const CFilesystemPath& filesystemPath)
//----------------------------------------------------------------------------------------------------------------------
{
	// Read the DOS and PE headers
	CFileDataSource	fileDataSource(CFile(filesystemPath), false);
	UInt16			dosSignature;
	UInt32			peHeaderOffset;
	UInt32			peSignature;
	UInt16			machine;
	if (fileDataSource.read(0, &dosSignature, sizeof(UInt16)).hasValue() || (dosSignature != IMAGE_DOS_SIGNATURE) ||
			fileDataSource.read(0x3C, &peHeaderOffset, sizeof(UInt32)).hasValue() ||
			fileDataSource.read(peHeaderOffset, &peSignature, sizeof(UInt32)).hasValue() ||
			(peSignature != IMAGE_NT_SIGNATURE) ||
			fileDataSource.read(peHeaderOffset + 4, &machine, sizeof(UInt16)).hasValue())
		// Not a library, or missing or damaged
		return Loadability(Loadability::kStateUnreadable);

	// Check for this process's architecture.  The PE header's OS version is not checked, as nearly every library
	//	declares an old one, and nothing here quarantines libraries.
#if defined(_M_ARM64)
	return (machine == IMAGE_FILE_MACHINE_ARM64) ?
			Loadability(Loadability::kStatePossible) : Loadability(Loadability::kStateWrongArchitecture);
#elif defined(_M_X64)
	return (machine == IMAGE_FILE_MACHINE_AMD64) ?
			Loadability(Loadability::kStatePossible) : Loadability(Loadability::kStateWrongArchitecture);
#elif defined(_M_IX86)
	return (machine == IMAGE_FILE_MACHINE_I386) ?
			Loadability(Loadability::kStatePossible) : Loadability(Loadability::kStateWrongArchitecture);
#endif
}
