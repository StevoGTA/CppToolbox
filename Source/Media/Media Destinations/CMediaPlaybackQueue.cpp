//----------------------------------------------------------------------------------------------------------------------
//	CMediaPlaybackQueue.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMediaPlaybackQueue.h"

#include "ConcurrencyPrimitives.h"
#include "CThread.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaPlaybackQueueThread

class CMediaPlaybackQueueThread : public CThread {
	public:
		struct ItemPreparedMessage : public CSRSWMessageQueue::ProcMessage {
					// Lifecycle Methods
					ItemPreparedMessage(Proc proc, void* userData, const TIResult<I<CMediaPlayer> >& mediaPlayer) :
						CSRSWMessageQueue::ProcMessage(sizeof(ItemPreparedMessage), proc, userData),
								mMediaPlayer(new TIResult<I<CMediaPlayer> >(mediaPlayer))
						{}

					// Instance methods
			void	cleanup()
						{ Delete(mMediaPlayer); }

			// Properties
			TIResult<I<CMediaPlayer> >*	mMediaPlayer;
		};

						CMediaPlaybackQueueThread(CSRSWMessageQueues& messageQueues, const CMediaPlaybackQueue::Info& info) :
							CThread(CString(OSSTR("CMediaPlaybackQueueThread"))),
									mInfo(info), mMessageQueue(1024),
									mShutdownRequested(false),
									mCurrentItemPrepareCancelled(false)
							{
								// Setup
								messageQueues.add(mMessageQueue);
							}

				void	run()
							{
								// Run until shutdown
								while (!mShutdownRequested) {
									// Wait
									mSemaphore.waitFor();

									// Get current item
									mCurrentItemLock.lock();
									OR<I<CMediaPlaybackQueue::Item> >	item = mCurrentItem;
									mCurrentItemLock.unlock();

									// Check for current item
									if (mCurrentItem.hasReference()) {
										// Prepare
										TIResult<I<CMediaPlayer> >	mediaPlayer = (*mCurrentItem)->prepare();
										if (!mCurrentItemPrepareCancelled) {
											// Queue message
											mMessageQueue.submit(ItemPreparedMessage(handleItemPrepared, this,
													mediaPlayer));
										}

										// Clear current item
										mCurrentItemLock.lock();
										mCurrentItem = OR<I<CMediaPlaybackQueue::Item> >();
										mCurrentItemLock.unlock();
									}
								}
							}

				void	prepare(I<CMediaPlaybackQueue::Item>& item)
							{
								// Store
								mCurrentItemLock.lock();
								mCurrentItem = OR<I<CMediaPlaybackQueue::Item> >(item);
								mCurrentItemPrepareCancelled = false;
								mCurrentItemLock.unlock();

								// Signal
								mSemaphore.signal();
							}
				void	cancelInFlightPrepare()
							{
								// Setup
								bool	currentItemHasReference = false;

								// Cancel current item if there is one
								mCurrentItemLock.lock();
								if (mCurrentItem.hasReference()) {
									// Cancel
									mCurrentItemPrepareCancelled = true;
									(*mCurrentItem)->cancel();

									currentItemHasReference = true;
								}
								mCurrentItemLock.unlock();

								// Check if need to wait
								while (currentItemHasReference) {
									// Sleep
									CThread::sleepFor(0.001);

									// Check current item
									mCurrentItemLock.lock();
									currentItemHasReference = mCurrentItem.hasReference();
									mCurrentItemLock.unlock();
								}
							}
				void	shutdown()
							{
								// Update
								mShutdownRequested = true;
								mMessageQueue.handleAll();

								// Signal
								mSemaphore.signal();
							}
		static	void	handleItemPrepared(CSRSWMessageQueue::ProcMessage& message, void* userData)
							{
								// Setup
								CMediaPlaybackQueueThread&	internals = *((CMediaPlaybackQueueThread*) userData);
								ItemPreparedMessage&		itemPreparedMessage = (ItemPreparedMessage&) message;

								// Check if shutdown requested
								if (!internals.mShutdownRequested)
									// Inform
									internals.mInfo.itemPrepareCompleted(*itemPreparedMessage.mMediaPlayer);

								// Cleanup
								itemPreparedMessage.cleanup();
							}

	private:
		CMediaPlaybackQueue::Info			mInfo;
		CSemaphore							mSemaphore;
		CSRSWMessageQueue					mMessageQueue;

		bool								mShutdownRequested;

		OR<I<CMediaPlaybackQueue::Item> >	mCurrentItem;
		bool								mCurrentItemPrepareCancelled;
		CLock								mCurrentItemLock;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaPlaybackQueueInternals

class CMediaPlaybackQueueInternals {
	public:
		CMediaPlaybackQueueInternals(CSRSWMessageQueues& messageQueues, const CMediaPlaybackQueue::Info& info) :
			mThread(messageQueues, info),
					mCurrentItemIndex(0)
			{}
		~CMediaPlaybackQueueInternals()
			{
				// Request shutdown
				mThread.shutdown();

				// Wait until is no lonnger running
				while (mThread.getIsRunning())
					// Sleep
					CThread::sleepFor(0.001);
			}

		CMediaPlaybackQueueThread				mThread;

		TNArray<I<CMediaPlaybackQueue::Item> >	mItems;
		CArray::ItemIndex						mCurrentItemIndex;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaPlaybackQueue

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CMediaPlaybackQueue::CMediaPlaybackQueue(CSRSWMessageQueues& messageQueues, const Info& info)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CMediaPlaybackQueueInternals(messageQueues, info);
}

//----------------------------------------------------------------------------------------------------------------------
CMediaPlaybackQueue::~CMediaPlaybackQueue()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlaybackQueue::set(const TArray<I<Item> >& items)
//----------------------------------------------------------------------------------------------------------------------
{
	// Cancel in-flight prepare
	mInternals->mThread.cancelInFlightPrepare();

	// Store
	mInternals->mItems = items;
	mInternals->mCurrentItemIndex = 0;

	// Check for items
	if (!mInternals->mItems.isEmpty())
		// Prepare
		mInternals->mThread.prepare(mInternals->mItems[0]);
}
