//----------------------------------------------------------------------------------------------------------------------
//	CUUID-Apple.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CUUID.h"

//----------------------------------------------------------------------------------------------------------------------
SUUIDBytes eCreateUUIDBytes();
SUUIDBytes eCreateUUIDBytes()
//----------------------------------------------------------------------------------------------------------------------
{
	// Create new UUID and get raw bytes
	CFUUIDRef	uuidRef = ::CFUUIDCreate(kCFAllocatorDefault);
	CFUUIDBytes	uuidBytes = ::CFUUIDGetUUIDBytes(uuidRef);
	::CFRelease(uuidRef);

	return *((SUUIDBytes*) &uuidBytes);
}
