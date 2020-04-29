//----------------------------------------------------------------------------------------------------------------------
//	CNotificationCenter.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CNotificationCenter.h"

////----------------------------------------------------------------------------------------------------------------------
//// MARK: SNotificationObserverInfo
//
//struct SNotificationObserverInfo {
//			// Lifecycle methods
//			SNotificationObserverInfo(const void* observerRef, NotificationProc proc, void* userData) :
//				mIsObserverRef(true), mProc(proc), mUserData(userData)
//				{
//					mObserverInfo.mObserverRef = observerRef;
//				}
//			SNotificationObserverInfo(UInt32 observerIndex, NotificationProc proc, void* userData) :
//				mIsObserverRef(false), mProc(proc), mUserData(userData)
//				{
//					mObserverInfo.mObserverIndex = observerIndex;
//				}
//
//			// Instance methods
//	void	callProc(const CString& notificationName, const void* senderRef, const CDictionary& info)
//				{ mProc(notificationName, senderRef, info, mUserData); }
//
//	// Properties
//	bool					mIsObserverRef;
//	union {
//		const	void*			mObserverRef;
//				UInt32			mObserverIndex;
//	}						mObserverInfo;
//	NotificationProc		mProc;
//	void*					mUserData;
//};

//----------------------------------------------------------------------------------------------------------------------
// MARK: SNotificationObserverFullInfo

struct SNotificationObserverFullInfo {
			// Lifecycle methods
			SNotificationObserverFullInfo(const void* senderRef,
					const SNotificationObserverInfo& notificationObserverInfo) :
				mSenderRef(senderRef), mNotificationObserverInfo(notificationObserverInfo)
				{}

	// Properties
	const	void*						mSenderRef;
			SNotificationObserverInfo	mNotificationObserverInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CNotificationCenterInternals

class CNotificationCenterInternals {
	public:
				CNotificationCenterInternals() {}
// : mNextObserverIndex(0) {}
				~CNotificationCenterInternals() {}

		void	registerObserver(const CString& notificationName, const void* senderRef,
						const SNotificationObserverInfo& notificationObserverInfo)
					{
						// Setup
						SNotificationObserverFullInfo*	notificationObserverFullInfo =
																new SNotificationObserverFullInfo(senderRef,
																		notificationObserverInfo);

						// Get existing observer infos
						OV<TPtrArray<SNotificationObserverFullInfo*>*>	notificationObserverFullInfos =
																				mInfo[notificationName];

						// Add
						if (notificationObserverFullInfos.hasValue())
							// Add to existing set
							(*notificationObserverFullInfos)->add(notificationObserverFullInfo);
						else
							// First
							mInfo.set(notificationName,
									new TPtrArray<SNotificationObserverFullInfo*>(notificationObserverFullInfo, true));
					}
		void	unregisterObserver(const CString& notificationName, const void* observerRef)
					{
						// Get existing observer infos
						OV<TPtrArray<SNotificationObserverFullInfo*>*>	notificationObserverFullInfos =
																				mInfo[notificationName];
						if (!notificationObserverFullInfos.hasValue())
							// No observers
							return;

						// Remove observers
						unregisterObserver(observerRef, *notificationObserverFullInfos);

						// Check if have any left
						if ((*notificationObserverFullInfos)->isEmpty())
							// No more observers for this notification name
							mInfo.remove(notificationName);
					}
		void	unregisterObserver(const void* observerRef)
					{
						// Iterate all notification names
						TSet<CString>	keys = mInfo.getKeys();
						for (TIteratorS<CString> iterator = keys.getIterator(); iterator.hasValue();
								iterator.advance()) {
							// Get existing observer infos
							CString&										notificationName = iterator.getValue();
							OV<TPtrArray<SNotificationObserverFullInfo*>*>	notificationObserverFullInfos =
																					mInfo[notificationName];

							// Remove observers
							unregisterObserver(observerRef, *notificationObserverFullInfos);

							// Check if have any left
							if ((*notificationObserverFullInfos)->isEmpty())
								// No more observers for this notification name
								mInfo.remove(notificationName);
						}
					}

