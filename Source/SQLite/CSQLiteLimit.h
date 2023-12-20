//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteLimit.h			©2023 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSQLiteLimit

class CSQLiteLimit {
	// Classes
	private:
		class Internals;

	// Methods
	public:
							// Lifecycle methods
							CSQLiteLimit(UInt32 limit, const OV<UInt32>& offset = OV<UInt32>());
							~CSQLiteLimit();

							// Instance methods
		const	CString&	getString() const;

	// Properties
	private:
		Internals*	mInternals;
};
