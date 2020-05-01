//----------------------------------------------------------------------------------------------------------------------
//	TIndexes.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CBits.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: TIndexRange

template <typename T> struct TIndexRange {

	// Lifecycle methods
	TIndexRange(T start, T end) : mStart(start), mEnd(end) {}
	TIndexRange(const TIndexRange<T>& other) : mStart(other.mStart), mEnd(other.mEnd) {}

	// Properties
	T	mStart;
	T	mEnd;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TIndexes

template <typename T> class TIndexes : CBits {
	// Methods
	public:
										// Instance methods
				void					add(T index)
											{ set(index); }
				void					remove(T index)
											{ clear(index); }

				TArray<TIndexRange<T> >	getRanges() const
											{
												// Setup
												TNArray<TIndexRange<T> >	ranges;

												// Iterate all bits
												OV<T>	start;
												OV<T>	end;
												for (UInt32 i = 0; i < getCount(); i++) {
													// Check this bit
													if (get(i)) {
														// Check if have range
														if (!start.hasValue())
															// Start of new range
															start.setValue(i);

														// Continue existing range
														end.setValue(i);
													} else if (start.hasValue()) {
														// Add range
														ranges += TIndexRange<T>(*start, *end);
														start.removeValue();
													}
												}

												// Check for one more range
												if (start.hasValue())
													// Add last range
													ranges += TIndexRange<T>(*start, *end);

												return ranges;
											}

										// Class methods
		static	TIndexes<T>				withCount(UInt32 count)
											{ return TIndexes<T>(count); }
		static	TIndexes<T>				forIndex(T index)
											{
												TIndexes<T>	indexes(1);
												indexes.add(index);

												return indexes;
											}

	private:
										// Lifecycle methods
										TIndexes(UInt32 count) : CBits(count) {}
};
