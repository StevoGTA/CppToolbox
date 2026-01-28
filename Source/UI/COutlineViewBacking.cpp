//----------------------------------------------------------------------------------------------------------------------
//	COutlineViewBacking.cpp			©2025 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "COutlineViewBacking.h"

#include "CDictionary.h"
#include "CUUID.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: COutlineViewBacking::Internals

class COutlineViewBacking::Internals {
	public:
//		class Item {
//			public:
//				Item(const I<COutlineViewItem>& outlineViewItem) :
//					 mOutlineViewItem(outlineViewItem),
//							mNeedsReload(true), mReloadInProgress(false)
//					{}
//				Item(const Item& other) :
//					mOutlineViewItem(other.mOutlineViewItem),
//							mChildItemIDs(other.mChildItemIDs), mNeedsReload(other.mNeedsReload),
//							mReloadInProgress(other.mReloadInProgress)
//					{}
//
//				I<COutlineViewItem>	mOutlineViewItem;
//
//				TNArray<CString>	mChildItemIDs;
//				bool				mNeedsReload;
//				bool				mReloadInProgress;
//		};

		Internals()
			{
			}

//		TNDictionary<Item>			mItemByItemID;
//		TNArray<CString>			mTopLevelItemIDs;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - COutlineViewBacking

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
COutlineViewBacking::COutlineViewBacking(const TArray<I<CTableViewBacking::Column> >& tableViewBackingColumns) :
		CTableViewBacking(tableViewBackingColumns)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals();
}

//----------------------------------------------------------------------------------------------------------------------
COutlineViewBacking::~COutlineViewBacking()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

////----------------------------------------------------------------------------------------------------------------------
//const TArray<CString>& COutlineViewBacking::getTopLevelItemIDs() const
////----------------------------------------------------------------------------------------------------------------------
//{
//	return mInternals->mTopLevelItemIDs;
//}

////----------------------------------------------------------------------------------------------------------------------
//UInt32 COutlineViewBacking::getChildCount(const OV<CString>& itemID) const
////----------------------------------------------------------------------------------------------------------------------
//{
//	return !itemID.hasValue() ?
//			mInternals->mTopLevelItemIDs.getCount() : mInternals->mItemByItemID[*itemID]->mChildItemIDs.getCount();
//}

////----------------------------------------------------------------------------------------------------------------------
//CString COutlineViewBacking::getChildItemID(const OV<CString>& itemID, UInt32 index) const
////----------------------------------------------------------------------------------------------------------------------
//{
//	return !itemID.hasValue() ?
//			mInternals->mTopLevelItemIDs[index] : mInternals->mItemByItemID[*itemID]->mChildItemIDs[index];
//}

////----------------------------------------------------------------------------------------------------------------------
//const I<COutlineViewItem>& COutlineViewBacking::getOutlineViewItem(const CString& itemID) const
////----------------------------------------------------------------------------------------------------------------------
//{
//	return mInternals->mItemByItemID[itemID]->mOutlineViewItem;
//}

////----------------------------------------------------------------------------------------------------------------------
//TArray<I<COutlineViewItem> > COutlineViewBacking::getOutlineViewItems(const TArray<CString>& itemIDs) const
////----------------------------------------------------------------------------------------------------------------------
//{
//	// Collect outline view items
//	TNArray<I<COutlineViewItem> >	outlineViewItems;
//	for (TIteratorD<CString> iterator = itemIDs.getIterator(); iterator.hasValue(); iterator.advance())
//		// Add tree item
//		outlineViewItems += mInternals->mItemByItemID[*iterator]->mOutlineViewItem;
//
//	return outlineViewItems;
//}

////----------------------------------------------------------------------------------------------------------------------
//void COutlineViewBacking::set(const TArray<I<COutlineViewItem> >& outlineViewItems, const OV<CString>& parentItemID)
////----------------------------------------------------------------------------------------------------------------------
//{
//	// Check level
//	if (!parentItemID.hasValue()) {
//		// Top level
//		mInternals->mItemByItemID.removeAll();
//		mInternals->mTopLevelItemIDs.removeAll();
//	} else {
//		// Child
//		Internals::Item& item = *mInternals->mItemByItemID[*parentItemID];
//
//		for (TIteratorD<CString> iterator = item.mChildItemIDs.getIterator(); iterator.hasValue(); iterator.advance())
//			// Remove
//			mInternals->mItemByItemID.remove(*iterator);
//	}
//
//	// Add
//	add(outlineViewItems, parentItemID);
//}

////----------------------------------------------------------------------------------------------------------------------
//void COutlineViewBacking::add(const TArray<I<COutlineViewItem> >& outlineViewItems, const OV<CString>& parentItemID)
////----------------------------------------------------------------------------------------------------------------------
//{
//	// Check level
//	if (!parentItemID.hasValue()) {
//		// Top level
//		for (TIteratorD<I<COutlineViewItem> > iterator = outlineViewItems.getIterator(); iterator.hasValue();
//				iterator.advance()) {
//			// Add
//			mInternals->mItemByItemID.set((*iterator)->getID(), Internals::Item(*iterator));
//			mInternals->mTopLevelItemIDs += (*iterator)->getID();
//		}
//	} else {
//		// Child
//		Internals::Item& item = *mInternals->mItemByItemID[*parentItemID];
//		for (TIteratorD<I<COutlineViewItem> > iterator = outlineViewItems.getIterator(); iterator.hasValue();
//				iterator.advance()) {
//			// Add
//			mInternals->mItemByItemID.set((*iterator)->getID(), Internals::Item(*iterator));
//			item.mChildItemIDs += (*iterator)->getID();
//		}
//	}
//}
