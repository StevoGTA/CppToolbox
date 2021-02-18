//----------------------------------------------------------------------------------------------------------------------
//	CNotificationCenter.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CNotificationCenter.h"

#include "TLockingArray.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SNotificationObserverFullInfo

struct SNotificationObserverFullInfo {
			// Lifecycle methods
			SNotificationObserverFullInfo(const void* senderRef,
					const CNotificationCenter::ObserverInfo& observerInfo) :
				mSenderRef(senderRef), mObserverInfo(observerInfo)
				{}
			SNotificationObserverFullInfo(const SNotificationObserverFullInfo& other) :
				mSenderRef(other.mSenderRef), mObserverInfo(other.mObserverInfo)
				{}

	// Properties
	const	void*								mSenderRef;
			CNotificationCenter::ObserverInfo	mObserverInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CNotificationCenterInternals

class CNotificationCenterInternals {
	public:
				CNotificationCenterInternals() {}

		void	registerObserver(const CString& notificationName, const void* senderRef,
						const CNotificationCenter::ObserverInfo& observerInfo)
					{
						// Setup
						SNotificationObserverFullInfo	notificationObserverFullInfo(senderRef, observerInfo);

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
						for (CArray::ItemIndex i = notificationObserverFullInfos.getCount(); i > 0; i--) {
							// Check for match
							if (notificationObserverFullInfos[i - 1].mObserverInfo.mObserverRef == observerRef)
								// Match
								notificationObserverFullInfos.removeAtIndex(i - 1);
						}
					}

	public:
		TNDictionary<TNArray<SNotificationObserverFullInfo> >	mInfo;
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
		const ObserverInfo& observerInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add observer
	mInternals->registerObserver(notificationName, senderRef, observerInfo);
}

//----------------------------------------------------------------------------------------------------------------------
void CNotificationCenter::registerObserver(const CString& notificationName, const ObserverInfo& observerInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add observer
	mInternals->registerObserver(notificationName, nil, observerInfo);
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
		notificationObserverFullInfo.mObserverInfo.callProc(notificationName, senderRef, info);
	}
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDeferredNotificationCenterInternals

class CDeferredNotificationCenterInternals {
	public:
		struct Info {
			Info(const CString& notificationName, const void* senderRef, const CDictionary& info) :
				mNotificationName(notificationName), mSenderRef(senderRef), mInfo(info)
				{}
			Info(const Info& other) :
				mNotificationName(other.mNotificationName), mSenderRef(other.mSenderRef), mInfo(other.mInfo)
				{}

					CString		mNotificationName;
			const	void*		mSenderRef;
					CDictionary	mInfo;
		};

		CDeferredNotificationCenterInternals() {}

		TNLockingArray<Info>	mInfos;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDeferredNotificationCenter

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CDeferredNotificationCenter::CDeferredNotificationCenter()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CDeferredNotificationCenterInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CDeferredNotificationCenter::~CDeferredNotificationCenter()
//----------------------------------------------------------------------------------------------------------------------
{
	// Make sure to flush
	flush();

	// Cleanup
	Delete(mInternals);
}

// MARK: CNotificationCenter methods

//----------------------------------------------------------------------------------------------------------------------
void CDeferredNotificationCenter::queue(const CString& notificationName, const void* senderRef, const CDictionary& info)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add
	mInternals->mInfos += CDeferredNotificationCenterInternals::Info(notificationName, senderRef, info);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CDeferredNotificationCenter::flush()
//----------------------------------------------------------------------------------------------------------------------
{
	// Send all
	while (mInternals->mInfos.getCount() > 0) {
		// Pop first
		CDeferredNotificationCenterInternals::Info	info = mInternals->mInfos.popFirst();

		// Send
		send(info.mNotificationName, info.mSenderRef, info.mInfo);
	}
}
