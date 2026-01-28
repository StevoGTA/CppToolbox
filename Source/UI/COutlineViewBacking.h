//----------------------------------------------------------------------------------------------------------------------
//	COutlineViewBacking.h			©2025 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "COutlineViewItem.h"
#include "CTableViewBacking.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: COutlineViewBacking

class COutlineViewBacking : public CTableViewBacking {
	// Classes
	private:
		class Internals;

	// Methods
	public:
		// Lifecycle methods
		COutlineViewBacking();
		~COutlineViewBacking();

	// Properties
	private:
		Internals*	mInternals;
};
