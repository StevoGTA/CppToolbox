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
							CSQLiteLimit(const OV<UInt32> limit, UInt32 offset);
							CSQLiteLimit(const OV<UInt32> limit);
							~CSQLiteLimit();

							// Instance methods
		const	CString&	getString() const;

	// Properties
	private:
		Internals*	mInternals;
};
