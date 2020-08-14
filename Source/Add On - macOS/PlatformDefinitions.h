//----------------------------------------------------------------------------------------------------------------------
//	PlatformDefinitions.h	©2013 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------------------------------------------------
// Global
#include <CoreServices/CoreServices.h>

#ifdef __cplusplus
	#include <algorithm>
#endif

#define TARGET_OS_MACOS	1

#define	DEPRECATED	DEPRECATED_ATTRIBUTE
#define force_inline __attribute__((always_inline))

//----------------------------------------------------------------------------------------------------------------------
#define	MAKE_OSTYPE(a,b,c,d)	((a << 24) | (b << 16) | (c << 8) | d)

//----------------------------------------------------------------------------------------------------------------------
// Lifecycle helpers
#define Delete(x)		{ delete x; x = nil; }
#define DeleteArray(x)	{ delete [] x; x = nil; }
