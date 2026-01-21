//----------------------------------------------------------------------------------------------------------------------
//	CTableViewBacking.cpp			©2025 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CTableViewBacking.h"

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CTableViewBacking::Internals

class CTableViewBacking::Internals {
	public:
//						Internals(const TArray<I<Column> >& columns) :
//							mColumns(columns)
//							{
//								// Setup
//								for (TIteratorD<I<Column> > iterator = columns.getIterator(); iterator.hasValue();
//										iterator.advance())
//									// Store
//									mColumnByIdentifier.set((*iterator)->getIdentifier(), *iterator);
//							}
//
//		static	bool	isColumnDisplayed(const I<Column>& column, void* userData)
//							{ return column->isDisplayed(); }
//
//		TNArray<I<Column> >			mColumns;
//		TNDictionary<I<Column> >	mColumnByIdentifier;
//
//		OV<CTableViewItem::Procs>	mTableViewItemProcs;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CTableViewBacking

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CTableViewBacking::CTableViewBacking(const TArray<I<Column> >& columns)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(columns);
}

//----------------------------------------------------------------------------------------------------------------------
CTableViewBacking::~CTableViewBacking()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}
