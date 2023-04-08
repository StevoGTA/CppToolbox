//----------------------------------------------------------------------------------------------------------------------
//	CBits.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "PlatformDefinitions.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CBits

class CBits {
	// Methods
	public:
				// Lifecycle methods
				CBits(UInt32 count = 8);
				CBits(const CBits& other);
				~CBits();

				// Instance methods
		UInt32	getCount() const;
		bool	get(UInt32 index) const;
		void	set(UInt32 index, bool value = true);
		void	clear(UInt32 index)
					{ set(index, false); }

	// Properties
	private:
		class Internals;
		Internals*	mInternals;
};
