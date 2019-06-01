//----------------------------------------------------------------------------------------------------------------------
//	CEquatable.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "PlatformDefinitions.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CEquatable

class CEquatable {
	// Methods
	public:
						// Lifecycle methods
						CEquatable() {}
		virtual			~CEquatable() {}

						// Instance methods
				bool	operator!=(const CEquatable& other) const
							{ return !operator==(other); }

						// Subclass methods
		virtual	bool	operator==(const CEquatable& other) const = 0;
};
