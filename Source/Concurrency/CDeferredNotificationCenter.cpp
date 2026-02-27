//----------------------------------------------------------------------------------------------------------------------
//	CDeferredNotificationCenter.cpp			©2023 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDeferredNotificationCenter.h"

#include "ConcurrencyPrimitives.h"
#include "CThread.h"

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
	#include <dispatch/dispatch.h>
#elif defined(TARGET_OS_WINDOWS)
	#include "winrt\Microsoft.UI.Dispatching.h"

	using namespace winrt::Microsoft::UI::Dispatching;
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

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
				Internals(CDeferredNotificationCenter& deferredNotificationCenter) :
#elif defined(TARGET_OS_WINDOWS)
				Internals(CDeferredNotificationCenter& deferredNotificationCenter,
						const DispatcherQueue& dispatcherQueue) :
#endif
					mQueueArmed(false), mDeferredNotificationCenter(deferredNotificationCenter),
							mUIThreadRef(CThread::getCurrentRef())
#if defined(TARGET_OS_WINDOWS)
							, mDispatcherQueue(dispatcherQueue)
#endif
					{ mActiveInternals += this; }
				~Internals()
					{ mActiveInternals -= this; }

		void	post(const Info& info)
					{
						// Check if on UI thread
						if (CThread::getCurrentRef() == mUIThreadRef)
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
								mDispatcherQueue.TryEnqueue([this]() {
									// Check if active
									if (mActiveInternals.contains(this))
										// Send notifications
										send();
								});
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
						for (TArray<Info>::Iterator iterator = mInfos.getIterator(); iterator; iterator++)
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
				CThread::Ref					mUIThreadRef;
				TNArray<Info>					mInfos;

#if defined(TARGET_OS_WINDOWS)
				DispatcherQueue					mDispatcherQueue;
#endif

		static	TNumberArray<void*>				mActiveInternals;
};

TNumberArray<void*>	CDeferredNotificationCenter::Internals::mActiveInternals;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDeferredNotificationCenter

// MARK: Lifecycle methods

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
//----------------------------------------------------------------------------------------------------------------------
CDeferredNotificationCenter::CDeferredNotificationCenter()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(*this);
}
#elif defined(TARGET_OS_WINDOWS)
//----------------------------------------------------------------------------------------------------------------------
CDeferredNotificationCenter::CDeferredNotificationCenter(const DispatcherQueue& dispatcherQueue)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(*this, dispatcherQueue);
}
#endif

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
