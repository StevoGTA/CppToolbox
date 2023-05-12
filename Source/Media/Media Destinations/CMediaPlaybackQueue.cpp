//----------------------------------------------------------------------------------------------------------------------
//	CMediaPlaybackQueue.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMediaPlaybackQueue.h"

#include "ConcurrencyPrimitives.h"
#include "CThread.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaPlaybackQueueItemPrepareThread

class CMediaPlaybackQueueItemPrepareThread : public CThread {
	public:
		struct StartedMessage : public CSRSWMessageQueue::ProcMessage {
					// Lifecycle Methods
					StartedMessage(Proc proc, void* userData, const I<CMediaPlaybackQueue::Item>& item) :
						CSRSWMessageQueue::ProcMessage(sizeof(StartedMessage), proc, userData),
								mItem(new I<CMediaPlaybackQueue::Item>(item))
						{}

					// Instance methods
			void	cleanup()
						{ Delete(mItem); }

			// Properties
			I<CMediaPlaybackQueue::Item>*	mItem;
		};
		struct CompletedMessage : public CSRSWMessageQueue::ProcMessage {
					// Lifecycle Methods
					CompletedMessage(Proc proc, void* userData, const I<CMediaPlaybackQueue::Item>& item,
							const TVResult<I<CMediaPlayer> >& mediaPlayer) :
						CSRSWMessageQueue::ProcMessage(sizeof(CompletedMessage), proc, userData),
								mItem(new I<CMediaPlaybackQueue::Item>(item)),
								mMediaPlayer(new TVResult<I<CMediaPlayer> >(mediaPlayer))
						{}

					// Instance methods
			void	cleanup()
						{
							Delete(mItem);
							Delete(mMediaPlayer);
						}

			// Properties
			I<CMediaPlaybackQueue::Item>*	mItem;
			TVResult<I<CMediaPlayer> >*		mMediaPlayer;
		};

						CMediaPlaybackQueueItemPrepareThread(CSRSWMessageQueues& messageQueues,
								const CMediaPlaybackQueue::Info& info) :
							CThread(CString(OSSTR("CMediaPlaybackQueueItemPrepareThread"))),
									mInfo(info), mMessageQueue(1024),
									mShutdownRequested(false),
									mCancelled(false)
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

									// Get item
									mItemLock.lock();
									OR<I<CMediaPlaybackQueue::Item> >	item = mItem;
									mItemLock.unlock();

