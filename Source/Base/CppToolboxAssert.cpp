//----------------------------------------------------------------------------------------------------------------------
//	CppToolboxAssert.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CppToolboxAssert.h"

#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Assert Errors

SError	AssertFailedError(CString(OSSTR("Assert")), 1, CString(OSSTR("Assert Failed")));
SError	AssertNilValueError(CString(OSSTR("Assert")), 2, CString(OSSTR("Unexpected nil value")));
SError	AsserNonNilValueError(CString(OSSTR("Assert")), 2, CString(OSSTR("Unexpected non-nil value")));
