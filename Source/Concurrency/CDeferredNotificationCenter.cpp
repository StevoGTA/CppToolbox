//----------------------------------------------------------------------------------------------------------------------
//	CDeferredNotificationCenter.cpp			©2023 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDeferredNotificationCenter.h"

#include "TLockingArray.h"

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDeferredNotificationCenter::Internals

class CDeferredNotificationCenter::Internals {
	public:
		struct Info {
			Info(const CString& notificationName, const OI<Sender>& sender, const CDictionary& info) :
				mNotificationName(notificationName), mSender(sender), mInfo(info)
				{}
			Info(const Info& other) :
				mNotificationName(other.mNotificationName), mSender(other.mSender), mInfo(other.mInfo)
				{}

			CString		mNotificationName;
			OI<Sender>	mSender;
			CDictionary	mInfo;
		};

						Internals(CDeferredNotificationCenter& deferredNotificationCenter) :
							mDeferredNotificationCenter(deferredNotificationCenter), mMessageQueue(10 * 1024)
							{}

		static	void	flush(CSRSWMessageQueue::ProcMessage& message,
								CDeferredNotificationCenter* deferredNotificationCenter)
							{ deferredNotificationCenter->flush(); }

		CDeferredNotificationCenter&	mDeferredNotificationCenter;
		CSRSWMessageQueue				mMessageQueue;
		TNLockingArray<Info>			mInfos;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDeferredNotificationCenter

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CDeferredNotificationCenter::CDeferredNotificationCenter()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(*this);
}

//----------------------------------------------------------------------------------------------------------------------
CDeferredNotificationCenter::~CDeferredNotificationCenter()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CNotificationCenter methods

//----------------------------------------------------------------------------------------------------------------------
void CDeferredNotificationCenter::queue(const CString& notificationName, const OI<Sender>& sender,
		const CDictionary& info)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add
	mInternals->mInfos += Internals::Info(notificationName, sender, info);

	// Submit message
	mInternals->mMessageQueue.submit(
			CSRSWMessageQueue::ProcMessage((CSRSWMessageQueue::ProcMessage::Proc) Internals::flush, this));
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CSRSWMessageQueue& CDeferredNotificationCenter::getMessageQueue() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mMessageQueue;
}

//----------------------------------------------------------------------------------------------------------------------
void CDeferredNotificationCenter::flush()
//----------------------------------------------------------------------------------------------------------------------
{
	// Send all
	while (mInternals->mInfos.getCount() > 0) {
		// Pop first
		Internals::Info	info = mInternals->mInfos.popFirst();

		// Send
		send(info.mNotificationName, info.mSender, info.mInfo);
	}
}
