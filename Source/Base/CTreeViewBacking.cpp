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
// MARK: - CTreeViewBacking::Internals

class CTreeViewBacking::Internals {
	public:
		class Item;

	public:
		struct InternalInfo {
			// Types
			typedef	void	(*RemoveViewItemIDsProc)(const TArray<CString>& viewItemIDs, void* userData);
			typedef	void	(*NoteItemsProc)(const TArray<Item>& items, void* userData);

					// Lifecycle methods
					InternalInfo(RemoveViewItemIDsProc removeViewItemIDsProc,
							NoteItemsProc noteItemsProc, void* userData) :
						mRemoveViewItemIDsProc(removeViewItemIDsProc),
								mNoteItemsProc(noteItemsProc), mUserData(userData)
						{}

					// Instance methods
			void	removeViewItemIDs(const TArray<CString>& viewItemIDs) const
						{ mRemoveViewItemIDsProc(viewItemIDs, mUserData); }
			void	noteItems(const TArray<Item>& items) const
						{ mNoteItemsProc(items, mUserData); }

			// Properties
			private:
				RemoveViewItemIDsProc	mRemoveViewItemIDsProc;
				NoteItemsProc			mNoteItemsProc;
				void*					mUserData;
		};

	public:
		class Item {
			public:
						Item(const I<CTreeItem>& treeItem, const CString& viewItemID,
								const CTreeViewBacking::Info& info, const InternalInfo& internalInfo) :
							 mTreeItem(treeItem), mViewItemID(viewItemID), mInfo(info), mInternalInfo(internalInfo),
									mNeedsReload(true), mReloadInProgress(false)
							{}
						Item(const I<CTreeItem>& treeItem, const CTreeViewBacking::Info& info,
								const InternalInfo& internalInfo) :
							 mTreeItem(treeItem), mViewItemID(CUUID().getBase64String()), mInfo(info),
									mInternalInfo(internalInfo),
									mNeedsReload(true), mReloadInProgress(false)
							{}
						Item(const Item& other) :
							mTreeItem(other.mTreeItem), mViewItemID(other.mViewItemID), mInfo(other.mInfo),
									mInternalInfo(other.mInternalInfo),
									mChildViewItemIDs(other.mChildViewItemIDs), mNeedsReload(other.mNeedsReload),
									mReloadInProgress(other.mReloadInProgress)
							{}

				void	reloadChildItems()
							{
								// Check if needs reload
								if (!mNeedsReload || mReloadInProgress)
									// Punt
									return;

								// Setup
								mReloadInProgress = true;

								// Remove existing items
								mInternalInfo.removeViewItemIDs(mChildViewItemIDs);
								mChildViewItemIDs.removeAll();

								// Check how to get child items
								if (mInfo.canGetChildTreeItemsSync()) {
									// Get child tree items
									TMArray<I<CTreeItem> >	childTreeItems = mInfo.getChildTreeItems(mTreeItem);

									// Iterate child tree items
									TNArray<Item>	items;
									for (TIteratorD<I<CTreeItem> > iterator = childTreeItems.getIterator();
											iterator.hasValue(); iterator.advance()) {
										// Create tree view backing item
										Item	item(*iterator, mInfo, mInternalInfo);

										// Update arrays
										items += item;
										mChildViewItemIDs += item.mViewItemID;
									}

									// Note
									mInternalInfo.noteItems(items);

									// Done
									mNeedsReload = false;
									mReloadInProgress = false;
								} else {
									// Load child tree items
								}
							}

						I<CTreeItem>			mTreeItem;
						CString					mViewItemID;
				const	CTreeViewBacking::Info&	mInfo;
				const	InternalInfo&			mInternalInfo;

						TNArray<CString>		mChildViewItemIDs;
						bool					mNeedsReload;
						bool					mReloadInProgress;
		};

	public:
						Internals(const CTreeViewBacking::Info& info) :
							mInfo(info),
									mInternalInfo(
											(InternalInfo::RemoveViewItemIDsProc) removeViewItemIDs,
											(InternalInfo::NoteItemsProc) noteItems, this)
							{}

		static	void	removeViewItemIDs(const TArray<CString>& viewItemIDs, Internals* internals)
							{ internals->mItemByViewItemID.remove(viewItemIDs); }
		static	void	noteItems(const TArray<Item>& item, Internals* internals)
							{
								// Iterate items
								for (TIteratorD<Item> iterator = item.getIterator(); iterator.hasValue();
										iterator.advance())
									// Update map
									internals->mItemByViewItemID.set(iterator->mViewItemID, *iterator);
							}

