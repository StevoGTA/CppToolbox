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
							mState(kStateStarting), mPauseRequested(false), mResumeRequested(false),
							mIsSeeking(false), mShutdownRequested(false), mCurrentFrameUpdatedCalled(false),
							mReachedEnd(false), mCurrentPresentationTimeInterval(0.0)

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
						if (performResult.getResult().hasError()) {
							// Finished or error
							mReachedEnd = true;

							// Check error
							if (performResult.getResult().getError() != SError::mEndOfData)
								// Error
								mInfo.error(mVideoFrameStore, performResult.getResult().getError());

							return false;
						}

						// Add frame
						const	CVideoFrame&	videoFrame = performResult.getVideoFrame();

						mVideoFramesLock.lock();
						mVideoFrames.add(videoFrame);
						mVideoFrames.sort(CVideoFrame::comparePresentationTimeInterval);
						mVideoFramesLock.unlock();

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
						// Don't muck with this - we're busy...
						mVideoFramesLock.lock();

						// Check if have frames
						if (!mVideoFrames.isEmpty()) {
							// Setup
							bool	framesUpdated = false;

							// Dump frames before position
							while ((mVideoFrames.getCount() > 1) &&
									(mCurrentPresentationTimeInterval >=
											mVideoFrames[1].getPresentationTimeInterval())) {
								// Dump first frame
								mVideoFrames.removeAtIndex(0);
								framesUpdated = true;
							}

							// Check if need to update current frame
							if (mIsSeeking && !mCurrentFrameUpdatedCalled && (mVideoFrames.getCount() >= 2) &&
									(mCurrentPresentationTimeInterval <
											mVideoFrames[1].getPresentationTimeInterval())) {
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

						// OK, done
						mVideoFramesLock.unlock();
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
		void	seek(UniversalTimeInterval timeInterval)
					{
						// Reset
						mCurrentPresentationTimeInterval = timeInterval;
						mCurrentFrameUpdatedCalled = false;
						mReachedEnd = false;

						mVideoFramesLock.lock();
						mVideoFrames.removeAll();
						mVideoFramesLock.unlock();
					}
		void	shutdown()
					{
						// Request shutdown
						mShutdownRequested = true;

						// Wait until is no lonnger running
						while (isRunning()) {
							// Signal
							mSemaphore.signal();

							// Sleep
							CThread::sleepFor(0.001);
						}
					}

	private:
		CVideoFrameStore&			mVideoFrameStore;
		CVideoFrameStore::Info		mInfo;
		CSemaphore					mSemaphore;

		State						mState;
		bool						mPauseRequested;
		bool						mResumeRequested;
		bool						mIsSeeking;
		bool						mShutdownRequested;
		bool						mCurrentFrameUpdatedCalled;
		bool						mReachedEnd;
		UniversalTimeInterval		mCurrentPresentationTimeInterval;

		TNArray<CVideoFrame>		mVideoFrames;
		CLock						mVideoFramesLock;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CVideoFrameStore::Internals

class CVideoFrameStore::Internals {
	public:
		Internals(CVideoFrameStore& videoFrameStore, const CString& identifier, const CVideoFrameStore::Info& info) :
			mVideoFrameStoreThread(videoFrameStore, identifier, info)
			{}

		CVideoFrameStoreThread	mVideoFrameStoreThread;
		OV<SMedia::Segment>		mMediaSegment;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CVideoFrameStore

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CVideoFrameStore::CVideoFrameStore(const CString& identifier, const Info& info) : CVideoDestination()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(*this, identifier, info);
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
TArray<CString> CVideoFrameStore::getSetupDescription(const CString& indent)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get upstream setup descriptions
	TNArray<CString>	setupDescriptions(CVideoDestination::getSetupDescription(indent));

	// Add our setup description
	setupDescriptions += indent + CString(OSSTR("Video Frame Store"));

	return setupDescriptions;
}

//----------------------------------------------------------------------------------------------------------------------
void CVideoFrameStore::setMediaSegment(const OV<SMedia::Segment>& mediaSegment)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mMediaSegment = mediaSegment;

	// Do super
	CVideoDestination::setMediaSegment(mediaSegment);
}

//----------------------------------------------------------------------------------------------------------------------
void CVideoFrameStore::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Pause thread
	mInternals->mVideoFrameStoreThread.pause();

	// Reset thread
	mInternals->mVideoFrameStoreThread.seek(timeInterval);

	// Seek
	CVideoDestination::seek(timeInterval);

	// Resume thread
	mInternals->mVideoFrameStoreThread.resume();
}

//----------------------------------------------------------------------------------------------------------------------
void CVideoFrameStore::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset thread
	mInternals->mVideoFrameStoreThread.seek(
			mInternals->mMediaSegment.hasValue() ? mInternals->mMediaSegment->getStartTimeInterval() : 0.0);

	// Reset
	CVideoDestination::reset();

	// Resume
	mInternals->mVideoFrameStoreThread.resume();
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
