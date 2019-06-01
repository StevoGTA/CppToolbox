//----------------------------------------------------------------------------------------------------------------------
//	PlatformDefinitions.h	©2013 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------------------------------------------------
// Global
#include <ApplicationServices/ApplicationServices.h>
#include <CoreServices/CoreServices.h>

#ifdef __cplusplus
	#include <algorithm>
	#include <pthread.h>
#endif

#define TARGET_OS_MACOS	1
#define TARGET_UI_COCOA	1

#define	DEPRECATED	DEPRECATED_ATTRIBUTE
#define EXPORT __attribute__((visibility("default")))
#define force_inline __attribute__((always_inline))

#define CNUMBER_COREFOUNDATION
#define CSTRING_COREFOUNDATION
#define CURL_COREFOUNDATION

// For Ogg
#define __MACOSX__

//----------------------------------------------------------------------------------------------------------------------
#include <MacTypes.h>

//----------------------------------------------------------------------------------------------------------------------
#define	MAKE_OSTYPE(a,b,c,d)	((a << 24) | (b << 16) | (c << 8) | d)

//----------------------------------------------------------------------------------------------------------------------
// new/delete helpers
#define DisposeOf(x)		{ delete x; x = nil; }
#define DisposeOfArray(x)	{ delete [] x; x = nil; }

#if defined(__OBJC__)
	#include <AppKit/AppKit.h>
	#include <Foundation/Foundation.h>
#endif

//----------------------------------------------------------------------------------------------------------------------
// Strings
#define	OSString	CFStringRef
#define	OSSTR(s)	CFSTR(s)
