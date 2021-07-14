//----------------------------------------------------------------------------------------------------------------------
//	CTreeViewBacking.cpp			©2021 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CTreeViewBacking.h"

#include "CDictionary.h"
#include "CUUID.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CRootTreeItem

class CRootTreeItem : public CTreeItem {};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CTreeViewBackingItem

class CTreeViewBackingItem {
	public:
				CTreeViewBackingItem(const I<CTreeItem>& treeItem) :
					mViewItemID(CUUID().getBase64String()), mTreeItem(treeItem)
					{}
				CTreeViewBackingItem(const CTreeViewBackingItem& other) :
					mViewItemID(other.mViewItemID), mTreeItem(other.mTreeItem),
							mChildViewItemIDs(other.mChildViewItemIDs)
					{}

		void	addChild(const CTreeViewBackingItem& treeViewBackingItem)
					{ mChildViewItemIDs += treeViewBackingItem.mViewItemID; }

		CString				mViewItemID;
		I<CTreeItem>		mTreeItem;
		TNArray<CString>	mChildViewItemIDs;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CTreeViewBackingInternals

class CTreeViewBackingInternals {
	public:
		CTreeViewBackingInternals() {}

		TNDictionary<CTreeViewBackingItem>	mTreeViewBackingItemMap;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CTreeViewBacking

// MARK: Properties

CString	CTreeViewBacking::mRootViewItemID(OSSTR("ROOT"));

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
	// Setup
	mInternals->mTreeViewBackingItemMap.removeAll();
	mInternals->mTreeViewBackingItemMap.set(mRootViewItemID, CTreeViewBackingItem(rootTreeItem));
}

//----------------------------------------------------------------------------------------------------------------------
void CTreeViewBacking::set(const TArray<I<CTreeItem> >& topLevelTreeItems)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals->mTreeViewBackingItemMap.removeAll();

	CTreeViewBackingItem	rootTreeViewBackingItem(I<CTreeItem>(new CRootTreeItem()));
	mInternals->mTreeViewBackingItemMap.set(mRootViewItemID, rootTreeViewBackingItem);

	// Iterate items
	for (TIteratorD<I<CTreeItem> > iterator = topLevelTreeItems.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Add
		CTreeViewBackingItem	treeViewBackingItem(*iterator);
		mInternals->mTreeViewBackingItemMap.set(treeViewBackingItem.mViewItemID, treeViewBackingItem);

		rootTreeViewBackingItem.addChild(treeViewBackingItem);
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CTreeViewBacking::add(const TArray<I<CTreeItem> >& topLevelTreeItems)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	OR<CTreeViewBackingItem>	rootTreeViewBackingItem = mInternals->mTreeViewBackingItemMap[mRootViewItemID];
	if (!rootTreeViewBackingItem.hasReference()) {
		// Setup root
		mInternals->mTreeViewBackingItemMap.set(mRootViewItemID,
				CTreeViewBackingItem(I<CTreeItem>(new CRootTreeItem())));
		rootTreeViewBackingItem = mInternals->mTreeViewBackingItemMap[mRootViewItemID];
	}

	// Iterate items
	for (TIteratorD<I<CTreeItem> > iterator = topLevelTreeItems.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Add
		CTreeViewBackingItem	treeViewBackingItem(*iterator);
		mInternals->mTreeViewBackingItemMap.set(treeViewBackingItem.mViewItemID, treeViewBackingItem);

		rootTreeViewBackingItem->addChild(treeViewBackingItem);
	}
}

//----------------------------------------------------------------------------------------------------------------------
const I<CTreeItem>& CTreeViewBacking::getTreeItem(const CString& viewItemID) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mTreeViewBackingItemMap[viewItemID]->mTreeItem;
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CTreeViewBacking::getChildCount(const CString& viewItemID) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	OR<CTreeViewBackingItem>	treeViewBackingItem = mInternals->mTreeViewBackingItemMap[viewItemID];

	return treeViewBackingItem.hasReference() ? treeViewBackingItem->mChildViewItemIDs.getCount() : 0;
}

//----------------------------------------------------------------------------------------------------------------------
CString CTreeViewBacking::getChildViewItemID(const CString& viewItemID, UInt32 index) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Return view item id
	return mInternals->mTreeViewBackingItemMap[viewItemID]->mChildViewItemIDs[index];
}
