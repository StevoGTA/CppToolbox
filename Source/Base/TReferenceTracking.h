//----------------------------------------------------------------------------------------------------------------------
//	TReferenceTracking.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------------------------------------------------
// MARK: TReferenceCountable

template <typename T> class TReferenceCountable {
	public:
						// Lifecycle methods
						TReferenceCountable() : mReferenceCount(1) {}
		virtual			~TReferenceCountable() {}

						// Instance methods
				T*		addReference()
							{ mReferenceCount++; return (T*) this; }
				void	removeReference()
							{
								// Decrement reference count and check if we are the last one
								if (--mReferenceCount == 0) {
									// We going away
									T*	THIS = (T*) this;
									DisposeOf(THIS);
								}
							}
				UInt32	getReferenceCount() const
							{ return mReferenceCount; }

	private:
		UInt32	mReferenceCount;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: TCopyOnWriteReferenceCountable

template <typename T> class TCopyOnWriteReferenceCountable : public TReferenceCountable<T> {
	public:
			// Lifecycle methods
			TCopyOnWriteReferenceCountable() : TReferenceCountable<T>() {}
			~TCopyOnWriteReferenceCountable() {}

			// Instance methods
		T*	prepareForWrite()
				{
					// Check reference count.  If there is more than 1 reference, we implement a
					//	"copy on write".  So we will clone ourselves so we have a personal buffer that
					//	can be changed while leaving the exiting buffer as-is for the other references.
					if (TReferenceCountable<T>::getReferenceCount() > 1) {
						// Multiple references
						TReferenceCountable<T>::removeReference();

						return new T((T&) *this);
					} else
						// Only a single reference
						return (T*) this;
				}

	private:
		UInt32	mReferenceCount;
};
