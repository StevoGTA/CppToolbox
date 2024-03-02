//----------------------------------------------------------------------------------------------------------------------
//	CHashable.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CEquatable.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CHashable

class CHashable : public CEquatable {
	// HashCollector
	public:
		class HashCollector {
			// Classes
			private:
				class Internals;

			// Methods
			public:
						// Lifecycle methods
						HashCollector();
						~HashCollector();

						// Instance methods
				void	add(const char* string);
				void	add(const UInt8* bytePtr, const UInt32 byteCount);

				UInt32	getValue() const;

			// Properties
			private:
				Internals*	mInternals;
		};

	// Methods
	public:
						// Lifecycle methods
						CHashable() : CEquatable() {}

						// Instance methods
				UInt32	getHashValue() const
							{ HashCollector hashCollector; hashInto(hashCollector); return hashCollector.getValue(); }

						// Subclass methods
		virtual	void	hashInto(HashCollector& hashCollector) const = 0;
};
