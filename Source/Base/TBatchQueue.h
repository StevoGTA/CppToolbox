//----------------------------------------------------------------------------------------------------------------------
//	TBatchQueue.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------------------------------------------------
// MARK: TBatchQueue

template <typename T> class TBatchQueue {
	// Procs
	public:
		typedef	void	(*Proc)(const TArray<T>& items, void* userData);

	// Methods
	public:
				// Lifecycle methods
				TBatchQueue(UInt32 maximumBatchSize, Proc proc, void* userData)
					{
						// Store
						mMaximumBatchSize = maximumBatchSize;
						mProc = proc;
						mUserData = userData;
					}
				TBatchQueue(Proc proc, void* userData)
					{
						// Setup
						mMaximumBatchSize = 500;
						mProc = proc;
						mUserData = userData;
					}

				// Instance methods
		void	add(const T& item)
					{
						// Add
						mItems += item;

						// Check if time to process some
						if (mItems.getCount() >= mMaximumBatchSize) {
							// Time to process
							TArray<T>	itemsToProcess = mItems.popFirst(mMaximumBatchSize);
							mProc(itemsToProcess, mUserData);
						}
					}
		void	add(const TArray<T>& items)
					{
						// Add
						mItems += items;

						// Check if time to process some
						while (mItems.getCount() >= mMaximumBatchSize) {
							// Time to process
							TArray<T>	itemsToProcess = mItems.popFirst(mMaximumBatchSize);
							mProc(itemsToProcess, mUserData);
						}
					}
		void	finalize()
					{
						// Nothing to do if no items
						if (mItems.isEmpty())
							// No items
							return;

						// Call proc
						mProc(mItems, mUserData);

						// Cleanup
						mItems.removeAll();
					}

	// Properties
	private:
		UInt32		mMaximumBatchSize;
		Proc		mProc;
		void*		mUserData;

		TNArray<T>	mItems;
};
