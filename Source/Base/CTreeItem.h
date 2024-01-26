//----------------------------------------------------------------------------------------------------------------------
//	CTreeItem.h			©2021 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------------------------------------------------
// MARK: CTreeItem

class CTreeItem {
	// Methods
	public:

										// Lifecycle methods
										CTreeItem() {}
		virtual							~CTreeItem() {}

										// Instance methods
		virtual	bool					hasChildren() const
											{ return false; }

										// Subclass methods
		virtual	OSType					getType() const = 0;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TItemTreeItem

template <typename T> class TItemTreeItem : public CTreeItem {
	// Methods
	public:
			// Lifecycle methods
			TItemTreeItem(T& item) : mItem(item) {}

			// Instance methods
		T&	getItem() const
				{ return mItem; }

	// Properties
	private:
		T&	mItem;
};
