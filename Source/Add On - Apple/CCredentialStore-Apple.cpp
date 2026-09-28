//----------------------------------------------------------------------------------------------------------------------
//	CCredentialStore-Apple.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CCredentialStore.h"

#include "CCoreFoundation.h"
#include "SError-Apple.h"

#include <Security/Security.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCredentialStore::Internals

class CCredentialStore::Internals {
	public:
					Internals(const CString& service) : mService(service) {}

		CDictionary	getQuery(const CString& key) const
						{
							// Generic password item in the data protection keychain, so access is scoped by
							//	signing identity rather than by user prompt
							CDictionary	query;
							query.set(CString(kSecClass), CString(kSecClassGenericPassword));
							query.set(CString(kSecAttrService), mService);
							query.set(CString(kSecAttrAccount), key);
							query.set(CString(kSecUseDataProtectionKeychain), true);

							return query;
						}

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
	// Setup
	CDictionary	query = mInternals->getQuery(key);
	query.set(CString(kSecReturnData), true);
	query.set(CString(kSecMatchLimit), CString(kSecMatchLimitOne));

	// Query
	CFTypeRef	result = nil;
	OSStatus	status = ::SecItemCopyMatching(*CCoreFoundation::dictionaryRefFrom(query), &result);
	if (status != errSecSuccess)
		// Not found
		return OV<CString>();

	// Compose string
	CString	string(CCoreFoundation::dataFrom((CFDataRef) result), CString::kEncodingUTF8);
	::CFRelease(result);

	return OV<CString>(string);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CCredentialStore::set(const CString& key, const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CDictionary	query = mInternals->getQuery(key);
	CDictionary	attributes;
	attributes.set(CString(kSecValueData), string.getUTF8Data());

	// Update existing item, else add
	OSStatus	status =
						::SecItemUpdate(*CCoreFoundation::dictionaryRefFrom(query),
								*CCoreFoundation::dictionaryRefFrom(attributes));
	if (status == errSecItemNotFound) {
		// Add
		query.set(CString(kSecValueData), string.getUTF8Data());
		status = ::SecItemAdd(*CCoreFoundation::dictionaryRefFrom(query), nil);
	}

	return (status == errSecSuccess) ? OV<SError>() : OV<SError>(SErrorFromOSStatus(status));
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CCredentialStore::remove(const CString& key)
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove
	OSStatus	status = ::SecItemDelete(*CCoreFoundation::dictionaryRefFrom(mInternals->getQuery(key)));

	return (status == errSecSuccess) ? OV<SError>() : OV<SError>(SErrorFromOSStatus(status));
}
