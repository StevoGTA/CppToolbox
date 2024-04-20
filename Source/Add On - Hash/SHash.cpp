//----------------------------------------------------------------------------------------------------------------------
//	SHash.cpp			©2024 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SHash.h"

#include "CString.h"
#include "xxhash.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SHash

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CString SHash::valueFor(const CData& data, Type type, bool uppercase)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check type
	switch (type) {
		case xxHash32: {
			// xxHash32
			XXH32_hash_t	hash = XXH32(data.getBytePtr(), data.getByteCount(), 0);

			return CData(&hash, sizeof(XXH32_hash_t), false).getHexString(uppercase);
		}

		case xxHash64: {
			// xxHash64
			XXH64_hash_t	hash = XXH64(data.getBytePtr(), data.getByteCount(), 0);

			return CData(&hash, sizeof(XXH64_hash_t), false).getHexString(uppercase);
		}

		case xxHash3_64: {
			// xxHash3 64 bits
			XXH64_hash_t	hash = XXH3_64bits(data.getBytePtr(), data.getByteCount());

			return CData(&hash, sizeof(XXH64_hash_t), false).getHexString(uppercase);
		}

		case xxHash3_128: {
			// xxHash3 128 bits
			XXH128_hash_t	hash = XXH3_128bits(data.getBytePtr(), data.getByteCount());

			return CData(&hash, sizeof(XXH128_hash_t), false).getHexString(uppercase);
		}

#if defined(TARGET_OS_WINDOWS)
		default:	return CString::mEmpty;
#endif
	}
}
