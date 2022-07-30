//----------------------------------------------------------------------------------------------------------------------
//	CCodec.cpp			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CCodec.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCodec

// MARK: Properties

static	CString	sErrorDomain(OSSTR("CCodec"));

const	SError	CCodec::mErrorUnsupported(sErrorDomain, 1, CString(OSSTR("Unsupported codec")));
