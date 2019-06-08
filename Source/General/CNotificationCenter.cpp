//----------------------------------------------------------------------------------------------------------------------
//	CNotificationCenter.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CNotificationCenter.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SNotificationObserverInfo

struct SNotificationObserverInfo {
			// Lifecycle methods
			SNotificationObserverInfo(const void* observerRef, NotificationProc proc, void* userData) :
				mIsObserverRef(true), mProc(proc), mUserData(userData)
				{
					mObserverInfo.mObserverRef = observerRef;
				}
			SNotificationObserverInfo(UInt32 observerIndex, NotificationProc proc, void* userData) :
				mIsObserverRef(false), mProc(proc), mUserData(userData)
				{
					mObserverInfo.mObserverIndex = observerIndex;
				}

			// Instance methods
	void	callProc(const CString& notificationName, const void* senderRef, const CDictionary& info)
				{ mProc(notificationName, senderRef, info, mUserData); }

	// Properties
	bool					mIsObserverRef;
	union {
		const	void*			mObserverRef;
				UInt32			mObserverIndex;
	}						mObserverInfo;
	NotificationProc		mProc;
	void*					mUserData;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CNotificationCenterInternals

class CNotificationCenterInternals {
	public:
				CNotificationCenterInternals() : mNextObserverIndex(0) {}
				~CNotificationCenterInternals() {}

		void	registerObserver(const CString& notificationName, SNotificationObserverInfo* observerInfo)
					{
						// Get existing observer infos
						TPtrArray<SNotificationObserverInfo*>*	array = mInfo[notificationName];

						// Add
						if (array != nil)
							// Add to existing set
							array->add(observerInfo);
						else
							// First
							mInfo.set(notificationName, new TPtrArray<SNotificationObserverInfo*>(observerInfo, true));
					}
		void	unregisterObserver(const CString& notificationName, const void* observerRef)
					{
						// Get existing observer infos
						TPtrArray<SNotificationObserverInfo*>*	array = mInfo[notificationName];

						// Check if have any observer infos
						if (array != nil) {
							// Iterate array
							for (CArrayItemIndex i = 0; i < array->getCount(); i++) {
								// Get info
								SNotificationObserverInfo*	observerInfo = (*array)[i];

								// Check for match
								if (observerInfo->mIsObserverRef &&
										(observerInfo->mObserverInfo.mObserverRef == observerRef)) {
									// Match
									array->removeAtIndex(i);

									// Check for empty array
									if (array->getCount() == 0)
										// Remove
										mInfo.remove(notificationName);

									return;
								}
							}
						}
					}
		void	unregisterObserver(const void* observerRef)
					{
						// Iterate all notification names
						for (TIteratorS<CString> iterator = mInfo.getKeys().getIterator(); iterator.hasValue();
								iterator.advance()) {
							// Get existing observer infos
							CString&							notificationName = iterator.getValue();
							TPtrArray<SNotificationObserverInfo*>*	array = mInfo[notificationName];
							for (CArrayItemIndex i = array->getCount(); i > 0; i--) {
								// Get info
								SNotificationObserverInfo*	observerInfo = (*array)[i - 1];

								// Check for match
								if (observerInfo->mIsObserverRef &&
										(observerInfo->mObserverInfo.mObserverRef == observerRef))
									// Match
									array->removeAtIndex(i - 1);
							}

							// Check for empty array
							if (array->getCount() == 0)
								// Remove
								mInfo.remove(notificationName);
						}
					}
		void	unregisterObserver(UInt32 observerReference)
					{
						// Iterate all notification names
						bool	found = false;
						for (TIteratorS<CString> iterator = mInfo.getKeys().getIterator(); iterator.hasValue();
								iterator.advance()) {
							// Get existing observer infos
							CString&							notificationName = iterator.getValue();
							TPtrArray<SNotificationObserverInfo*>*	array = mInfo[notificationName];
							for (CArrayItemIndex i = array->getCount(); !found && (i > 0); i--) {
								// Get info
								SNotificationObserverInfo*	observerInfo = (*array)[i - 1];

								// Check for match
								if (!observerInfo->mIsObserverRef &&
										(observerInfo->mObserverInfo.mObserverIndex == observerReference)) {
									// Match
									array->removeAtIndex(i - 1);
									found = true;
								}
							}

							// Check for empty array
							if (array->getCount() == 0)
								// Remove
								mInfo.remove(notificationName);

							// Check found
							if (found)
								return;
						}
					}

		TDictionary<TPtrArray<SNotificationObserverInfo*>*>	mInfo;
		UInt32												mNextObserverIndex;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CNotificationCenter

CNotificationCenter	CNotificationCenter::mStandard;

// MARK: Lifcycle methods

//----------------------------------------------------------------------------------------------------------------------
CNotificationCenter::CNotificationCenter()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CNotificationCenterInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CNotificationCenter::~CNotificationCenter()
//----------------------------------------------------------------------------------------------------------------------
{
	// Cleanup
	DisposeOf(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CNotificationCenter::registerObserver(const CString& notificationName, const void* observerRef,
		NotificationProc proc, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add observer
	mInternals->registerObserver(notificationName, new SNotificationObserverInfo(observerRef, proc, userData));
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CNotificationCenter::registerObserver(const CString& notificationName, NotificationProc proc, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add observer
	UInt32	observerIndex = mInternals->mNextObserverIndex++;
	mInternals->registerObserver(notificationName, new SNotificationObserverInfo(observerIndex, proc, userData));

	return observerIndex;
}

//----------------------------------------------------------------------------------------------------------------------
void CNotificationCenter::unregisterObserver(const CString& notificationName, const void* observerRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove observer
	mInternals->unregisterObserver(notificationName, observerRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CNotificationCenter::unregisterObserver(const void* observerRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove observer
	mInternals->unregisterObserver(observerRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CNotificationCenter::unregisterObserver(UInt32 observerReference)
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove observer
	mInternals->unregisterObserver(observerReference);
}

//----------------------------------------------------------------------------------------------------------------------
void CNotificationCenter::send(const CString& notificationName, const void* senderRef, const CDictionary& info)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TPtrArray<SNotificationObserverInfo*>*	array = mInternals->mInfo[notificationName];

	// Do we have observer infos?
	if (array != nil) {
		// Iterate observer infos
		for (TIteratorS<SNotificationObserverInfo*> iterator = array->getIterator(); iterator.hasValue();
				iterator.advance())
			// Call proc
			iterator.getValue()->callProc(notificationName, senderRef, info);
	}
}

////----------------------------------------------------------------------------------------------------------------------
//void CNotificationCenter::postOnMainThread(const CString& notificationName, const void* senderRef,
//		const CDictionary& info)
////----------------------------------------------------------------------------------------------------------------------
//{
//}
