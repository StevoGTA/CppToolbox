//----------------------------------------------------------------------------------------------------------------------
//	CNotificationCenter.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CNotificationCenter.h"

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
				~CNotificationCenterInternals() {}

		void	registerObserver(const CString& notificationName, const void* senderRef,
						const SNotificationObserverInfo& notificationObserverInfo)
					{
						// Setup
						SNotificationObserverFullInfo*	notificationObserverFullInfo =
																new SNotificationObserverFullInfo(senderRef,
																		notificationObserverInfo);

						// Get existing observer infos
						TPtrArray<SNotificationObserverFullInfo*>*	array = mInfo[notificationName];

						// Add
						if (array != nil)
							// Add to existing set
							array->add(notificationObserverFullInfo);
						else
							// First
							mInfo.set(notificationName,
									new TPtrArray<SNotificationObserverFullInfo*>(notificationObserverFullInfo, true));
					}
		void	unregisterObserver(const CString& notificationName, const void* observerRef)
					{
						// Get existing observer infos
						TPtrArray<SNotificationObserverFullInfo*>*	notificationObserverFullInfos =
																			mInfo[notificationName];
						if (notificationObserverFullInfos == nil)
							// No observers
							return;

						// Remove observers
						unregisterObserver(observerRef, notificationObserverFullInfos);

						// Check if have any left
						if (notificationObserverFullInfos->isEmpty())
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
							CString&							notificationName = iterator.getValue();
							TPtrArray<SNotificationObserverFullInfo*>*	notificationObserverFullInfos =
																				mInfo[notificationName];

							// Remove observers
							unregisterObserver(observerRef, notificationObserverFullInfos);

							// Check if have any left
							if (notificationObserverFullInfos->isEmpty())
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

	public:
		TDictionary<TPtrArray<SNotificationObserverFullInfo*>*>	mInfo;
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

//----------------------------------------------------------------------------------------------------------------------
void CNotificationCenter::unregisterObserver(UInt32 observerReference)
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove observer
	mInternals->unregisterObserver(observerReference);
}

//----------------------------------------------------------------------------------------------------------------------
void CNotificationCenter::send(const CString& notificationName, const void* senderRef, const CDictionary& info) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TPtrArray<SNotificationObserverFullInfo*>*	array = mInternals->mInfo[notificationName];
	if (array == nil)
		// No observers
		return;

		// Iterate observer infos
	for (TIteratorS<SNotificationObserverFullInfo*> iterator = array->getIterator(); iterator.hasValue();
			iterator.advance()) {
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
//		const CDictionary& info)
////----------------------------------------------------------------------------------------------------------------------
//{
//}