	private:
		void	unregisterObserver(const void* observerRef,
						TPtrArray<SNotificationObserverFullInfo*>* notificationObserverFullInfos)
					{
						// Iterate array
						for (CArrayItemIndex i = notificationObserverFullInfos->getCount(); i > 0; i--) {
							// Get info
							SNotificationObserverFullInfo*	notificationObserverFullInfo =
																	(*notificationObserverFullInfos)[i - 1];

							// Check for match
							if (notificationObserverFullInfo->mNotificationObserverInfo.mObserverRef == observerRef)
								// Match
								notificationObserverFullInfos->removeAtIndex(i - 1);
						}
					}
//		void	unregisterObserver(UInt32 observerReference)
//					{
//						// Iterate all notification names
//						bool	found = false;
//						for (TIteratorS<CString> iterator = mInfo.getKeys().getIterator(); iterator.hasValue();
//								iterator.advance()) {
//							// Get existing observer infos
//							CString&							notificationName = iterator.getValue();
//							TPtrArray<SNotificationObserverInfo*>*	array = mInfo[notificationName];
//							for (CArrayItemIndex i = array->getCount(); !found && (i > 0); i--) {
//								// Get info
//								SNotificationObserverInfo*	observerInfo = (*array)[i - 1];
//
//								// Check for match
//								if (!observerInfo->mIsObserverRef &&
//										(observerInfo->mObserverInfo.mObserverIndex == observerReference)) {
//									// Match
//									array->removeAtIndex(i - 1);
//									found = true;
//								}
//							}
//
//							// Check for empty array
//							if (array->getCount() == 0)
//								// Remove
//								mInfo.remove(notificationName);
//
//							// Check found
//							if (found)
//								return;
//						}
//					}

	public:
		TDictionary<TPtrArray<SNotificationObserverFullInfo*>*>	mInfo;
//		UInt32													mNextObserverIndex;
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
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CNotificationCenter::registerObserver(const CString& notificationName, const void* senderRef,
		const SNotificationObserverInfo& notificationObserverInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add observer
	mInternals->registerObserver(notificationName, senderRef, notificationObserverInfo);
}

//----------------------------------------------------------------------------------------------------------------------
void CNotificationCenter::registerObserver(const CString& notificationName,
		const SNotificationObserverInfo& notificationObserverInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add observer
	mInternals->registerObserver(notificationName, nil, notificationObserverInfo);
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

////----------------------------------------------------------------------------------------------------------------------
//void CNotificationCenter::unregisterObserver(UInt32 observerReference)
////----------------------------------------------------------------------------------------------------------------------
//{
//	// Remove observer
//	mInternals->unregisterObserver(observerReference);
//}

//----------------------------------------------------------------------------------------------------------------------
void CNotificationCenter::send(const CString& notificationName, const void* senderRef, const CDictionary& info) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	OV<TPtrArray<SNotificationObserverFullInfo*>*>	notificationObserverFullInfos = mInternals->mInfo[notificationName];
	if (!notificationObserverFullInfos.hasValue())
		// No observers
		return;

	// Iterate observer infos
	for (TIteratorS<SNotificationObserverFullInfo*> iterator = (*notificationObserverFullInfos)->getIterator();
			iterator.hasValue(); iterator.advance()) {
		// Get info
		SNotificationObserverFullInfo*	notificationObserverFullInfo = iterator.getValue();

		// Check sender
		if ((notificationObserverFullInfo->mSenderRef != nil) &&
				(notificationObserverFullInfo->mSenderRef != senderRef))
			// Sender does not match
			return;

		// Call proc
		notificationObserverFullInfo->mNotificationObserverInfo.callProc(notificationName, senderRef, info);
	}
}

////----------------------------------------------------------------------------------------------------------------------
//void CNotificationCenter::postOnMainThread(const CString& notificationName, const void* senderRef,
//		const CDictionary& info) const
////----------------------------------------------------------------------------------------------------------------------
//{
//}
