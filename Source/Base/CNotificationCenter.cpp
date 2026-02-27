//----------------------------------------------------------------------------------------------------------------------
//	CNotificationCenter.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CNotificationCenter.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CNotificationCenter::Internals

class CNotificationCenter::Internals {
	public:
		struct ObserverInfo {
			// Methods
			ObserverInfo(const Sender& sender, const Observer& observer) :
				mSender(sender.copy()), mObserver(observer)
				{}
			ObserverInfo(const Observer& observer) : mObserver(observer) {}
			ObserverInfo(const ObserverInfo& other) : mSender(other.mSender), mObserver(other.mObserver) {}

			// Properties
			OI<Sender>	mSender;
			Observer	mObserver;
		};

	public:
				Internals() {}

		void	registerObserver(const CString& notificationName, const Sender& sender, const Observer& observer)
					{
						// Get existing observer infos
						OR<TNArray<ObserverInfo> >	observerInfos = mInfo[notificationName];

						// Add
						if (observerInfos.hasReference())
							// Add to existing array
							(*observerInfos).add(ObserverInfo(sender, observer));
						else
							// First
							mInfo.set(notificationName, TNArray<ObserverInfo>(ObserverInfo(sender, observer)));
					}
		void	registerObserver(const CString& notificationName, const Observer& observer)
					{
						// Get existing observer infos
						OR<TNArray<ObserverInfo> >	observerInfos = mInfo[notificationName];

						// Add
						if (observerInfos.hasReference())
							// Add to existing array
							(*observerInfos).add(ObserverInfo(observer));
						else
							// First
							mInfo.set(notificationName, TNArray<ObserverInfo>(ObserverInfo(observer)));
					}
		void	unregisterObserver(const CString& notificationName, Observer::Ref observerRef)
					{
						// Get existing observer infos
						OR<TNArray<ObserverInfo> >	observerInfos = mInfo[notificationName];
						if (!observerInfos.hasReference())
							// No observers
							return;

						// Remove observers
						unregisterObserver(observerRef, *observerInfos);

						// Check if have any left
						if (observerInfos->isEmpty())
							// No more observers for this notification name
							mInfo.remove(notificationName);
					}
		void	unregisterObserver(Observer::Ref observerRef)
					{
						// Iterate all notification names
						TSet<CString>	keys = mInfo.getKeys();
						for (TSet<CString>::Iterator iterator = keys.getIterator(); iterator; iterator++) {
							// Get existing observer infos
							OR<TNArray<ObserverInfo> >	observerInfos = mInfo[*iterator];

							// Remove observers
							unregisterObserver(observerRef, *observerInfos);

							// Check if have any left
							if (observerInfos->isEmpty())
								// No more observers for this notification name
								mInfo.remove(*iterator);
						}
					}

	private:
		void	unregisterObserver(Observer::Ref observerRef, TNArray<ObserverInfo>& observerInfos)
					{
						// Iterate array
						for (CArray::ItemIndex i = observerInfos.getCount(); i > 0; i--) {
							// Check for match
							if (observerInfos[i - 1].mObserver.mRef == observerRef)
								// Match
								observerInfos.removeAtIndex(i - 1);
						}
					}

	public:
		TNDictionary<TNArray<ObserverInfo> >	mInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CNotificationCenter

// MARK: Lifcycle methods

//----------------------------------------------------------------------------------------------------------------------
CNotificationCenter::CNotificationCenter()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals();
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
void CNotificationCenter::registerObserver(const CString& notificationName, const Sender& sender,
		const Observer& observer)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add observer
	mInternals->registerObserver(notificationName, sender, observer);
}

//----------------------------------------------------------------------------------------------------------------------
void CNotificationCenter::registerObserver(const CString& notificationName, const Observer& observer)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add observer
	mInternals->registerObserver(notificationName, observer);
}

//----------------------------------------------------------------------------------------------------------------------
void CNotificationCenter::unregisterObserver(const CString& notificationName, Observer::Ref observerRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove observer
	mInternals->unregisterObserver(notificationName, observerRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CNotificationCenter::unregisterObserver(Observer::Ref observerRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove observer
	mInternals->unregisterObserver(observerRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CNotificationCenter::send(const CString& notificationName, const OR<Sender>& sender, const CDictionary& info) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	OR<TNArray<Internals::ObserverInfo> >	observerInfos = mInternals->mInfo[notificationName];
	if (!observerInfos.hasReference())
		// No observers
		return;

	// Iterate observer infos
	for (TArray<Internals::ObserverInfo>::Iterator iterator = observerInfos->getIterator(); iterator; iterator++) {
		// Get info
		const	Internals::ObserverInfo&	observerInfo = *iterator;

		// Check sender
		if (!observerInfo.mSender.hasInstance() || !sender.hasReference() || (*observerInfo.mSender == *sender))
			// Call proc
			observerInfo.mObserver.callProc(notificationName, sender, info);
	}
}
