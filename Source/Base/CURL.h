//----------------------------------------------------------------------------------------------------------------------
//	CURL.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CURL

class CURL {
	// Classes
	private:
		class Internals;

	// Methods
	public:
				// Lifecycle methods
				CURL(const CString& string);
				CURL(const CURL& other);
				~CURL();

				// Instance methods
		CString	getString() const;

	// Properties
	private:
		Internals*	mInternals;
};
