//----------------------------------------------------------------------------------------------------------------------
//	PlatformDefinitions.h	©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#define _USE_MATH_DEFINES

#include <algorithm>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <memory.h>
#include <tchar.h>
#include <time.h>

//----------------------------------------------------------------------------------------------------------------------
// Defines
#define TARGET_OS_WINDOWS 1
#define TARGET_RT_LITTLE_ENDIAN 1

#define	nil	NULL

#define	MAKE_OSTYPE(a,b,c,d)	((a << 24) | (b << 16) | (c << 8) | d)

#define force_inline __forceinline
#define __attribute__(x)
#define _Nullable
#define __nonnull(x)

#define Endian16_Swap(value)	_byteswap_ushort(value)
#define Endian32_Swap(value)	_byteswap_ulong(value)
#define Endian64_Swap(value)	_byteswap_uint64(value)

#define EndianS16_BtoN(value)	((SInt16) _byteswap_ushort(value))
#define EndianS16_NtoB(value)	((SInt16) _byteswap_ushort(value))
#define EndianU16_BtoN(value)	((UInt16) _byteswap_ushort(value))
#define EndianU16_NtoB(value)	((UInt16) _byteswap_ushort(value))
#define EndianS32_BtoN(value)	((SInt32) _byteswap_ulong(value))
#define EndianS32_NtoB(value)	((SInt32) _byteswap_ulong(value))
#define EndianU32_BtoN(value)	((UInt32) _byteswap_ulong(value))
#define EndianU32_NtoB(value)	((UInt32) _byteswap_ulong(value))
#define EndianS64_BtoN(value)	((SInt64) _byteswap_uint64(value))
#define EndianS64_NtoB(value)	((SInt64) _byteswap_uint64(value))
#define EndianU64_BtoN(value)	((UInt64) _byteswap_uint64(value))
#define EndianU64_NtoB(value)	((UInt64) _byteswap_uint64(value))

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

//----------------------------------------------------------------------------------------------------------------------
// Types
typedef float		Float32;
typedef double		Float64;
typedef int8_t		SInt8;
typedef int16_t		SInt16;
typedef int32_t		SInt32;
typedef int64_t		SInt64;
typedef uint8_t		UInt8;
typedef uint16_t	UInt16;
typedef	uint32_t	UInt32;
typedef uint64_t	UInt64;

typedef	UInt32		OSType;

typedef	UInt16		UTF16Char;
typedef	UInt32		UTF32Char;

//----------------------------------------------------------------------------------------------------------------------
// Lifecycle helpers
#define Delete(x)		{ delete x; x = nil; }
#define DeleteArray(x)	{ delete [] x; x = nil; }
