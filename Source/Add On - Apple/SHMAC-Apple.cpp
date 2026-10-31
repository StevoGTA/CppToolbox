//----------------------------------------------------------------------------------------------------------------------
//	SHMAC-Apple.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SHMAC.h"

#include <CommonCrypto/CommonHMAC.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: SHMAC

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CData SHMAC::sha256(const CData& data, const CData& key)
//----------------------------------------------------------------------------------------------------------------------
{
	// Compute
	UInt8	hmac[CC_SHA256_DIGEST_LENGTH];
	::CCHmac(kCCHmacAlgSHA256, *key.getUInt8Buffer(), key.getByteCount(), *data.getUInt8Buffer(), data.getByteCount(),
			hmac);

	return CData(hmac, CC_SHA256_DIGEST_LENGTH);
}
