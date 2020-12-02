//----------------------------------------------------------------------------------------------------------------------
//	TInstance.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------------------------------------------------
// MARK: I (Instance)

template <typename T> struct I {
		// Lifecycle methods
		I(T* instance) : mInstance(instance), mReferenceCount(new UInt32) { *mReferenceCount = 1; }
		I(const I<T>& other) :
			mInstance(other.mInstance), mReferenceCount(other.mReferenceCount)
			{ (*mReferenceCount)++; }
		~I()
			{
				// One less reference
				if (--(*mReferenceCount) == 0)
					// All done
					Delete(mInstance);
			}

		// Instamce methods
	T&	operator*() const
			{ return *mInstance; }
	T*	operator->() const
			{ return mInstance; }

	// Properties
	private:
		T*		mInstance;
		UInt32*	mReferenceCount;
 };
