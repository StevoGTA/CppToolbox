//----------------------------------------------------------------------------------------------------------------------
//	CReferenceCountable.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "PlatformDefinitions.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CReferenceCountable

class CReferenceCountable {
	// Methods
	public:
						// Lifecycle methods
		virtual			~CReferenceCountable()
							{
								// Check
								if (getReferenceCount() > 0)
									// Remove reference
									removeReference();
								else
									// Cleanup
									Delete(mReferenceCount);
							}

						// Instance methods
				void	removeReference()
							{
								// Decrement reference count and check if we are the last one
								if (--(*mReferenceCount) == 0)
									// Cleanup
									cleanup();
							}

	protected:
						// Lifecycle methods
						CReferenceCountable() : mReferenceCount(new std::atomic<UInt32>(1)) {}
						CReferenceCountable(const CReferenceCountable& other) :
							mReferenceCount(other.mReferenceCount)
							{ addReferenceInternal(); }

						// Subclass methods
				void	addReferenceInternal()
							{ (*mReferenceCount)++; }
				UInt32	getReferenceCount() const
							{ return mReferenceCount->load(); }

		virtual	void	cleanup()
							{}

	// Properties
	private:
		std::atomic<UInt32>*	mReferenceCount;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: TReferenceCountable

template <typename T> class TReferenceCountable : public CReferenceCountable {
	// Methods
	public:
			// Instance methods
		T*	addReference()
				{ addReferenceInternal(); return (T*) this; }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: TReferenceCountableAutoDelete

template <typename T> class TReferenceCountableAutoDelete : public TReferenceCountable<T> {
	// Methods
	protected:
				// Lifecycle methods
				TReferenceCountableAutoDelete() : TReferenceCountable<T>() {}
				TReferenceCountableAutoDelete(const TReferenceCountableAutoDelete<T>& other) :
					TReferenceCountable<T>(other)
					{}

				// TReferenceCountable methods
		void	cleanup()
					{ T* _this = (T*) this; Delete(_this); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TCopyOnWriteReferenceCountable

template <typename T> class TCopyOnWriteReferenceCountable : public TReferenceCountableAutoDelete<T> {
	// Methods
	public:
						// Class methods
		static	void	prepareForWrite(T** t)
							{
								// Check reference count.  If there is more than 1 reference, we implement a
								//	"copy on write".  So we will clone ourselves so we have a personal buffer that
								//	can be changed while leaving the exiting buffer as-is for the other references.
								if ((*t)->CReferenceCountable::getReferenceCount() > 1) {
									// Save
									T*	oldT = *t;

									// Create new
									*t = new T((T&) **t);

									// No longer need a reference to old
									oldT->removeReference();
								}
							}

	protected:
						// Lifecycle methods
						TCopyOnWriteReferenceCountable() : TReferenceCountableAutoDelete<T>() {}
						TCopyOnWriteReferenceCountable(const TCopyOnWriteReferenceCountable<T>& other) :
							TReferenceCountableAutoDelete<T>(other)
							{}
};
