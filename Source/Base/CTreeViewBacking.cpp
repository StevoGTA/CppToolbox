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
// MARK: - STreeViewBackingInternalInfo

class CTreeViewBackingItem;
struct STreeViewBackingInternalInfo {
	// Types
	typedef	void	(*RemoveViewItemIDsProc)(const TArray<CString>& viewItemIDs, void* userData);
	typedef	void	(*NoteTreeViewBackingItemsProc)(const TArray<CTreeViewBackingItem>& treeViewBackingItems,
							void* userData);

			// Lifecycle methods
			STreeViewBackingInternalInfo(RemoveViewItemIDsProc removeViewItemIDsProc,
					NoteTreeViewBackingItemsProc noteTreeViewBackingItemsProc, void* userData) :
				mRemoveViewItemIDsProc(removeViewItemIDsProc),
						mNoteTreeViewBackingItemsProc(noteTreeViewBackingItemsProc), mUserData(userData)
				{}

			// Instance methods
	void	removeViewItemIDs(const TArray<CString>& viewItemIDs) const
				{ mRemoveViewItemIDsProc(viewItemIDs, mUserData); }
	void	noteTreeViewBackingItems(const TArray<CTreeViewBackingItem>& treeViewBackingItems) const
				{ mNoteTreeViewBackingItemsProc(treeViewBackingItems, mUserData); }

	// Properties
	private:
		RemoveViewItemIDsProc			mRemoveViewItemIDsProc;
		NoteTreeViewBackingItemsProc	mNoteTreeViewBackingItemsProc;
		void*							mUserData;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CTreeViewBackingItem

class CTreeViewBackingItem {
	public:
				CTreeViewBackingItem(const I<CTreeItem>& treeItem, const CString& viewItemID,
						const CTreeViewBacking::Info& info, const STreeViewBackingInternalInfo& internalInfo) :
					 mTreeItem(treeItem), mViewItemID(viewItemID), mInfo(info), mInternalInfo(internalInfo),
					 		mNeedsReload(true), mReloadInProgress(false)
					{}
				CTreeViewBackingItem(const I<CTreeItem>& treeItem, const CTreeViewBacking::Info& info,
						const STreeViewBackingInternalInfo& internalInfo) :
					 mTreeItem(treeItem), mViewItemID(CUUID().getBase64String()), mInfo(info),
					 		mInternalInfo(internalInfo),
					 		mNeedsReload(true), mReloadInProgress(false)
					{}
				CTreeViewBackingItem(const CTreeViewBackingItem& other) :
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
							if (mInfo.mCompareTreeItemsProc != nil)
								// Sort
								childTreeItems.sort(mInfo.mCompareTreeItemsProc);

							// Iterate child tree items
							TNArray<CTreeViewBackingItem>	treeViewBackingItems;
							for (TIteratorD<I<CTreeItem> > iterator = childTreeItems.getIterator(); iterator.hasValue();
									iterator.advance()) {
								// Create tree view backing item
								CTreeViewBackingItem	treeViewBackingItem(*iterator, mInfo, mInternalInfo);

								// Update arrays
								treeViewBackingItems += treeViewBackingItem;
								mChildViewItemIDs += treeViewBackingItem.mViewItemID;
							}

							// Note
							mInternalInfo.noteTreeViewBackingItems(treeViewBackingItems);

							// Done
							mNeedsReload = false;
							mReloadInProgress = false;
						} else {
							// Load child tree items
						}
					}

				I<CTreeItem>					mTreeItem;
				CString							mViewItemID;
		const	CTreeViewBacking::Info&			mInfo;
		const	STreeViewBackingInternalInfo&	mInternalInfo;

				TNArray<CString>				mChildViewItemIDs;
				bool							mNeedsReload;
				bool							mReloadInProgress;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CTreeViewBackingInternals

class CTreeViewBackingInternals {
	public:
						CTreeViewBackingInternals(const CTreeViewBacking::Info& info) :
							mInfo(info), mInternalInfo(removeViewItemIDs, noteTreeViewBackingItems, this)
							{}

