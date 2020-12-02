//----------------------------------------------------------------------------------------------------------------------
//	SError.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SError

// MARK: Properties

SError	SError::mUnimplemented(CString(OSSTR("SError")), 1, CString(OSSTR("Unimplemented")));
SError	SError::mEndOfData(CString(OSSTR("SError")), 2, CString(OSSTR("End of Data")));
