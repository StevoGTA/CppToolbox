//----------------------------------------------------------------------------------------------------------------------
//	PlatformDefinitions.h	©2013 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------------------------------------------------
// Global
#ifdef __cplusplus
	#include <algorithm>
#endif

#include <CoreFoundation/CoreFoundation.h>

#undef TARGET_OS_TVOS
#undef TARGET_OS_WATCHOS
#undef TARGET_OS_WINDOWS

#define	DEPRECATED	DEPRECATED_ATTRIBUTE
#define force_inline __attribute__((always_inline))

//----------------------------------------------------------------------------------------------------------------------
#include <MacTypes.h>

//----------------------------------------------------------------------------------------------------------------------
#define	MAKE_OSTYPE(a,b,c,d)	((a << 24) | (b << 16) | (c << 8) | d)

//----------------------------------------------------------------------------------------------------------------------
// Lifecycle helpers
#define Delete(x)		{ delete x; x = nil; }
#define DeleteArray(x)	{ delete [] x; x = nil; }

//----------------------------------------------------------------------------------------------------------------------
// Byte swapping
#if !defined(EndianS16_BtoN)
	#define EndianS16_BtoN(value)	((SInt16) CFSwapInt16(value))
	#define EndianS16_NtoB(value)	((SInt16) CFSwapInt16(value))
	#define EndianU16_BtoN(value)	((UInt16) CFSwapInt16(value))
	#define EndianU16_NtoB(value)	((UInt16) CFSwapInt16(value))
	#define EndianS32_BtoN(value)	((SInt32) CFSwapInt32(value))
	#define EndianS32_NtoB(value)	((SInt32) CFSwapInt32(value))
	#define EndianU32_BtoN(value)	((UInt32) CFSwapInt32(value))
	#define EndianU32_NtoB(value)	((UInt32) CFSwapInt32(value))
	#define EndianS64_BtoN(value)	((SInt64) CFSwapInt64(value))
	#define EndianS64_NtoB(value)	((SInt64) CFSwapInt64(value))
	#define EndianU64_BtoN(value)	((UInt64) CFSwapInt64(value))
	#define EndianU64_NtoB(value)	((UInt64) CFSwapInt64(value))

	#define EndianS16_LtoN(value)	value
	#define EndianS16_NtoL(value)	value
	#define EndianU16_LtoN(value)	value
	#define EndianU16_NtoL(value)	value
	#define EndianS32_LtoN(value)	value
	#define EndianS32_NtoL(value)	value
	#define EndianU32_LtoN(value)	value
	#define EndianU32_NtoL(value)	value
	#define EndianS64_LtoN(value)	value
	#define EndianS64_NtoL(value)	value
	#define EndianU64_LtoN(value)	value
	#define EndianU64_NtoL(value)	value

	#define Endian16_Swap(value)	(UInt16) (__builtin_constant_p(value) ? OSSwapConstInt16(value) : OSSwapInt16(value))
	#define Endian32_Swap(value)	(UInt32) (__builtin_constant_p(value) ? OSSwapConstInt32(value) : OSSwapInt32(value))
	#define Endian64_Swap(value)	(UInt64) (__builtin_constant_p(value) ? OSSwapConstInt64(value) : OSSwapInt64(value))

#endif
