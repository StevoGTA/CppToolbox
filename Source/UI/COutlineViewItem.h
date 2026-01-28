//----------------------------------------------------------------------------------------------------------------------
//	COutlineViewItem.h			©2025 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CTableViewItem.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: COutlineViewItem

class COutlineViewItem : public CTableViewItem {
	// Methods
	public:
						// Lifecycle methods
						COutlineViewItem() : CTableViewItem() {}

						// Instance methods
		virtual	UInt32	getChildCount() const
							{ return 0; }
};
