//----------------------------------------------------------------------------------------------------------------------
//	CTableViewBacking.h			©2025 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CColor.h"
#include "CTableViewItem.h"
#include "SValue.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CTableViewBacking

class CTableViewBacking {
	// Classes
	private:
		class Internals;

	// Methods
	public:
				// Lifecycle methods
				CTableViewBacking();
		virtual	~CTableViewBacking();

	// Properties
	private:
		Internals*	mInternals;
};