		CTreeViewBacking::Info	mInfo;
		InternalInfo			mInternalInfo;

		TNDictionary<Item>		mItemByViewItemID;
		TNArray<CString>		mTopLevelViewItemIDs;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CTreeViewBacking

// MARK: Properties

 const	CString	CTreeViewBacking::mRootViewItemID(OSSTR("ROOT"));

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CTreeViewBacking::CTreeViewBacking(const Info& info)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(info);
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
	mInternals->mItemByViewItemID.removeAll();
	mInternals->mItemByViewItemID.set(mRootViewItemID,
			Internals::Item(rootTreeItem, mRootViewItemID, mInternals->mInfo, mInternals->mInternalInfo));

	mInternals->mTopLevelViewItemIDs.removeAll();
}

//----------------------------------------------------------------------------------------------------------------------
void CTreeViewBacking::set(const TArray<I<CTreeItem> >& topLevelTreeItems)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals->mItemByViewItemID.removeAll();

	// Add
	add(topLevelTreeItems);
}

//----------------------------------------------------------------------------------------------------------------------
void CTreeViewBacking::add(const TArray<I<CTreeItem> >& topLevelTreeItems)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate items
	for (TIteratorD<I<CTreeItem> > iterator = topLevelTreeItems.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Setup
		Internals::Item	item(*iterator, mInternals->mInfo, mInternals->mInternalInfo);

		// Store
		mInternals->mItemByViewItemID.set(item.mViewItemID, item);
		mInternals->mTopLevelViewItemIDs += item.mViewItemID;
	}
}

//----------------------------------------------------------------------------------------------------------------------
TArray<I<CTreeItem> > CTreeViewBacking::getTopLevelTreeItems() const
//----------------------------------------------------------------------------------------------------------------------
{
	return getTreeItems(mInternals->mTopLevelViewItemIDs);
}

//----------------------------------------------------------------------------------------------------------------------
const I<CTreeItem>& CTreeViewBacking::getTreeItem(const CString& viewItemID) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mItemByViewItemID[viewItemID]->mTreeItem;
}

//----------------------------------------------------------------------------------------------------------------------
TArray<I<CTreeItem> > CTreeViewBacking::getTreeItems(const TArray<CString>& viewItemIDs) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Collect tree items
	TNArray<I<CTreeItem> >	treeItems;
	for (TIteratorD<CString> iterator = viewItemIDs.getIterator(); iterator.hasValue(); iterator.advance())
		// Add tree item
		treeItems += mInternals->mItemByViewItemID[*iterator]->mTreeItem;

	return treeItems;
}

//----------------------------------------------------------------------------------------------------------------------
bool CTreeViewBacking::hasChildren(const CString& viewItemID) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	Internals::Item& item = *mInternals->mItemByViewItemID[viewItemID];

	// Check if have proc
	if (!mInternals->mInfo.canGetChildTreeItemsSync())
		// Query proc
		return item.mTreeItem->hasChildren();
	else {
		// Reload
		item.reloadChildItems();

		return !item.mChildViewItemIDs.isEmpty();
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CTreeViewBacking::getChildCount(const CString& viewItemID) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	OR<Internals::Item>	item = mInternals->mItemByViewItemID[viewItemID];

	// Check situation
	if ((viewItemID == mRootViewItemID) && !item.hasReference())
		// Requesting root item, but no root item
		return mInternals->mTopLevelViewItemIDs.getCount();
	else {
		// Reload
		item->reloadChildItems();

		return item->mChildViewItemIDs.getCount();
	}
}

//----------------------------------------------------------------------------------------------------------------------
CString CTreeViewBacking::getChildViewItemID(const CString& viewItemID, UInt32 index) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	OR<Internals::Item>	item = mInternals->mItemByViewItemID[viewItemID];

	// Check situation
	if ((viewItemID == mRootViewItemID) && !item.hasReference())
		// Requesting child of root item, but no root item
		return mInternals->mItemByViewItemID[mInternals->mTopLevelViewItemIDs[index]]->mViewItemID;
	else {
		// Reload
		item->reloadChildItems();

		return item->mChildViewItemIDs[index];
	}
}
