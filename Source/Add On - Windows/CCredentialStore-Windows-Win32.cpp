//----------------------------------------------------------------------------------------------------------------------
//	CCredentialStore-Windows-Win32.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CCredentialStore.h"

#include "SError-Windows.h"

#undef Delete
#include <Windows.h>
#include <wincred.h>
#define Delete(x)	{ delete x; x = nil; }

#pragma comment(lib, "advapi32")

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCredentialStore::Internals

class CCredentialStore::Internals {
	public:
				Internals(const CString& service) : mService(service) {}

		CString	getTargetName(const CString& key) const
					{ return mService + CString(OSSTR("/")) + key; }

		CString	mService;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CCredentialStore

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CCredentialStore::CCredentialStore(const CString& service)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(service);
}

//----------------------------------------------------------------------------------------------------------------------
CCredentialStore::~CCredentialStore()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
OV<CString> CCredentialStore::getString(const CString& key) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	PCREDENTIAL	credential;
	if (!::CredRead(mInternals->getTargetName(key).getOSString(), CRED_TYPE_GENERIC, 0, &credential))
		// Not found
		return OV<CString>();

	// Compose string
	CString	string(credential->CredentialBlob, credential->CredentialBlobSize, CString::kEncodingUTF16LE);
	::CredFree(credential);

	return OV<CString>(string);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CCredentialStore::set(const CString& key, const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CString		targetName = mInternals->getTargetName(key);
	CData		data = *string.getData(CString::kEncodingUTF16LE);

	CREDENTIAL	credential = {0};
	credential.Type = CRED_TYPE_GENERIC;
	credential.TargetName = (LPTSTR) targetName.getOSString();
	credential.UserName = (LPTSTR) key.getOSString();
	credential.CredentialBlob = (LPBYTE) *data.getUInt8Buffer();
	credential.CredentialBlobSize = (DWORD) data.getByteCount();
	credential.Persist = CRED_PERSIST_LOCAL_MACHINE;

	// Write (replaces any existing credential with the same target name)
	return ::CredWrite(&credential, 0) ? OV<SError>() : OV<SError>(SErrorFromWindowsError(::GetLastError()));
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CCredentialStore::remove(const CString& key)
//----------------------------------------------------------------------------------------------------------------------
{
	// Delete
	return ::CredDelete(mInternals->getTargetName(key).getOSString(), CRED_TYPE_GENERIC, 0) ?
			OV<SError>() : OV<SError>(SErrorFromWindowsError(::GetLastError()));
}
