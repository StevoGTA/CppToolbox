//----------------------------------------------------------------------------------------------------------------------
//	CTreeViewBacking.h			©2021 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"
#include "CTreeItem.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CTreeViewBacking

class CTreeViewBackingInternals;
class CTreeViewBacking {

	// Methods
	public:
								// Lifecycle methods
								CTreeViewBacking();
								~CTreeViewBacking();

								// Instance methods
				void			set(const I<CTreeItem>& rootTreeItem);

				void			set(const TArray<I<CTreeItem> >& topLevelTreeItems);
				void			add(const TArray<I<CTreeItem> >& topLevelTreeItems);

		const	I<CTreeItem>&	getTreeItem(const CString& viewItemID) const;

				UInt32			getChildCount(const CString& viewItemID) const;
				CString			getChildViewItemID(const CString& viewItemID, UInt32 index) const;

	// Properties
	public:
		static	CString						mRootViewItemID;

	private:
				CTreeViewBackingInternals*	mInternals;
};
