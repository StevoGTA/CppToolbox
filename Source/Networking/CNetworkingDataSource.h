//----------------------------------------------------------------------------------------------------------------------
//	CNetworkingDataSource.h			©2023 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataSource.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CRemoteRandomAccessDataSource

class CRemoteRandomAccessDataSource : public CRandomAccessDataSource {
	// Methods
	public:
								// Lifecycle methods
								CRemoteRandomAccessDataSource() : CRandomAccessDataSource() {}

								// CDataSource methods
				TVResult<CData>	readData();

								// Instance methods

	// Properties
};
