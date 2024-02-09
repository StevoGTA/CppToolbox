//----------------------------------------------------------------------------------------------------------------------
//	CTreeViewBacking.h			©2021 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"
#include "CTreeItem.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CTreeViewBacking

class CTreeViewBacking {
	// Info
	public:
		struct Info {
			public:
				// Types
				typedef	TMArray<I<CTreeItem> >	(*GetChildTreeItemsProc)(const I<CTreeItem>& treeItem, void* userData);

				typedef	void					(*LoadChildTreeItemsCompletionProc)(
														const TArray<I<CTreeItem> > treeItems);
				typedef	void					(*LoadChildTreeItemsProc)(const I<CTreeItem>& treeItem,
														LoadChildTreeItemsCompletionProc completionProc,
														void* userData);

				typedef	bool					(*CompareTreeItemsProc)(const I<CTreeItem>& treeItem1,
														const I<CTreeItem>& treeItem2, void* userData);

			public:
				// Methods
													// Lifecycle methods
													Info(GetChildTreeItemsProc getChildTreeItemsProc,
															CompareTreeItemsProc compareTreeItemsProc, void* userData) :
														mGetChildTreeItemsProc(getChildTreeItemsProc),
																mLoadChildTreeItemsProc(nil),
																mCompareTreeItemsProc(compareTreeItemsProc),
																mUserData(userData)
														{}
													Info(LoadChildTreeItemsProc loadChildTreeItemsProc,
															CompareTreeItemsProc compareTreeItemsProc, void* userData) :
														mGetChildTreeItemsProc(nil),
																mLoadChildTreeItemsProc(loadChildTreeItemsProc),
																mCompareTreeItemsProc(compareTreeItemsProc),
																mUserData(userData)
														{}
													Info(const Info& other) :
														mGetChildTreeItemsProc(other.mGetChildTreeItemsProc),
																mLoadChildTreeItemsProc(other.mLoadChildTreeItemsProc),
																mCompareTreeItemsProc(other.mCompareTreeItemsProc),
																mUserData(other.mUserData)
														{}

													// Instance methods
							bool					canGetChildTreeItemsSync() const
														{ return mGetChildTreeItemsProc != nil; }

							TMArray<I<CTreeItem> >	getChildTreeItems(const I<CTreeItem>& treeItem) const
														{ return mGetChildTreeItemsProc(treeItem, mUserData); }

							void					loadChildTreeItems(const I<CTreeItem>& treeItem) const
														{ mLoadChildTreeItemsProc(treeItem,
																loadChildTreeItemsCompletion, mUserData); }

													// Class methods
					static	void					loadChildTreeItemsCompletion(const TArray<I<CTreeItem> > treeItems)
														{}

			// Properties
			private:
				GetChildTreeItemsProc	mGetChildTreeItemsProc;

				LoadChildTreeItemsProc	mLoadChildTreeItemsProc;

				CompareTreeItemsProc	mCompareTreeItemsProc;

				void*					mUserData;
		};

	// Classes
	private:
		class Internals;

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
		static	const	CString		mRootViewItemID;

	private:
						Internals*	mInternals;
};
