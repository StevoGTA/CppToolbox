//----------------------------------------------------------------------------------------------------------------------
//	CVideoFrameStore.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CVideoFrameStore.h"

#include "ConcurrencyPrimitives.h"
#include "CThread.h"
#include "TLockingArray.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CVideoFrameStoreThread

class CVideoFrameStoreThread : public CThread {
	public:
		enum State {
			kStateStarting,
			kStateWaiting,
			kStateFilling,
		};

				CVideoFrameStoreThread(CVideoFrameStore& videoFrameStore, const CString& identifier,
						const CVideoFrameStore::Info& info) :
					CThread(CString(OSSTR("Video Frame Store - ")) + identifier),
							mVideoFrameStore(videoFrameStore), mInfo(info),
							mPauseRequested(false), mResumeRequested(false),
							mShutdownRequested(false), mCurrentPresentationTimeInterval(0.0),
							mCurrentFrameUpdatedCalled(false), mIsSeeking(false), mReachedEnd(false),
							mState(kStateStarting)
					{
						// Start
						start();
					}

		void	run()
					{
						// Run until shutdown
						while (!mShutdownRequested) {
							// Check state
							switch (mState) {
								case kStateStarting:
									// Starting
									mState = mResumeRequested ? kStateFilling : kStateWaiting;
									break;

								case kStateWaiting:
									// Waiting
									mSemaphore.waitFor();

									// Check if stop filling requested
									if (!mPauseRequested)
										// Go for reading
										mState = kStateFilling;
									break;

								case kStateFilling:
									// Filling
									if (mPauseRequested || !tryAddFrame())
										// Go to waiting
										mState = kStateWaiting;
									break;
							}
						}
					}

		bool	tryAddFrame()
					{
						// Check if have enough
						if (mVideoFrames.getCount() >= 10)
							// Pause
							return false;

						// Perform
						CVideoProcessor::PerformResult	performResult = mVideoFrameStore.perform();
						if (!performResult.getVideoSourceStatus().isSuccess()) {
							// Finished or error
							mReachedEnd = true;
							
							if (performResult.getVideoSourceStatus().getError() != SError::mEndOfData)
								// Error
								mInfo.error(mVideoFrameStore, performResult.getVideoSourceStatus().getError());

							return false;
						}

						// Store frame
						const	CVideoFrame&	videoFrame = performResult.getVideoFrame();
						mVideoFrames.add(videoFrame);
						mVideoFrames.sort(CVideoFrame::comparePresentationTimeInterval);

						// Update current frame
						updateCurrentFrame();

						return true;
					}
		void	updateCurrentPresentationTimeInterval(UniversalTimeInterval timeInterval)
					{
						// Store
						mCurrentPresentationTimeInterval = timeInterval;

						// Update current frame
						updateCurrentFrame();
					}
		void	updateIsSeeking(bool isSeeking)
					{ mIsSeeking = isSeeking; }
		void	updateCurrentFrame()
					{
						// Check if have frames
						if (mVideoFrames.isEmpty())
							// Nothing to do
							return;

						// Setup
						bool	framesUpdated = false;

						// Dump frames before position
						while ((mVideoFrames.getCount() > 1) &&
								(mCurrentPresentationTimeInterval >= mVideoFrames[1].getPresentationTimeInterval())) {
							// Dump first frame
							mVideoFrames.removeAtIndex(0);
							framesUpdated = true;
						}

						// Check if need to update current frame
						if (mIsSeeking && !mCurrentFrameUpdatedCalled && (mVideoFrames.getCount() >= 2) &&
								(mCurrentPresentationTimeInterval < mVideoFrames[1].getPresentationTimeInterval())) {
							// Notify
							mInfo.currentFrameUpdated(mVideoFrameStore, mVideoFrames[0]);
							mCurrentFrameUpdatedCalled = true;

							// Trigger for more decoding
							mSemaphore.signal();
						} else if (!mCurrentFrameUpdatedCalled || framesUpdated) {
							// Notify
							mInfo.currentFrameUpdated(mVideoFrameStore, mVideoFrames[0]);
							mCurrentFrameUpdatedCalled = true;

							// Trigger for more decoding
							mSemaphore.signal();
						}
					}
		void	pause()
					{
						// Request pause
						mPauseRequested = true;

						// Wait until waiting
						while (mState != kStateWaiting)
							// Sleep
							CThread::sleepFor(0.001);
					}
		void	resume()
					{
						// Update
						mPauseRequested = false;
						mResumeRequested = true;
						mReachedEnd = false;

						// Signal
						mSemaphore.signal();
					}
		void	reset()
					{
						// Reset
						mCurrentPresentationTimeInterval = 0.0;
						mCurrentFrameUpdatedCalled = false;
						mReachedEnd = false;
						mVideoFrames.removeAll();
					}
		void	shutdown()
					{
						// Request shutdown
						mShutdownRequested = true;

						// Signal if waiting
						mSemaphore.signal();

						// Wait until is no lonnger running
						while (getIsRunning())
							// Sleep
							CThread::sleepFor(0.001);
					}

