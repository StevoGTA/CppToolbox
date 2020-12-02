//----------------------------------------------------------------------------------------------------------------------
//	CDataSource.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDataSource.h"

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sErrorDomain(OSSTR("CDataSource"));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDataSource

// MARK: Properties

SError	CDataSource::mSetPosBeforeStartError(sErrorDomain, 1, CString(OSSTR("Data source set position before start")));
SError	CDataSource::mSetPosAfterEndError(sErrorDomain, 2, CString(OSSTR("Data source set position after end")));

