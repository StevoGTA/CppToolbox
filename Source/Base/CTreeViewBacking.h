//----------------------------------------------------------------------------------------------------------------------
//	CTreeViewBacking.h			©2021 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CArray.h"
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
		void	set(const I<CTreeItem>& rootTreeItem);

		void	set(const TArray<I<CTreeItem> >& topLevelTreeItems);
		void	add(const TArray<I<CTreeItem> >& topLevelTreeItems);

	// Properties
	private:
		CTreeViewBackingInternals*	mInternals;
};
