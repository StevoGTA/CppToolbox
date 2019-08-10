//----------------------------------------------------------------------------------------------------------------------
//	CppToolboxAssert.h			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CppToolboxError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Procs

extern	void	eAssertHandleProc(UError error, const char* file, const char* proc, UInt32 line);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Macros

#define AssertFailUnimplemented()	AssertFailWith(kUnimplementedError)
#define	AssertFailWith(error)		{ eAssertHandleProc(error, __FILE__, __func__, __LINE__); }
#define	AssertFailIf(cond)			{ if (cond) AssertFailWith(kAssertFailedError); }
#define	AssertNotNil(value)			{ if ((value) == nil) AssertFailWith(kNilValueError); }
#define	AssertNil(value)			{ if ((value) != nil) AssertFailWith(kNonNilValueError); }
