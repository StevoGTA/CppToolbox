//----------------------------------------------------------------------------------------------------------------------
//	CDataSource.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDataSource.h"

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDataSourceSetup

class CDataSourceSetup {
	public:
		CDataSourceSetup()
			{
				CErrorRegistry::registerError(kDataProviderReadBeyondEndError,
						CString(OSSTR("Read beyond end")));
				CErrorRegistry::registerError(kDataProviderSetPosBeforeStartError,
						CString(OSSTR("Set position before start")));
				CErrorRegistry::registerError(kDataProviderSetPosAfterEndError,
						CString(OSSTR("Set position after end")));
			}
};

static	CDataSourceSetup	sDataSourceSetup;
