//----------------------------------------------------------------------------------------------------------------------
//	TWrappers-Windows.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------------------------------------------------
// MARK: CI (COM Instance)

template <typename T> struct CI {
			// Lifecycle methods
			CI(T* instance) : mInstance(instance), mReferenceCount(new UInt32) { *mReferenceCount = 1; }
			CI(const CI<T>& other) :
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

	CI<T>&	operator=(const CI<T>& other)
				{
					// Check for instance
					if (--(*mReferenceCount) == 0) {
						// All done
						mInstance->Release();
						Delete(mReferenceCount);
					}

					// Copy
					mInstance = other.mInstance;
					mReferenceCount = other.mReferenceCount;

					// Additional reference
					(*mReferenceCount)++;

					return *this;
				}

			// Instamce methods
	T*		operator*() const
				{ return mInstance; }
	T*		operator->() const
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
			OCI(T* instance = nullptr) : mInstance(instance), mReferenceCount(new UInt32) { *mReferenceCount = 1; }
			OCI(const OCI<T>& other) :
				mInstance(other.mInstance), mReferenceCount(other.mReferenceCount)
				{
					// One more reference
					(*mReferenceCount)++;
				}
			~OCI()
				{
					// One less reference
					if (--(*mReferenceCount) == 0) {
						// All done
						if (mInstance != nullptr)
							// Release instance
							mInstance->Release();

						// Cleanup
						Delete(mReferenceCount);
					}
				}

	OCI<T>&	operator=(const OCI<T>& other)
				{
					// One less reference
					if (--(*mReferenceCount) == 0) {
						// All done
						if (mInstance != nullptr)
							// Release instance
							mInstance->Release();

						// Cleanup
						Delete(mReferenceCount);
					}

					// Copy
					mInstance = other.mInstance;
					mReferenceCount = other.mReferenceCount;

					// One more reference
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
