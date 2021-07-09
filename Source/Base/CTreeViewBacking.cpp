//----------------------------------------------------------------------------------------------------------------------
//	CTreeViewBacking.cpp			©2021 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CTreeViewBacking.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CTreeViewBackingItem

class CTreeViewBackingItem {
	public:
		CTreeViewBackingItem(const I<CTreeItem>& treeItem) : mTreeItem(treeItem) {}
		CTreeViewBackingItem(const CTreeViewBackingItem& other) : mTreeItem(other.mTreeItem) {}

		I<CTreeItem>	mTreeItem;
};
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CTreeViewBackingInternals

class CTreeViewBackingInternals {
	public:
		CTreeViewBackingInternals() {}

		TNArray<CTreeViewBackingItem>	mTopLevelTreeViewBackingItems;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CTreeViewBacking

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CTreeViewBacking::CTreeViewBacking()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CTreeViewBackingInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CTreeViewBacking::~CTreeViewBacking()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CTreeViewBacking::set(const I<CTreeItem>& rootTreeItem)
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
void CTreeViewBacking::set(const TArray<I<CTreeItem> >& topLevelTreeItems)
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
void CTreeViewBacking::add(const TArray<I<CTreeItem> >& topLevelTreeItems)
//----------------------------------------------------------------------------------------------------------------------
{
// Temp functionality
for (TIteratorD<I<CTreeItem> > iterator = topLevelTreeItems.getIterator(); iterator.hasValue(); iterator.advance())
	// Add
	mInternals->mTopLevelTreeViewBackingItems += CTreeViewBackingItem(*iterator);
}
