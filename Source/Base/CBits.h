//----------------------------------------------------------------------------------------------------------------------
//	CBits.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "PlatformDefinitions.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CBits

class CBits {
	// Classes
	private:
		class Internals;

	// Methods
	public:
				// Lifecycle methods
				CBits(UInt32 count = 8, bool initialValue = false);
				CBits(const CBits& other);
				~CBits();

				// Instance methods
		UInt32	getCount() const;
		bool	get(UInt32 index) const;
		void	set(UInt32 index, bool value = true);
		void	clear(UInt32 index)
					{ set(index, false); }

		CBits&	operator=(const CBits& other);

	// Properties
	private:
		Internals*	mInternals;
};
