//----------------------------------------------------------------------------------------------------------------------
//	CDataSource.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDataSource.h"

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDataSource

// MARK: Properties

SError	CDataSource::mSetPosBeforeStartError(CString(OSSTR("CDataSource")), 1,
				CString(OSSTR("Data source set position before start")));
SError	CDataSource::mSetPosAfterEndError(CString(OSSTR("CDataSource")), 2,
				CString(OSSTR("Data source set position after end")));

