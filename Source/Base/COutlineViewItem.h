//----------------------------------------------------------------------------------------------------------------------
//	COutlineViewItem.h			©2025 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CUUID.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: COutlineViewItem

class COutlineViewItem {
	// Methods
	public:

									// Lifecycle methods
									COutlineViewItem() {}
		virtual						~COutlineViewItem() {}

									// Subclass methods
		virtual	const	CString&	getID() const = 0;
		virtual			OSType		getType() const = 0;
		virtual			bool		hasChildren() const
										{ return false; }
};
