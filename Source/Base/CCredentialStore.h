//----------------------------------------------------------------------------------------------------------------------
//	CCredentialStore.h			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCredentialStore

class CCredentialStore {
	// Classes
	private:
		class Internals;

	// Methods
	public:
					// Lifecycle methods
					CCredentialStore(const CString& service);
					~CCredentialStore();

					// Instance methods
		OV<CString>	getString(const CString& key) const;
		OV<SError>	set(const CString& key, const CString& string);
		OV<SError>	remove(const CString& key);

	// Properties
	private:
		Internals*	mInternals;
};
