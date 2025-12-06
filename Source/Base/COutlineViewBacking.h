//----------------------------------------------------------------------------------------------------------------------
//	COutlineViewBacking.h			©2025 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"
#include "COutlineViewItem.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: COutlineViewBacking

class COutlineViewBacking {
	// Classes
	private:
		class Internals;

	// Methods
	public:
												// Lifecycle methods
												COutlineViewBacking();
		virtual									~COutlineViewBacking();

												// Instance methods
		const	TArray<CString>&				getTopLevelItemIDs() const;
				bool							hasChildren(const CString& itemID) const;
				UInt32							getChildCount(const OV<CString>& itemID) const;
				CString							getChildItemID(const OV<CString>& itemID, UInt32 index) const;
		const	I<COutlineViewItem>&			getOutlineViewItem(const CString& itemID) const;
				TArray<I<COutlineViewItem> >	getOutlineViewItems(const TArray<CString>& itemIDs) const;

				void							set(const TArray<I<COutlineViewItem> >& outlineViewItems,
														const OV<CString>& parentItemID = OV<CString>());
				void							add(const TArray<I<COutlineViewItem> >& outlineViewItems,
														const OV<CString>& parentItemID = OV<CString>());

	// Properties
	private:
		Internals*	mInternals;
};
