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
							mResumeRequested(false), mStopFillingRequested(false),
							mShutdownRequested(false), mNotifiedFirstFrameReady(false), mReachedEnd(false),
							mState(kStateStarting), mMediaPosition(SMediaPosition::fromStart(0.0))
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
									if (mResumeRequested) {
										// Start filling
										mState = kStateFilling;
										break;
									} else {
										// Wait for resume
										mState = kStateWaiting;

										// Fall through
									}

								case kStateWaiting:
									// Waiting
									mSemaphore.waitFor();

									// Check if stop filling requested
									if (!mStopFillingRequested)
										// Go for reading
										mState = kStateFilling;
									break;

								case kStateFilling:
									// Filling
									if (mStopFillingRequested || !tryAddFrame())
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
						CVideoProcessor::PerformResult	performResult = mVideoFrameStore.perform(mMediaPosition);
						if (!performResult.getVideoSourceStatus().isSuccess()) {
							// Finished or error
							mReachedEnd = true;
							
							if (*performResult.getVideoSourceStatus().getError() != SError::mEndOfData)
								// Error
								mInfo.error(mVideoFrameStore, *performResult.getVideoSourceStatus().getError());

							return false;
						}

						// Store frame
						const	CVideoFrame&	videoFrame = *performResult.getVideoFrame();
						mVideoFrames.add(videoFrame);
						mVideoFrames.sort(CVideoFrame::comparePresentationTimeInterval);

						// Update media position
						mMediaPosition = SMediaPosition::fromCurrent();

						// CHeck if notified first frame is ready
						if (!mNotifiedFirstFrameReady) {
							// Inform current frame updated
							mInfo.currentFrameUpdated(mVideoFrameStore, videoFrame);

							mNotifiedFirstFrameReady = true;
						}

						return true;
					}

		CVideoFrameStore&			mVideoFrameStore;
		CVideoFrameStore::Info		mInfo;
		CSemaphore					mSemaphore;

		bool						mResumeRequested;
		bool						mStopFillingRequested;
		bool						mShutdownRequested;
		bool						mNotifiedFirstFrameReady;
		bool						mReachedEnd;
		State						mState;
		SMediaPosition				mMediaPosition;
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
	// Request shutdown
	mInternals->mVideoFrameStoreThread.mShutdownRequested = true;

	// Signal if waiting
	mInternals->mVideoFrameStoreThread.mSemaphore.signal();

	// Wait until is no lonnger running
	while (mInternals->mVideoFrameStoreThread.getIsRunning())
		// Sleep
		CThread::sleepFor(0.001);

	// Cleanup
	Delete(mInternals);
}

// MARK: CVideoProcessor methods

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CVideoFrameStore::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Update
	mInternals->mVideoFrameStoreThread.mNotifiedFirstFrameReady = false;

	return CVideoDestination::reset();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CVideoFrameStore::notePositionUpdated(UniversalTimeInterval position)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	bool	framesUpdated = false;

	// Dump frames before position
	while ((mInternals->mVideoFrameStoreThread.mVideoFrames.getCount() > 1) &&
			(position >= mInternals->mVideoFrameStoreThread.mVideoFrames[1].getPresentationTimeInterval())) {
		// Dump first frame
		mInternals->mVideoFrameStoreThread.mVideoFrames.removeAtIndex(0);
		framesUpdated = true;
	}

	// Check if need to notify
	if (framesUpdated) {
		// Notify
		mInternals->mVideoFrameStoreThread.mInfo.currentFrameUpdated(*this,
				mInternals->mVideoFrameStoreThread.mVideoFrames[0]);

		// Trigger for more decoding
		mInternals->mVideoFrameStoreThread.mSemaphore.signal();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CVideoFrameStore::resume()
//----------------------------------------------------------------------------------------------------------------------
{
	// Update
	mInternals->mVideoFrameStoreThread.mResumeRequested = true;
	mInternals->mVideoFrameStoreThread.mStopFillingRequested = false;

	// Signal
	mInternals->mVideoFrameStoreThread.mSemaphore.signal();
}

//----------------------------------------------------------------------------------------------------------------------
void CVideoFrameStore::pause()
//----------------------------------------------------------------------------------------------------------------------
{
	// Request stop filling
	mInternals->mVideoFrameStoreThread.mStopFillingRequested = true;

	// Wait until waiting
	while (mInternals->mVideoFrameStoreThread.mState != CVideoFrameStoreThread::kStateWaiting)
		// Sleep
		CThread::sleepFor(0.001);
}

//----------------------------------------------------------------------------------------------------------------------
void CVideoFrameStore::startSeek()
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
void CVideoFrameStore::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
void CVideoFrameStore::finishSeek()
//----------------------------------------------------------------------------------------------------------------------
{
}
