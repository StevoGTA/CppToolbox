//----------------------------------------------------------------------------------------------------------------------
//	CUUID-Apple.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CUUID.h"

//----------------------------------------------------------------------------------------------------------------------
CUUID::Bytes eCreateUUIDBytes();
CUUID::Bytes eCreateUUIDBytes()
//----------------------------------------------------------------------------------------------------------------------
{
	// Create new UUID and get raw bytes
	CFUUIDRef	uuidRef = ::CFUUIDCreate(kCFAllocatorDefault);
	CFUUIDBytes	uuidBytes = ::CFUUIDGetUUIDBytes(uuidRef);
	::CFRelease(uuidRef);

	return *((CUUID::Bytes*) &uuidBytes);
}
