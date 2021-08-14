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
	// Info
	public:
		struct Info {
			// Types
			typedef	TArray<I<CTreeItem> >	(*ChildTreeItemsProc)(const I<CTreeItem>& treeItem, void* userData);

			typedef	bool					(*HasChildTreeItemsProc)(const I<CTreeItem>& treeItem, void* userData);
			typedef	void					(*LoadChildTreeItemsCompletionProc)(const TArray<I<CTreeItem> > treeItems);
			typedef	void					(*LoadChildTreeItemsProc)(const I<CTreeItem>& treeItem,
													LoadChildTreeItemsCompletionProc completionProc, void* userData);

			typedef	bool					(*CompareTreeItemsProc)(const I<CTreeItem>& treeItem1,
													const I<CTreeItem>& treeItem2, void* userData);

											// Lifecycle methods
											Info(HasChildTreeItemsProc hasChildTreeItemsProc,
													ChildTreeItemsProc childTreeItemsProc,
													LoadChildTreeItemsProc loadChildTreeItemsProc,
													CompareTreeItemsProc compareTreeItemsProc, void* userData) :
												mHasChildTreeItemsProc(hasChildTreeItemsProc),
														mChildTreeItemsProc(childTreeItemsProc),
														mLoadChildTreeItemsProc(loadChildTreeItemsProc),
														mCompareTreeItemsProc(compareTreeItemsProc), mUserData(userData)
												{}
											Info(const Info& other) :
												mHasChildTreeItemsProc(other.mHasChildTreeItemsProc),
														mChildTreeItemsProc(other.mChildTreeItemsProc),
														mLoadChildTreeItemsProc(other.mLoadChildTreeItemsProc),
														mCompareTreeItemsProc(other.mCompareTreeItemsProc),
														mUserData(other.mUserData)
												{}

											// Instance methods
					bool					canGetChildTreeItemsSync() const
												{ return mChildTreeItemsProc != nil; }

					TArray<I<CTreeItem> >	getChildTreeItems(const I<CTreeItem>& treeItem) const
												{ return mChildTreeItemsProc(treeItem, mUserData); }

					bool					hasChildTreeItems(const I<CTreeItem>& treeItem) const
												{ return mHasChildTreeItemsProc(treeItem, mUserData); }
					void					loadChildTreeItems(const I<CTreeItem>& treeItem) const
												{ mLoadChildTreeItemsProc(treeItem, loadChildTreeItemsCompletion,
														mUserData); }

											// Class methods
			static	void					loadChildTreeItemsCompletion(const TArray<I<CTreeItem> > treeItems)
												{}

			// Properties
			public:
				CompareTreeItemsProc	mCompareTreeItemsProc;

			private:
				ChildTreeItemsProc		mChildTreeItemsProc;

				HasChildTreeItemsProc	mHasChildTreeItemsProc;
				LoadChildTreeItemsProc	mLoadChildTreeItemsProc;

				void*					mUserData;
		};
	// Methods
	public:
										// Lifecycle methods
										CTreeViewBacking(const Info& info);
										~CTreeViewBacking();

										// Instance methods
				void					set(const I<CTreeItem>& rootTreeItem);

				void					set(const TArray<I<CTreeItem> >& topLevelTreeItems);
				void					add(const TArray<I<CTreeItem> >& topLevelTreeItems);

				TArray<I<CTreeItem> >	getTopLevelTreeItems() const;
		const	I<CTreeItem>&			getTreeItem(const CString& viewItemID) const;
				TArray<I<CTreeItem> >	getTreeItems(const TArray<CString>& viewItemIDs) const;

				bool					hasChildren(const CString& viewItemID) const;
				UInt32					getChildCount(const CString& viewItemID) const;
				CString					getChildViewItemID(const CString& viewItemID, UInt32 index) const;

	// Properties
	public:
		static	CString						mRootViewItemID;

	private:
				CTreeViewBackingInternals*	mInternals;
};
