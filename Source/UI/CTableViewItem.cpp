//----------------------------------------------------------------------------------------------------------------------
//	CTableViewItem.cpp			©2026 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CTableViewItem.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CTableViewItem

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TArray<CString> CTableViewItem::getIDs(const TArray<I<CTableViewItem> >& tableViewItems)
//----------------------------------------------------------------------------------------------------------------------
{
	// Collect IDs
	TNArray<CString>	ids;
	for (TArray<I<CTableViewItem> >::Iterator iterator = tableViewItems.getIterator(); iterator; iterator++)
		// Add id
		ids += (*iterator)->getID();

	return ids;
}
