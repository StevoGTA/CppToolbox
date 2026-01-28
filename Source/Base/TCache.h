
//----------------------------------------------------------------------------------------------------------------------
//	TCache.h			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: TCache2D

template <typename T> class TCache2D {
	// Methods
	public:
						// Lifecycle methods
						TCache2D() {}

						// Instance methods
				void	add(const CString& columnID, const CString& rowID, const T& t)
							{
								// Setup
								CString	itemIdentifier = columnID + CString::mHyphen + rowID;

								// Update
								OR<TNSet<CString> >	itemIdentifiersForColumn = mItemIdentifiersByColumnID.get(columnID);
								if (itemIdentifiersForColumn.hasReference())
									// Insert
									itemIdentifiersForColumn->insert(itemIdentifier);
								else
									// Store
									mItemIdentifiersByColumnID.set(columnID, TNSet<CString>(itemIdentifier));

								OR<TNSet<CString> >	itemIdentifiersForRow = mItemIdentifiersByRowID.get(rowID);
								if (itemIdentifiersForRow.hasReference())
									// Insert
									itemIdentifiersForRow->insert(itemIdentifier);
								else
									// Store
									mItemIdentifiersByRowID.set(rowID, TNSet<CString>(itemIdentifier));

								mItemByIdentifier.set(itemIdentifier, t);
							}
		const	OR<T>	get(const CString& columnID, const CString& rowID) const
							{
								// Setup
								CString	itemIdentifier = columnID + CString::mHyphen + rowID;

								return mItemByIdentifier.get(itemIdentifier);
							}

				void	invalidate()
							{
								// Cleanup everything
								mItemByIdentifier.removeAll();
								mItemIdentifiersByColumnID.removeAll();
								mItemIdentifiersByRowID.removeAll();
							}
				void	invalidateColumn(const CString& columnID)
							{
								// Retrieve item dentifiers for this column
								OR<TNSet<CString> >	itemIdentifiersForColumn = mItemIdentifiersByColumnID.get(columnID);
								if (!itemIdentifiersForColumn.hasReference())
									return;

								// Iterate item identifiers
								for (TIteratorS<CString> iterator = itemIdentifiersForColumn->getIterator();
										iterator.hasValue(); iterator.advance())
									// Remove
									mItemByIdentifier.remove(*iterator);
							}
				void	invalidateRow(const CString& rowID)
							{
								// Retrieve item identifiers for this row
								OR<TNSet<CString> >	itemIdentifiersForRow = mItemIdentifiersByRowID.get(rowID);
								if (!itemIdentifiersForRow.hasReference())
									return;

								// Iterate item identifiers
								for (TIteratorS<CString> iterator = itemIdentifiersForRow->getIterator();
										iterator.hasValue(); iterator.advance())
									// Remove
									mItemByIdentifier.remove(*iterator);
							}
				void	invalidate(const CString& columnID, const CString& rowID)
							{
								// Setup
								CString	itemIdentifier = columnID + CString::mHyphen + rowID;

								// Remove
								mItemByIdentifier.remove(itemIdentifier);
							}

	// Properties
	private:
		TNDictionary<T>					mItemByIdentifier;
		TNDictionary<TNSet<CString> >	mItemIdentifiersByColumnID;
		TNDictionary<TNSet<CString> >	mItemIdentifiersByRowID;
};
