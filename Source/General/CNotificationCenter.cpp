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
			SNotificationObserverFullInfo(const SNotificationObserverFullInfo& other) :
				mSenderRef(other.mSenderRef), mNotificationObserverInfo(other.mNotificationObserverInfo)
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

		void	registerObserver(const CString& notificationName, const void* senderRef,
						const SNotificationObserverInfo& notificationObserverInfo)
					{
						// Setup
						SNotificationObserverFullInfo	notificationObserverFullInfo(senderRef,
																notificationObserverInfo);

						// Get existing observer infos
						OR<TNArray<SNotificationObserverFullInfo> >	notificationObserverFullInfos =
																			mInfo[notificationName];

						// Add
						if (notificationObserverFullInfos.hasReference())
							// Add to existing array
							(*notificationObserverFullInfos).add(notificationObserverFullInfo);
						else
							// First
							mInfo.set(notificationName,
									TNArray<SNotificationObserverFullInfo>(notificationObserverFullInfo));
					}
		void	unregisterObserver(const CString& notificationName, const void* observerRef)
					{
						// Get existing observer infos
						OR<TNArray<SNotificationObserverFullInfo> >	notificationObserverFullInfos =
																			mInfo[notificationName];
						if (!notificationObserverFullInfos.hasReference())
							// No observers
							return;

						// Remove observers
						unregisterObserver(observerRef, *notificationObserverFullInfos);

						// Check if have any left
						if ((*notificationObserverFullInfos).isEmpty())
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
							CString&									notificationName = iterator.getValue();
							OR<TNArray<SNotificationObserverFullInfo> >	notificationObserverFullInfos =
																				mInfo[notificationName];

							// Remove observers
							unregisterObserver(observerRef, *notificationObserverFullInfos);

							// Check if have any left
							if ((*notificationObserverFullInfos).isEmpty())
								// No more observers for this notification name
								mInfo.remove(notificationName);
						}
					}

	private:
		void	unregisterObserver(const void* observerRef,
						TNArray<SNotificationObserverFullInfo>& notificationObserverFullInfos)
					{
						// Iterate array
						for (CArrayItemIndex i = notificationObserverFullInfos.getCount(); i > 0; i--) {
							// Check for match
							if (notificationObserverFullInfos[i - 1].mNotificationObserverInfo.mObserverRef ==
									observerRef)
								// Match
								notificationObserverFullInfos.removeAtIndex(i - 1);
						}
					}

	public:
		TDictionary<TNArray<SNotificationObserverFullInfo> >	mInfo;
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

//----------------------------------------------------------------------------------------------------------------------
void CNotificationCenter::send(const CString& notificationName, const void* senderRef, const CDictionary& info) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	OR<TNArray<SNotificationObserverFullInfo> >	notificationObserverFullInfos = mInternals->mInfo[notificationName];
	if (!notificationObserverFullInfos.hasReference())
		// No observers
		return;

	// Iterate observer infos
	for (TIteratorD<SNotificationObserverFullInfo> iterator = notificationObserverFullInfos->getIterator();
			iterator.hasValue(); iterator.advance()) {
		// Get info
		const	SNotificationObserverFullInfo&	notificationObserverFullInfo = iterator.getValue();

		// Check sender
		if ((notificationObserverFullInfo.mSenderRef != nil) && (notificationObserverFullInfo.mSenderRef != senderRef))
			// Sender does not match
			return;

		// Call proc
		notificationObserverFullInfo.mNotificationObserverInfo.callProc(notificationName, senderRef, info);
	}
}

////----------------------------------------------------------------------------------------------------------------------
//void CNotificationCenter::postOnMainThread(const CString& notificationName, const void* senderRef,
//		const CDictionary& info) const
////----------------------------------------------------------------------------------------------------------------------
//{
//}
