//----------------------------------------------------------------------------------------------------------------------
//	CppToolboxAssert.h			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "PlatformDefinitions.h"

class CString;
struct SError;

//----------------------------------------------------------------------------------------------------------------------
// MARK: Procs

extern	void	eAssertHandleProc(const SError& error, const char* file, UInt32 fileByteCount, const char* func,
						UInt32 funcByteCount, UInt32 line);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Errors

extern	SError	AssertFailedError;
extern	SError	AssertNilValueError;
extern	SError	AsserNonNilValueError;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Macros

#if defined(DEBUG)
	#define AssertFail()				AssertFailWith(AssertFailedError);
	#define AssertFailUnimplemented()	AssertFailWith(SError::mUnimplemented)
	#define	AssertFailWith(error)		{ eAssertHandleProc(error, __FILE__, sizeof(__FILE__), __func__,	\
												sizeof(__func__), __LINE__); }
	#define	AssertFailIf(cond)			{ if (cond) AssertFailWith(AssertFailedError); }
	#define	AssertNotNil(value)			{ if ((value) == nil) AssertFailWith(AssertNilValueError); }
	#define	AssertNil(value)			{ if ((value) != nil) AssertFailWith(AsserNonNilValueError); }
#else
	#define AssertFail()				{}
	#define AssertFailUnimplemented()	{}
	#define	AssertFailWith(error)		{}
	#define	AssertFailIf(cond)			{}
	#define	AssertNotNil(value)			{}
	#define	AssertNil(value)			{}
#endif
