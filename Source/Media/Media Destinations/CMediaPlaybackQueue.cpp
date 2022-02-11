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
		struct ItemPrepareStartedMessage : public CSRSWMessageQueue::ProcMessage {
					// Lifecycle Methods
					ItemPrepareStartedMessage(Proc proc, void* userData, const I<CMediaPlaybackQueue::Item>& item) :
						CSRSWMessageQueue::ProcMessage(sizeof(ItemPrepareStartedMessage), proc, userData),
								mItem(new I<CMediaPlaybackQueue::Item>(item))
						{}

					// Instance methods
			void	cleanup()
						{ Delete(mItem); }

			// Properties
			I<CMediaPlaybackQueue::Item>*	mItem;
		};
		struct ItemPrepareCompletedMessage : public CSRSWMessageQueue::ProcMessage {
					// Lifecycle Methods
					ItemPrepareCompletedMessage(Proc proc, void* userData, const I<CMediaPlaybackQueue::Item>& item,
							const TIResult<I<CMediaPlayer> >& mediaPlayer) :
						CSRSWMessageQueue::ProcMessage(sizeof(ItemPrepareCompletedMessage), proc, userData),
								mItem(new I<CMediaPlaybackQueue::Item>(item)),
								mMediaPlayer(new TIResult<I<CMediaPlayer> >(mediaPlayer))
						{}

					// Instance methods
			void	cleanup()
						{
							Delete(mItem);
							Delete(mMediaPlayer);
						}

			// Properties
			I<CMediaPlaybackQueue::Item>*	mItem;
			TIResult<I<CMediaPlayer> >*		mMediaPlayer;
		};

						CMediaPlaybackQueueThread(CSRSWMessageQueues& messageQueues,
								const CMediaPlaybackQueue::Info& info) :
							CThread(CString(OSSTR("CMediaPlaybackQueueThread"))),
									mInfo(info), mMessageQueue(1024),
									mShutdownRequested(false),
									mCurrentItemPrepareCancelled(false)
							{
								// Setup
								messageQueues.add(mMessageQueue);

								// Start
								start();
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
										// Queue message
										mMessageQueue.submit(ItemPrepareStartedMessage(handleItemPrepareStarted, this,
												*item));

										// Prepare
										TIResult<I<CMediaPlayer> >	mediaPlayer = (*mCurrentItem)->prepare();

										// Check cancelled
										if (!mCurrentItemPrepareCancelled) {
											// Queue message
											mMessageQueue.submit(ItemPrepareCompletedMessage(handleItemPrepareCompleted,
													this, *item, mediaPlayer));
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
									// Signal
									mSemaphore.signal();

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
		static	void	handleItemPrepareStarted(CSRSWMessageQueue::ProcMessage& message, void* userData)
							{
								// Setup
								CMediaPlaybackQueueThread&	internals = *((CMediaPlaybackQueueThread*) userData);
								ItemPrepareStartedMessage&	itemPrepareStartedMessage =
																	(ItemPrepareStartedMessage&) message;

								// Check if shutdown requested
								if (!internals.mShutdownRequested)
									// Inform
									internals.mInfo.itemPrepareStarted(*itemPrepareStartedMessage.mItem);

								// Cleanup
								itemPrepareStartedMessage.cleanup();
							}
		static	void	handleItemPrepareCompleted(CSRSWMessageQueue::ProcMessage& message, void* userData)
							{
								// Setup
								CMediaPlaybackQueueThread&		internals = *((CMediaPlaybackQueueThread*) userData);
								ItemPrepareCompletedMessage&	itemPrepareCompletedMessage =
																		(ItemPrepareCompletedMessage&) message;

								// Check if shutdown requested
								if (!internals.mShutdownRequested)
									// Inform
									internals.mInfo.itemPrepareCompleted(*itemPrepareCompletedMessage.mItem,
											*itemPrepareCompletedMessage.mMediaPlayer);

								// Cleanup
								itemPrepareCompletedMessage.cleanup();
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
				mThread.waitUntilFinished();
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
		mInternals->mThread.prepare(mInternals->mItems[mInternals->mCurrentItemIndex]);
}

//----------------------------------------------------------------------------------------------------------------------
bool CMediaPlaybackQueue::prepareFirst()
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	if (mInternals->mItems.isEmpty())
		// No items
		return false;
	if (mInternals->mCurrentItemIndex == 0)
		// Already at the first
		return false;

	// Cancel in-flight prepare
	mInternals->mThread.cancelInFlightPrepare();

	// Update
	mInternals->mCurrentItemIndex = 0;

	// Prepare
	mInternals->mThread.prepare(mInternals->mItems[mInternals->mCurrentItemIndex]);

	return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool CMediaPlaybackQueue::preparePrevious()
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	if (mInternals->mCurrentItemIndex == 0)
		// Already at the first
		return false;

	// Cancel in-flight prepare
	mInternals->mThread.cancelInFlightPrepare();

	// Update
	mInternals->mCurrentItemIndex--;

	// Prepare
	mInternals->mThread.prepare(mInternals->mItems[mInternals->mCurrentItemIndex]);

	return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool CMediaPlaybackQueue::prepareNext()
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	if (mInternals->mCurrentItemIndex == (mInternals->mItems.getCount() - 1))
		// Already at the last
		return false;

	// Cancel in-flight prepare
	mInternals->mThread.cancelInFlightPrepare();

	// Update
	mInternals->mCurrentItemIndex++;

	// Prepare
	mInternals->mThread.prepare(mInternals->mItems[mInternals->mCurrentItemIndex]);

	return true;
}
