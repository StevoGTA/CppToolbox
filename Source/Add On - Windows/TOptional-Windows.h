//----------------------------------------------------------------------------------------------------------------------
//	TOptional-Windows.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

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
