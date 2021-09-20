//----------------------------------------------------------------------------------------------------------------------
//	TWrappers-Windows.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------------------------------------------------
// MARK: CI (COM Instance)

template <typename T> struct CI {
		// Lifecycle methods
		CI(T* instance) : mInstance(instance), mReferenceCount(new UInt32) { *mReferenceCount = 1; }
		CI(const I<T>& other) :
			mInstance(other.mInstance), mReferenceCount(other.mReferenceCount)
			{ (*mReferenceCount)++; }
		~CI()
			{
				// One less reference
				if (--(*mReferenceCount) == 0) {
					// All done
					mInstance->Release();
					Delete(mReferenceCount);
				}
			}

		// Instamce methods
	T*	operator*() const
			{ return mInstance; }
	T*	operator->() const
			{ return mInstance; }

	// Properties
	private:
		T*		mInstance;
		UInt32*	mReferenceCount;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: OCI (Optional COM Instance)

template <typename T> struct OCI {
			// Lifecycle methods
			OCI() : mInstance(nullptr), mReferenceCount(nullptr) {}
			OCI(T* instance) : mInstance(instance), mReferenceCount(new UInt32) { *mReferenceCount = 1; }
			OCI(const OCI<T>& other) :
				mInstance(other.mInstance), mReferenceCount(other.mReferenceCount)
				{
					// Check if have reference
					if (mInstance != nullptr)
						// Additional reference
						(*mReferenceCount)++;
				}
			~OCI()
				{
					// Check for instance
					if ((mInstance != nullptr) && (--(*mReferenceCount) == 0)) {
						// All done
						mInstance->Release();
						Delete(mReferenceCount);
					}
				}

	OCI<T>&	operator=(const OCI<T>& other)
				{
					// Check for instance
					if ((mInstance != nullptr) && (--(*mReferenceCount) == 0)) {
						// All done
						mInstance->Release();
						Delete(mReferenceCount);
					}

					// Copy
					mInstance = other.mInstance;
					mReferenceCount = other.mReferenceCount;

					// Note additional reference if have reference
					if (mInstance != nullptr)
						// Additional reference
						(*mReferenceCount)++;

					return *this;
				}

			// Instamce methods
	bool	hasInstance() const
				{ return mInstance != nullptr; }

	T*		operator*() const
				{ AssertFailIf(mInstance == nullptr); return mInstance; }
	T*		operator->() const
				{ AssertFailIf(mInstance == nullptr); return mInstance; }

	// Properties
	private:
		T*		mInstance;
		UInt32*	mReferenceCount;
 };