									// Check for current item
									if (item.hasReference()) {
										// Queue message
										mMessageQueue.submit(StartedMessage(handleStarted, this, *item));

										// Prepare
										TVResult<I<CMediaPlayer> >	mediaPlayer = (*item)->prepare();

										// Check cancelled
										if (!mCancelled)
											// Queue message
											mMessageQueue.submit(CompletedMessage(handleCompleted, this, *item,
													mediaPlayer));

										// Clear item
										mItemLock.lock();
										mItem = OR<I<CMediaPlaybackQueue::Item> >();
										mItemLock.unlock();
									}
								}
							}

				void	prepare(I<CMediaPlaybackQueue::Item>& item)
							{
								// Store
								mItemLock.lock();
								mItem = OR<I<CMediaPlaybackQueue::Item> >(item);
								mCancelled = false;
								mItemLock.unlock();

								// Signal
								mSemaphore.signal();
							}
				void	cancelInflight()
							{
								// Setup
								bool	itemHasReference = false;

								// Cancel item if there is one
								mItemLock.lock();
								if (mItem.hasReference()) {
									// Cancel
									mCancelled = true;
									(*mItem)->cancel();

									itemHasReference = true;
								}
								mItemLock.unlock();

								// Check if need to wait
								while (itemHasReference) {
									// Signal
									mSemaphore.signal();

									// Sleep
									CThread::sleepFor(0.001);

									// Check item
									mItemLock.lock();
									itemHasReference = mItem.hasReference();
									mItemLock.unlock();
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
		static	void	handleStarted(CSRSWMessageQueue::ProcMessage& message, void* userData)
							{
								// Setup
								CMediaPlaybackQueueItemPrepareThread&	mediaPlaybackQueueItemPrepareThread =
																				*((CMediaPlaybackQueueItemPrepareThread*)
																						userData);
								StartedMessage&							startedMessage = (StartedMessage&) message;

								// Check if shutdown requested
								if (!mediaPlaybackQueueItemPrepareThread.mShutdownRequested)
									// Inform
									mediaPlaybackQueueItemPrepareThread.mInfo.itemPrepareStarted(*startedMessage.mItem);

								// Cleanup
								startedMessage.cleanup();
							}
		static	void	handleCompleted(CSRSWMessageQueue::ProcMessage& message, void* userData)
							{
								// Setup
								CMediaPlaybackQueueItemPrepareThread&	mediaPlaybackQueueItemPrepareThread =
																				*((CMediaPlaybackQueueItemPrepareThread*)
																						userData);
								CompletedMessage&						completedMessage = (CompletedMessage&) message;

								// Check if shutdown requested
								if (!mediaPlaybackQueueItemPrepareThread.mShutdownRequested)
									// Inform
									mediaPlaybackQueueItemPrepareThread.mInfo.itemPrepareCompleted(
											*completedMessage.mItem, *completedMessage.mMediaPlayer);

								// Cleanup
								completedMessage.cleanup();
							}

	private:
		CMediaPlaybackQueue::Info			mInfo;
		CSemaphore							mSemaphore;
		CSRSWMessageQueue					mMessageQueue;

		bool								mShutdownRequested;

		OR<I<CMediaPlaybackQueue::Item> >	mItem;
		bool								mCancelled;
		CLock								mItemLock;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaPlaybackQueue::Internals

class CMediaPlaybackQueue::Internals {
	public:
		Internals(CSRSWMessageQueues& messageQueues, const CMediaPlaybackQueue::Info& info) :
			mItemPrepareThread(messageQueues, info),
					mCurrentItemIndex(0)
			{}
		~Internals()
			{
				// Request shutdown
				mItemPrepareThread.shutdown();

				// Wait until is no lonnger running
				mItemPrepareThread.waitUntilFinished();
			}

		CMediaPlaybackQueueItemPrepareThread	mItemPrepareThread;

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
	mInternals = new Internals(messageQueues, info);
}

//----------------------------------------------------------------------------------------------------------------------
CMediaPlaybackQueue::~CMediaPlaybackQueue()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const TArray<I<CMediaPlaybackQueue::Item> >& CMediaPlaybackQueue::getItems() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mItems;
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlaybackQueue::set(const TArray<I<Item> >& items)
//----------------------------------------------------------------------------------------------------------------------
{
	// Cancel in-flight prepare
	mInternals->mItemPrepareThread.cancelInflight();

	// Store
	mInternals->mItems = items;
	mInternals->mCurrentItemIndex = 0;

	// Check for items
	if (!mInternals->mItems.isEmpty())
		// Prepare
		mInternals->mItemPrepareThread.prepare(mInternals->mItems[mInternals->mCurrentItemIndex]);
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
	mInternals->mItemPrepareThread.cancelInflight();

	// Update
	mInternals->mCurrentItemIndex = 0;

	// Prepare
	mInternals->mItemPrepareThread.prepare(mInternals->mItems[mInternals->mCurrentItemIndex]);

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
	mInternals->mItemPrepareThread.cancelInflight();

	// Update
	mInternals->mCurrentItemIndex--;

	// Prepare
	mInternals->mItemPrepareThread.prepare(mInternals->mItems[mInternals->mCurrentItemIndex]);

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
	mInternals->mItemPrepareThread.cancelInflight();

	// Update
	mInternals->mCurrentItemIndex++;

	// Prepare
	mInternals->mItemPrepareThread.prepare(mInternals->mItems[mInternals->mCurrentItemIndex]);

	return true;
}
