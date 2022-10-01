//----------------------------------------------------------------------------------------------------------------------
//	CHashing.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CEquatable.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CHashable

class CHasher;

class CHashable : public CEquatable {
	// Methods
	public:
						// Lifecycle methods
						CHashable() : CEquatable() {}
						~CHashable() {}

						// Subclass methods
		virtual	void	hashInto(CHasher& hasher) const = 0;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CHasher

class CHasherInternals;
class CHasher {
	// Methods
	public:
						// Lifecycle methods
						CHasher();
						~CHasher();

						// Instance methods
				void	add(const char* string);
				void	add(const UInt8* bytePtr, const UInt32 byteCount);

				UInt32	getValue() const;

						// Class methods
		static	UInt32	getValueForHashable(const CHashable& hashable)
							{ CHasher hasher; hashable.hashInto(hasher); return hasher.getValue(); }

	// Properties
	private:
		CHasherInternals*	mInternals;
};
