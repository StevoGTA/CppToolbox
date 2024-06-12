//----------------------------------------------------------------------------------------------------------------------
//	CBits.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "PlatformDefinitions.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CBits

class CDictionary;

class CBits {
	// Classes
	private:
		class Internals;

	// Methods
	public:
					// Lifecycle methods
					CBits(UInt32 count = 8, bool initialValue = false);
					CBits(const CDictionary& info);
					CBits(const CBits& other);
					~CBits();

					// Instance methods
		UInt32		getCount() const;
		bool		get(UInt32 index) const;
		CBits&		set(UInt32 index, bool value = true);
		CBits&		clear(UInt32 index)
						{ return set(index, false); }

		CDictionary	getInfo() const;

		CBits&		operator=(const CBits& other);
		bool		operator[](UInt32 index)
						{ return get(index); }

	// Properties
	private:
		Internals*	mInternals;
};