	private:
		CVideoFrameStore&			mVideoFrameStore;
		CVideoFrameStore::Info		mInfo;
		CSemaphore					mSemaphore;

		bool						mPauseRequested;
		bool						mResumeRequested;
		bool						mShutdownRequested;
		UniversalTimeInterval		mCurrentPresentationTimeInterval;
		bool						mCurrentFrameUpdatedCalled;
		bool						mIsSeeking;
		bool						mReachedEnd;
		State						mState;
		TNLockingArray<CVideoFrame>	mVideoFrames;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CVideoFrameStoreInternals

class CVideoFrameStoreInternals {
	public:
		CVideoFrameStoreInternals(CVideoFrameStore& videoFrameStore, const CString& identifier,
				const CVideoFrameStore::Info& info) :
			mVideoFrameStoreThread(videoFrameStore, identifier, info)
			{}

		CVideoFrameStoreThread	mVideoFrameStoreThread;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CVideoFrameStore

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CVideoFrameStore::CVideoFrameStore(const CString& identifier, const Info& info) : CVideoDestination()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CVideoFrameStoreInternals(*this, identifier, info);
}

//----------------------------------------------------------------------------------------------------------------------
CVideoFrameStore::~CVideoFrameStore()
//----------------------------------------------------------------------------------------------------------------------
{
	// Shutdown
	mInternals->mVideoFrameStoreThread.shutdown();

	// Cleanup
	Delete(mInternals);
}

// MARK: CVideoProcessor methods

//----------------------------------------------------------------------------------------------------------------------
void CVideoFrameStore::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Pause thread
	mInternals->mVideoFrameStoreThread.pause();

	// Reset thread
	mInternals->mVideoFrameStoreThread.reset();

	// Seek
	CVideoDestination::seek(timeInterval);

	// Resume thread
	mInternals->mVideoFrameStoreThread.updateCurrentPresentationTimeInterval(timeInterval);
	mInternals->mVideoFrameStoreThread.resume();
}

//----------------------------------------------------------------------------------------------------------------------
void CVideoFrameStore::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset thread
	mInternals->mVideoFrameStoreThread.reset();

	// Reset
	CVideoDestination::reset();
}

// MARK: CVideoDestination methods

//----------------------------------------------------------------------------------------------------------------------
void CVideoFrameStore::setupComplete()
//----------------------------------------------------------------------------------------------------------------------
{
	// Resume thread
	mInternals->mVideoFrameStoreThread.resume();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CVideoFrameStore::notePositionUpdated(UniversalTimeInterval position)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update
	mInternals->mVideoFrameStoreThread.updateCurrentPresentationTimeInterval(position);
}

//----------------------------------------------------------------------------------------------------------------------
void CVideoFrameStore::resume()
//----------------------------------------------------------------------------------------------------------------------
{
	// Resume thread
	mInternals->mVideoFrameStoreThread.resume();
}

//----------------------------------------------------------------------------------------------------------------------
void CVideoFrameStore::pause()
//----------------------------------------------------------------------------------------------------------------------
{
	// Pause thread
	mInternals->mVideoFrameStoreThread.pause();
}

//----------------------------------------------------------------------------------------------------------------------
void CVideoFrameStore::startSeek()
//----------------------------------------------------------------------------------------------------------------------
{
	// Is seeking
	mInternals->mVideoFrameStoreThread.updateIsSeeking(true);
}

//----------------------------------------------------------------------------------------------------------------------
void CVideoFrameStore::finishSeek()
//----------------------------------------------------------------------------------------------------------------------
{
	// No longer seeking
	mInternals->mVideoFrameStoreThread.updateIsSeeking(false);
}
