//----------------------------------------------------------------------------------------------------------------------
//	TBuffer.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "PlatformDefinitions.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: TBuffer

template <typename T> struct TBuffer {
	// Methods
	public:
				// Lifecycle methods
				TBuffer(UInt32 count)
					{
						// Setup
						mStorage = new T[count];
						mByteCount = count * sizeof(T);

						mReferenceCount = new UInt32;
						*mReferenceCount = 1;
					}
				TBuffer(T* buffer, UInt32 size) : mStorage(buffer), mByteCount(size), mReferenceCount(nil) {}
				TBuffer(const TBuffer<T>& other, UInt32 count) :
					mStorage(other.mStorage), mByteCount(count), mReferenceCount(other.mReferenceCount)
					{
						// Check for reference count
						if (mReferenceCount != nil)
							// Add reference
							(*mReferenceCount)++;
					}
				TBuffer(const TBuffer<T>& other) :
					mStorage(other.mStorage), mByteCount(other.mByteCount), mReferenceCount(other.mReferenceCount)
					{
						// Check for reference count
						if (mReferenceCount != nil)
							// Add reference
							(*mReferenceCount)++;
					}
				~TBuffer()
					{
						// Check if need to cleanup
						if ((mReferenceCount != nil) && (--(*mReferenceCount) == 0)) {
							// Cleanup
							DeleteArray(mStorage);
							Delete(mReferenceCount);
						}
					}

				// Instance methods
		T*		operator*() const
					{ return mStorage; }
		T&		operator[](UInt32 index) const
					{ return mStorage[index]; }
		UInt32	getCount() const
					{ return mByteCount / sizeof(T); }
		UInt32	getByteCount() const
					{ return mByteCount; }
		void	clear() const
					{ ::memset(mStorage, 0, mByteCount); }

	// Properties
	private:
		T*		mStorage;
		UInt32	mByteCount;
		UInt32*	mReferenceCount;
};
