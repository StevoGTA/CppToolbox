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
// MARK: - CAudioPlayer::BufferThread::Internals

class CAudioPlayer::BufferThread::Internals {
	public:
		enum State {
			kStateStarting,
			kStateWaiting,
			kStateFilling,
		};

				Internals(CAudioPlayer& audioPlayer, CSRSWBIPSegmentedQueue& queue,
						UInt32 bytesPerFrame, UInt32 maxOutputFrames, BufferThread::ErrorProc errorProc,
						void* procsUserData) :
					mAudioPlayer(audioPlayer), mErrorProc(errorProc), mQueue(queue), mBytesPerFrame(bytesPerFrame),
							mMaxOutputFrames(maxOutputFrames), mProcsUserData(procsUserData),
							mPauseRequested(false), mResumeRequested(false), mShutdownRequested(false),
							mReachedEnd(false), mState(kStateStarting)
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
													writeBufferInfo.mSegmentByteCount, framesToRequest, mBytesPerFrame);

						// Each fill queues 2 * max output frames.  The thread keeps filling until the queue is full.
						TVResult<CAudioProcessor::SourceInfo>	audioProcessorSourceInfo =
																		mAudioPlayer.performInto(audioFrames);
						if (audioProcessorSourceInfo.hasValue()) {
							// Success
							mQueue.commitWrite(audioFrames.getCurrentFrameCount() * mBytesPerFrame);

							return true;
						} else {
							// Finished or error
							mReachedEnd = true;
							
							if (audioProcessorSourceInfo.getError() != SError::mEndOfData)
								// Error
								mErrorProc(audioProcessorSourceInfo.getError(), mProcsUserData);

							return false;
						}
					}

		CAudioPlayer&					mAudioPlayer;
		ErrorProc						mErrorProc;
		CSemaphore						mSemaphore;
		CSRSWBIPSegmentedQueue&			mQueue;
		UInt32							mBytesPerFrame;
		UInt32							mMaxOutputFrames;
		void*							mProcsUserData;
		OV<CAudioFrames::Requirements>	mRequirements;

		std::atomic<bool>				mPauseRequested;
		std::atomic<bool>				mResumeRequested;
		std::atomic<bool>				mShutdownRequested;
		std::atomic<bool>				mReachedEnd;
		std::atomic<State>				mState;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioPlayer::BufferThread

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioPlayer::BufferThread::BufferThread(CAudioPlayer& audioPlayer, CSRSWBIPSegmentedQueue& queue,
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
CAudioPlayer::BufferThread::~BufferThread()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CThread methods

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::BufferThread::run()
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
void CAudioPlayer::BufferThread::pause()
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
void CAudioPlayer::BufferThread::resume()
//----------------------------------------------------------------------------------------------------------------------
{
	// Update
	mInternals->mResumeRequested = true;
	mInternals->mReachedEnd = false;
	mInternals->mPauseRequested = false;

	// Signal
	mInternals->mSemaphore.signal();
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::BufferThread::noteQueueReadComplete()
//----------------------------------------------------------------------------------------------------------------------
{
	// Signal
	mInternals->mSemaphore.signal();
}

//----------------------------------------------------------------------------------------------------------------------
bool CAudioPlayer::BufferThread::getDidReachEnd() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mReachedEnd;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::BufferThread::shutdown()
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

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioPlayer

// MARK: Properties

const	UniversalTimeInterval	CAudioPlayer::mPreviewDuration = 0.1;

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
UniversalTimeInterval CAudioPlayer::getPlaybackBufferDuration()
//----------------------------------------------------------------------------------------------------------------------
{
	return CPreferences::shared().getUniversalTimeInterval(sPlaybackBufferDurationPref);
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::setPlaybackBufferDuration(UniversalTimeInterval playbackBufferDuration)
//----------------------------------------------------------------------------------------------------------------------
{
	CPreferences::shared().set(sPlaybackBufferDurationPref, playbackBufferDuration);
}
