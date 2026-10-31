//----------------------------------------------------------------------------------------------------------------------
//	SHMAC-Windows-Win32.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SHMAC.h"

#include <Windows.h>
#include <bcrypt.h>

#pragma comment(lib, "bcrypt.lib")

//----------------------------------------------------------------------------------------------------------------------
// MARK: SHMAC

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CData SHMAC::sha256(const CData& data, const CData& key)
//----------------------------------------------------------------------------------------------------------------------
{
	// Compute
	UInt8	hmac[32];
	::BCryptHash(BCRYPT_HMAC_SHA256_ALG_HANDLE, (PUCHAR) *key.getUInt8Buffer(), (ULONG) key.getByteCount(),
			(PUCHAR) *data.getUInt8Buffer(), (ULONG) data.getByteCount(), hmac, sizeof(hmac));

	return CData(hmac, sizeof(hmac));
}
