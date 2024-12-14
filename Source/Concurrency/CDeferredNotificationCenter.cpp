//----------------------------------------------------------------------------------------------------------------------
//	CDeferredNotificationCenter.cpp			©2023 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDeferredNotificationCenter.h"

#include "ConcurrencyPrimitives.h"

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
	#include "CThread.h"

	#include <dispatch/dispatch.h>
#elif defined(TARGET_OS_WINDOWS)
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDeferredNotificationCenter::Internals

class CDeferredNotificationCenter::Internals {
	public:
		struct Info {
			Info(const CString& notificationName, const Sender& sender, const CDictionary& info) :
				mNotificationName(notificationName), mSender(sender.copy()), mInfo(info)
				{}
			Info(const CString& notificationName, const CDictionary& info) :
				mNotificationName(notificationName), mInfo(info)
				{}
			Info(const Info& other) :
				mNotificationName(other.mNotificationName), mSender(other.mSender), mInfo(other.mInfo)
				{}

			CString		mNotificationName;
			OI<Sender>	mSender;
			CDictionary	mInfo;
		};

				Internals(CDeferredNotificationCenter& deferredNotificationCenter) :
					mQueueArmed(false), mDeferredNotificationCenter(deferredNotificationCenter)
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
							, mMainThreadRef(CThread::getCurrentRef())
#elif defined(TARGET_OS_WINDOWS)
#endif
					{ mActiveInternals += this; }
				~Internals()
					{ mActiveInternals -= this; }

		void	post(const Info& info)
					{
						// Check if on UI thread
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
						if (CThread::getCurrentRef() == mMainThreadRef)
#elif defined(TARGET_OS_WINDOWS)
#endif
							// On UI thread
							send(info);
						else {
							// Lock
							mLock.lock();

							// Add
							mInfos += info;

							// Check if queue is armed
							if (!mQueueArmed) {
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
								dispatch_async(dispatch_get_main_queue(), ^{
									// Check if active
									if (mActiveInternals.contains(this))
										// Send notifications
										send();
								});
#elif defined(TARGET_OS_WINDOWS)
#endif
								// Queue is now armed
								mQueueArmed = true;
							}

							// Unlock
							mLock.unlock();
						}
					}
		void	send()
					{
						// Lock
						mLock.lock();

						// Send notifications
						for (TIteratorD<Info> iterator = mInfos.getIterator(); iterator.hasValue();
								iterator.advance())
							// Send
							send(*iterator);
						mInfos.removeAll();

						// Queue is no longer armed
						mQueueArmed = false;

						// Unlock
						mLock.unlock();
					}
		void	send(const Info& info) const
					{
						// Check for sender
						if (info.mSender.hasInstance())
							// Have sender
							mDeferredNotificationCenter.send(info.mNotificationName, OR<Sender>(*info.mSender),
									info.mInfo);
						else
							// Don't have sender
							mDeferredNotificationCenter.send(info.mNotificationName, OR<Sender>(), info.mInfo);
					}

				bool							mQueueArmed;
				CDeferredNotificationCenter&	mDeferredNotificationCenter;
				CLock							mLock;
				CThread::Ref					mMainThreadRef;
				TNArray<Info>					mInfos;

		static	TNumberArray<void*>				mActiveInternals;

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
#elif defined(TARGET_OS_WINDOWS)
#endif
};

TNumberArray<void*>	CDeferredNotificationCenter::Internals::mActiveInternals;

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
void CDeferredNotificationCenter::post(const CString& notificationName, const Sender& sender, const CDictionary& info)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->post(Internals::Info(notificationName, sender, info));
}

//----------------------------------------------------------------------------------------------------------------------
void CDeferredNotificationCenter::post(const CString& notificationName, const CDictionary& info)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->post(Internals::Info(notificationName, info));
}
