//----------------------------------------------------------------------------------------------------------------------
//	TReferenceTracking.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------------------------------------------------
// MARK: SReferenceTracker
struct SReferenceTracker {
	// Methods
	public:
									// Lifecycle methods
									SReferenceTracker() : mReferenceCount(1) {}
		virtual						~SReferenceTracker() {}

									// Instance methods
				SReferenceTracker*	addReference()
										{ mReferenceCount++; return this; }
				void				removeReference()
										{
											// One less reference
											if (--mReferenceCount == 0) {
												// Dispose of us
												SReferenceTracker*	THIS = this;
												DisposeOf(THIS);
											}
										}

	// Properties
	private:
		UInt32	mReferenceCount;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: SObjectReferenceTracker
template <typename T> struct SObjectReferenceTracker : public SReferenceTracker {
	// Methods
	public:
		// Lifecycle methods
		SObjectReferenceTracker(T* object) : SReferenceTracker(), mObject(object) {}
		~SObjectReferenceTracker() { DisposeOf(mObject); }

	// Properties
	public:
		T*	mObject;
};
