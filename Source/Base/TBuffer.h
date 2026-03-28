//----------------------------------------------------------------------------------------------------------------------
//	TBuffer.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CppToolboxAssert.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: TBuffer

template <typename T> struct TBuffer {
	// Methods
	public:
					// Lifecycle methods
					TBuffer(UInt64 count)
						{
							// Setup
							mStorage = new T[count];
							mByteCount = count * sizeof(T);

							mReferenceCount = new UInt32;
							*mReferenceCount = 1;
						}
					TBuffer(T* buffer, UInt64 count) :
						mStorage(buffer), mByteCount(count * sizeof(T)), mReferenceCount(nil)
						{}
					TBuffer(const TBuffer<T>& other, UInt64 count) :
						mStorage(other.mStorage), mByteCount(count * sizeof(T)), mReferenceCount(other.mReferenceCount)
						{
							// Check
							AssertFailIf(mByteCount > other.mByteCount);

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
		UInt64		getCount() const
						{ return mByteCount / sizeof(T); }
		UInt64		getByteCount() const
						{ return mByteCount; }
		void		clear() const
						{ ::memset(mStorage, 0, mByteCount); }

		T*			operator*() const
						{ return mStorage; }
		T&			operator[](UInt64 index) const
						{ return mStorage[index]; }
		TBuffer<T>&	operator=(const TBuffer<T>& other)
						{
							// Check if need to cleanup
							if ((mReferenceCount != nil) && (--(*mReferenceCount) == 0)) {
								// Cleanup
								DeleteArray(mStorage);
								Delete(mReferenceCount);
							}

							// Copy values
							mStorage = other.mStorage;
							mByteCount = other.mByteCount;
							mReferenceCount = other.mReferenceCount;

							// Check for reference count
							if (mReferenceCount != nil)
								// Add reference
								(*mReferenceCount)++;

							return *this;
						}

	// Properties
	private:
		T*		mStorage;
		UInt64	mByteCount;
		UInt32*	mReferenceCount;
};
