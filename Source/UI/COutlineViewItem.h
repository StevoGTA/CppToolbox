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
		virtual	UInt32							getChildCount() const
													{ return 0; }

												// Class methods
		static	TArray<CString>					getIDs(const TArray<I<COutlineViewItem> >& outlineViewItems)
													{ return CTableViewItem::getIDs(
															(const TArray<I<CTableViewItem> >&) outlineViewItems); }
		static	TArray<I<COutlineViewItem> >	filtered(const TArray<I<COutlineViewItem> >& outlineViewItems,
														const TArray<CString> ids);
};
