//----------------------------------------------------------------------------------------------------------------------
//	TRange.h			©2024 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "TWrappers.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: TRange

template <typename T> struct TRange {
	// Methods
	public:
						// Lifecycle methods
						TRange(T start, T length) : mStart(start), mLength(length) {}

						// Instance methods
		T				getStart() const
							{ return mStart; }
		T				getLength() const
							{ return mLength; }
		T				getEnd() const
							{ return mStart + mLength; }

		bool			contains(const TRange<T>& range) const
							{ return (range.mStart >= mStart) && (range.getEnd() <= getEnd()); }
		bool			overlaps(const TRange<T>& range) const
							{ return std::max<T>(mStart, range.mStart) < std::min<T>(getEnd(), range.getEnd()); }

		TRange<T>		offset(T amount) const
							{ return TRange<T>(mStart + amount, mLength); }
		TRange<T>		inserting(T position, T length) const
							{
								// Check situation
								if (mStart >= position)
									// Move later
									return TRange<T>(mStart + length, mLength);
								else if (getEnd() > position)
									// Grow
									return TRange<T>(mStart, mLength + length);
								else
									// Before
									return *this;
							}
		OV<TRange<T> >	removing(const TRange<T>& range) const
							{
								// Check situation
								if (mStart >= range.getEnd())
									// After
									return OV<TRange<T> >(TRange<T>(mStart - range.mLength, mLength));
								else if (getEnd() <= range.mStart)
									// Before
									return OV<TRange<T> >(*this);
								else {
									// Overlapping - keep what remains
									T	length =
												mLength -
														(std::min<T>(getEnd(), range.getEnd()) -
																std::max<T>(mStart, range.mStart));

									return (length > 0) ?
											OV<TRange<T> >(TRange<T>(std::min<T>(mStart, range.mStart), length)) :
											OV<TRange<T> >();
								}
							}

	// Properties
	private:
		T	mStart;
		T	mLength;
};

typedef	TRange<UInt32>	SRange32;
typedef	TRange<UInt64>	SRange64;
