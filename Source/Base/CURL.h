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
		explicit			CURL(const CString& string);
							CURL(const CURL& other);
							~CURL();

							// Instance methods
					CString	getString() const;

					CURL&	operator=(const CURL& other);

	// Properties
	private:
		Internals*	mInternals;
};
