//----------------------------------------------------------------------------------------------------------------------
//	TBatchQueue.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CArray.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: TBatchQueue

template <typename T, typename AT> class TBatchQueue {
	// Procs
	public:
		typedef	void	(*Proc)(const AT& items, void* userData);

	// Methods
	public:
				// Lifecycle methods
				TBatchQueue(UInt32 maximumBatchSize, Proc proc, void* userData = nil) :
					mMaximumBatchSize(maximumBatchSize), mProc(proc), mProcUserData(userData)
					{}
				TBatchQueue(Proc proc, void* userData = nil) :
					mMaximumBatchSize(500), mProc(proc), mProcUserData(userData)
					{}

				// Instance methods
		void	add(const T& item)
					{
						// Add
						mItems += item;

						// Check if time to process some
						if (mItems.getCount() >= mMaximumBatchSize) {
							// Time to process
							AT	itemsToProcess = mItems.popFirst(mMaximumBatchSize);
							mProc(itemsToProcess, mProcUserData);
						}
					}
		void	add(const AT& items)
					{
						// Add
						mItems += items;

						// Check if time to process some
						while (mItems.getCount() >= mMaximumBatchSize) {
							// Time to process
							AT	itemsToProcess = mItems.popFirst(mMaximumBatchSize);
							mProc(itemsToProcess, mProcUserData);
						}
					}
		void	finalize()
					{
						// Nothing to do if no items
						if (mItems.isEmpty())
							// No items
							return;

						// Call proc
						mProc(mItems, mProcUserData);

						// Cleanup
						mItems.removeAll();
					}

	// Properties
	private:
		UInt32	mMaximumBatchSize;
		Proc	mProc;
		void*	mProcUserData;

		AT		mItems;
};