		static	void	removeViewItemIDs(const TArray<CString>& viewItemIDs, void* userData)
							{
								// Setup
								CTreeViewBackingInternals&	internals = *((CTreeViewBackingInternals*) userData);

								// Remove
								internals.mTreeViewBackingItemMap.remove(viewItemIDs);
							}
		static	void	noteTreeViewBackingItems(const TArray<CTreeViewBackingItem>& treeViewBackingItems,
								void* userData)
							{
								// Setup
								CTreeViewBackingInternals&	internals = *((CTreeViewBackingInternals*) userData);

								// Iterate tree view backing items
								for (TIteratorD<CTreeViewBackingItem> iterator = treeViewBackingItems.getIterator();
										iterator.hasValue(); iterator.advance())
									// Update map
									internals.mTreeViewBackingItemMap.set(iterator->mViewItemID, *iterator);
							}

		const	CTreeViewBacking::Info&				mInfo;
				STreeViewBackingInternalInfo		mInternalInfo;

				TNDictionary<CTreeViewBackingItem>	mTreeViewBackingItemMap;
				TNArray<CString>					mTopLevelViewItemIDs;
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
	mInternals = new CTreeViewBackingInternals(info);
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
	mInternals->mTreeViewBackingItemMap.set(mRootViewItemID,
			CTreeViewBackingItem(rootTreeItem, mRootViewItemID, mInternals->mInfo, mInternals->mInternalInfo));

	mInternals->mTopLevelViewItemIDs.removeAll();
}

//----------------------------------------------------------------------------------------------------------------------
void CTreeViewBacking::set(const TArray<I<CTreeItem> >& topLevelTreeItems)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals->mTreeViewBackingItemMap.removeAll();

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
		CTreeViewBackingItem	treeViewBackingItem(*iterator, mInternals->mInfo, mInternals->mInternalInfo);

		// Store
		mInternals->mTreeViewBackingItemMap.set(treeViewBackingItem.mViewItemID, treeViewBackingItem);
		mInternals->mTopLevelViewItemIDs += treeViewBackingItem.mViewItemID;
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
	return mInternals->mTreeViewBackingItemMap[viewItemID]->mTreeItem;
}

//----------------------------------------------------------------------------------------------------------------------
TArray<I<CTreeItem> > CTreeViewBacking::getTreeItems(const TArray<CString>& viewItemIDs) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Collect tree items
	TNArray<I<CTreeItem> >	treeItems;
	for (TIteratorD<CString> iterator = viewItemIDs.getIterator(); iterator.hasValue(); iterator.advance())
		// Add tree item
		treeItems += mInternals->mTreeViewBackingItemMap[*iterator]->mTreeItem;

	return treeItems;
}

//----------------------------------------------------------------------------------------------------------------------
bool CTreeViewBacking::hasChildren(const CString& viewItemID) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CTreeViewBackingItem& treeViewBackingItem = *mInternals->mTreeViewBackingItemMap[viewItemID];

	// Check if have proc
	if (!mInternals->mInfo.canGetChildTreeItemsSync())
		// Query proc
		return mInternals->mInfo.hasChildTreeItems(treeViewBackingItem.mTreeItem);
	else {
		// Reload
		treeViewBackingItem.reloadChildItems();

		return !treeViewBackingItem.mChildViewItemIDs.isEmpty();
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CTreeViewBacking::getChildCount(const CString& viewItemID) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	OR<CTreeViewBackingItem>	treeViewBackingItem = mInternals->mTreeViewBackingItemMap[viewItemID];

	// Check situation
	if ((viewItemID == mRootViewItemID) && !treeViewBackingItem.hasReference())
		// Requesting root item, but no root item
		return mInternals->mTopLevelViewItemIDs.getCount();
	else {
		// Reload
		treeViewBackingItem->reloadChildItems();

		return treeViewBackingItem->mChildViewItemIDs.getCount();
	}
}

//----------------------------------------------------------------------------------------------------------------------
CString CTreeViewBacking::getChildViewItemID(const CString& viewItemID, UInt32 index) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	OR<CTreeViewBackingItem>	treeViewBackingItem = mInternals->mTreeViewBackingItemMap[viewItemID];

	// Check situation
	if ((viewItemID == mRootViewItemID) && !treeViewBackingItem.hasReference())
		// Requesting child of root item, but no root item
		return mInternals->mTreeViewBackingItemMap[mInternals->mTopLevelViewItemIDs[index]]->mViewItemID;
	else {
		// Reload
		treeViewBackingItem->reloadChildItems();

		return treeViewBackingItem->mChildViewItemIDs[index];
	}
}
