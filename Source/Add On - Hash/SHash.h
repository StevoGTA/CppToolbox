//----------------------------------------------------------------------------------------------------------------------
//	SHash.h			©2024 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CData.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SHash

struct SHash {
	// Type
	public:
		enum Type {
			xxHash32,
			xxHash64,
			xxHash3_64,
			xxHash3_128,
		};
	
	// Methods
	public:
						// Class methods
		static	CString valueFor(const CData& data, Type type = xxHash3_64, bool uppercase = false);
};
