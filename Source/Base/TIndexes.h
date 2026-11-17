//----------------------------------------------------------------------------------------------------------------------
//	TIndexes.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CBits.h"
#include "TRange.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: TIndexes

template <typename T> class TIndexes : CBits {
	// Methods
	public:
										// Instance methods
					void				add(T index)
											{ set(index); }
					void				remove(T index)
											{ clear(index); }

					TArray<TRange<T> >	getRanges() const
											{
												// Setup
												TNArray<TRange<T> >	ranges;

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
														ranges += TRange<T>(*start, *end - *start + 1);
														start.removeValue();
													}
												}

												// Check for one more range
												if (start.hasValue())
													// Add last range
													ranges += TRange<T>(*start, *end - *start + 1);

												return ranges;
											}

										// Class methods
		static		TIndexes<T>			withCount(UInt32 count)
											{ return TIndexes<T>(count); }
		static		TIndexes<T>			forIndex(T index)
											{
												TIndexes<T>	indexes(1);
												indexes.add(index);

												return indexes;
											}

	private:
										// Lifecycle methods
		explicit						TIndexes(UInt32 count) : CBits(count) {}
};
