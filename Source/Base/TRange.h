//----------------------------------------------------------------------------------------------------------------------
//	TRange.h			©2024 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "PlatformDefinitions.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: TRange

template <typename T> struct TRange {
	// Methods
	public:
		// Lifecycle methods
		TRange(T start, T length) : mStart(start), mLength(length) {}
		TRange(const TRange<T>& other) : mStart(other.mStart), mLength(other.mLength) {}

			// Instance methods
		T	getStart() const
				{ return mStart; }
		T	getLength() const
				{ return mLength; }

	// Properties
	private:
		T	mStart;
		T	mLength;
};

typedef	TRange<UInt32>	SRange32;
typedef	TRange<UInt64>	SRange64;
