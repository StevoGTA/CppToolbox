//----------------------------------------------------------------------------------------------------------------------
//	SError.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SError

// MARK: Properties

const	SError	SError::mCancelled(CString(OSSTR("SError")), 1, CString(OSSTR("Cancelled")));
const	SError	SError::mEndOfData(CString(OSSTR("SError")), 2, CString(OSSTR("End of Data")));
const	SError	SError::mUnimplemented(CString(OSSTR("SError")), 3, CString(OSSTR("Unimplemented")));
