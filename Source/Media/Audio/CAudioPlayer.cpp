//----------------------------------------------------------------------------------------------------------------------
//	CAudioPlayer.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioPlayer.h"

#include "ConcurrencyPrimitives.h"
#include "CPreferences.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CPreferences::UniversalTimeIntervalPref	sPlaybackBufferDurationPref(
														OSSTR("coreAudioPlayerOutputUnitReadAheadBufferTimeSecs"),
														0.25);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioPlayer

// MARK: Properties

const	UniversalTimeInterval	CAudioPlayer::kPreviewDuration = 0.1;

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
UniversalTimeInterval CAudioPlayer::getPlaybackBufferDuration()
//----------------------------------------------------------------------------------------------------------------------
{
	return CPreferences::mDefault.getUniversalTimeInterval(sPlaybackBufferDurationPref);
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::setPlaybackBufferDuration(UniversalTimeInterval playbackBufferDuration)
//----------------------------------------------------------------------------------------------------------------------
{
	CPreferences::mDefault.set(sPlaybackBufferDurationPref, playbackBufferDuration);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioPlayerBufferThread::Internals

class CAudioPlayerBufferThread::Internals {
	public:
		enum State {
			kStateStarting,
			kStateWaiting,
			kStateFilling,
		};

				Internals(CAudioPlayer& audioPlayer, CSRSWBIPSegmentedQueue& queue,
						UInt32 bytesPerFrame, UInt32 maxOutputFrames, CAudioPlayerBufferThread::ErrorProc errorProc,
						void* procsUserData) :
					mAudioPlayer(audioPlayer), mErrorProc(errorProc), mQueue(queue), mBytesPerFrame(bytesPerFrame),
							mMaxOutputFrames(maxOutputFrames), mProcsUserData(procsUserData),
							mPauseRequested(false), mResumeRequested(false),
							mShutdownRequested(false), mPerformedFirstFill(false), mReachedEnd(false),
							mState(kStateStarting)
					{}

		bool	tryFill()
					{
						// Setup
						if (!mRequirements.hasValue())
							// Query
							mRequirements = OV<CAudioFrames::Requirements>(mAudioPlayer.queryRequirements());

						// Request write
						UInt32									framesToRequest =
																		mRequirements->getFrameCount(
																				mMaxOutputFrames * 2);
						CSRSWBIPSegmentedQueue::WriteBufferInfo	writeBufferInfo =
																		mQueue.requestWrite(
																				framesToRequest * mBytesPerFrame);
						if (!writeBufferInfo.hasBuffer())
							// No space
							return false;

						// Perform read
						CAudioFrames		audioFrames(writeBufferInfo.mBuffer, mQueue.getSegmentCount(),
													writeBufferInfo.mSegmentByteCount,
													!mPerformedFirstFill ?
															framesToRequest :
															writeBufferInfo.mByteCount / mBytesPerFrame,
													mBytesPerFrame);

						// If we have not performed the first fill, we queue 2 * max output frames to get us started.
						//	Subsequently, we try to fill the buffer as much as possible.
						SAudioSourceStatus	audioSourceStatus = mAudioPlayer.performInto(audioFrames);
						if (audioSourceStatus.isSuccess()) {
							// Success
							mQueue.commitWrite(audioFrames.getCurrentFrameCount() * mBytesPerFrame);

							return true;
						} else {
							// Finished or error
							mReachedEnd = true;
							
							if (audioSourceStatus.getError() != SError::mEndOfData)
								// Error
								mErrorProc(audioSourceStatus.getError(), mProcsUserData);

							return false;
						}
					}

		CAudioPlayer&						mAudioPlayer;
		CAudioPlayerBufferThread::ErrorProc	mErrorProc;
		CSemaphore							mSemaphore;
		CSRSWBIPSegmentedQueue&				mQueue;
		UInt32								mBytesPerFrame;
		UInt32								mMaxOutputFrames;
		void*								mProcsUserData;
		OV<CAudioFrames::Requirements>		mRequirements;

		bool								mPauseRequested;
		bool								mResumeRequested;
		bool								mShutdownRequested;
		bool								mPerformedFirstFill;
		bool								mReachedEnd;
		State								mState;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioPlayerBufferThread

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioPlayerBufferThread::CAudioPlayerBufferThread(CAudioPlayer& audioPlayer, CSRSWBIPSegmentedQueue& queue,
		UInt32 bytesPerFrame, UInt32 maxOutputFrames, ErrorProc errorProc, void* procsUserData) :
	CThread(CString(OSSTR("Audio Reader - ")) + audioPlayer.getIdentifier())
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals(audioPlayer, queue, bytesPerFrame, maxOutputFrames, errorProc, procsUserData);

	// Start
	start();
}

//----------------------------------------------------------------------------------------------------------------------
CAudioPlayerBufferThread::~CAudioPlayerBufferThread()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CThread methods

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayerBufferThread::run()
//----------------------------------------------------------------------------------------------------------------------
{
	// Run until shutdown
	while (!mInternals->mShutdownRequested) {
		// Check state
		switch (mInternals->mState) {
			case Internals::kStateStarting:
				// Starting
				mInternals->mState = mInternals->mResumeRequested ? Internals::kStateFilling : Internals::kStateWaiting;
				break;

			case Internals::kStateWaiting:
				// Waiting
				mInternals->mSemaphore.waitFor();

				// Check if stop filling requested
				if (!mInternals->mPauseRequested)
					// Go for filling
					mInternals->mState = Internals::kStateFilling;
				break;

			case Internals::kStateFilling:
				// Filling
				if (mInternals->mPauseRequested || !mInternals->tryFill())
					// Go to waiting
					mInternals->mState = Internals::kStateWaiting;
				break;
		}
	}
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayerBufferThread::pause()
//----------------------------------------------------------------------------------------------------------------------
{
	// Request pause
	mInternals->mPauseRequested = true;

	// Wait until waiting
	while (mInternals->mState != Internals::kStateWaiting)
		// Sleep
		CThread::sleepFor(0.001);
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayerBufferThread::resume()
//----------------------------------------------------------------------------------------------------------------------
{
	// Update
	mInternals->mPauseRequested = false;
	mInternals->mResumeRequested = true;
	mInternals->mPerformedFirstFill = false;
	mInternals->mReachedEnd = false;

	// Signal
	mInternals->mSemaphore.signal();
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayerBufferThread::noteQueueReadComplete()
//----------------------------------------------------------------------------------------------------------------------
{
	// Signal
	mInternals->mSemaphore.signal();
}

//----------------------------------------------------------------------------------------------------------------------
bool CAudioPlayerBufferThread::getDidReachEnd() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mReachedEnd;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayerBufferThread::shutdown()
//----------------------------------------------------------------------------------------------------------------------
{
	// Request shutdown
	mInternals->mShutdownRequested = true;

	// Wait until is no lonnger running
	while (isRunning()) {
		// Signal
		mInternals->mSemaphore.signal();

		// Sleep
		CThread::sleepFor(0.001);
	}
}
