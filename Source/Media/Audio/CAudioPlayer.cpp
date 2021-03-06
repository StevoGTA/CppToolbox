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

const	UniversalTimeInterval	CAudioPlayer::kMinBufferDuration = 0.1;
const	UniversalTimeInterval	CAudioPlayer::kMaxBufferDuration = 2.0;
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
// MARK: - CAudioPlayerReaderThreadInternals

class CAudioPlayerReaderThreadInternals {
	public:
		enum State {
			kStateStarting,
			kStateWaiting,
			kStateReading,
		};

				CAudioPlayerReaderThreadInternals(CAudioPlayer& audioPlayer, CSRSWBIPSegmentedQueue& queue,
						UInt32 bytesPerFrame, UInt32 maxOutputFrames, CAudioPlayerReaderThread::ErrorProc errorProc,
						void* procsUserData) :
					mAudioPlayer(audioPlayer), mErrorProc(errorProc), mQueue(queue), mBytesPerFrame(bytesPerFrame),
							mMaxOutputFrames(maxOutputFrames), mProcsUserData(procsUserData),
							mResumeRequesed(false), mStopReadingRequested(false),
							mShutdownRequested(false), mReachedEndOfData(false), mState(kStateStarting),
							mMediaPosition(SMediaPosition::fromStart(0.0)), mFramesToRead(~0)
					{}

		bool	tryRead()
					{
						// Request write
						UInt32									bytesToRequest =
																		std::min<UInt32>(
																						mMaxOutputFrames * 2,
																						mFramesToRead) *
																				mBytesPerFrame;
						CSRSWBIPSegmentedQueue::WriteBufferInfo	writeBufferInfo = mQueue.requestWrite(bytesToRequest);
						if (writeBufferInfo.mSize < mBytesPerFrame)
							// No space
							return false;

						// Perform read
						// If the media position is "from start", we queue 2 * max output frames to get us started.
						//	Subsequently (i.e. if media position is "from current"), we try to fill the buffer as much
						//	as possible.
						UInt32				bytesToRead =
													(mMediaPosition.getMode() == SMediaPosition::kFromStart) ?
															bytesToRequest : writeBufferInfo.mSize;
						CAudioFrames		audioFrames(writeBufferInfo.mBuffer, mQueue.getSegmentCount(),
													writeBufferInfo.mSegmentSize / mBytesPerFrame,
													bytesToRead / mBytesPerFrame, mBytesPerFrame);
						SAudioSourceStatus	audioSourceStatus = mAudioPlayer.perform(mMediaPosition, audioFrames);
						if (audioSourceStatus.isSuccess()) {
							// Success
							mQueue.commitWrite(audioFrames.getCurrentFrameCount() * mBytesPerFrame);
							mMediaPosition = SMediaPosition::fromCurrent();
							mFramesToRead -= audioFrames.getCurrentFrameCount();

							return true;
						} else {
							// Finished
							mReachedEndOfData = true;
							if (*audioSourceStatus.getError() != SError::mEndOfData)
								// Error
								mErrorProc(*audioSourceStatus.getError(), mProcsUserData);

							return false;
						}
					}

		CAudioPlayer&						mAudioPlayer;
		CAudioPlayerReaderThread::ErrorProc	mErrorProc;
		CSemaphore							mSemaphore;
		CSRSWBIPSegmentedQueue&				mQueue;
		UInt32								mBytesPerFrame;
		UInt32								mMaxOutputFrames;
		void*								mProcsUserData;

		bool								mResumeRequesed;
		bool								mStopReadingRequested;
		bool								mShutdownRequested;
		bool								mReachedEndOfData;
		State								mState;
		SMediaPosition						mMediaPosition;
		UInt32								mFramesToRead;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioPlayerReaderThread

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioPlayerReaderThread::CAudioPlayerReaderThread(CAudioPlayer& audioPlayer, CSRSWBIPSegmentedQueue& queue,
		UInt32 bytesPerFrame, UInt32 maxOutputFrames, ErrorProc errorProc, void* procsUserData) :
	CThread(CString(OSSTR("Audio Reader - ")) + audioPlayer.getIdentifier())
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals =
			new CAudioPlayerReaderThreadInternals(audioPlayer, queue, bytesPerFrame, maxOutputFrames, errorProc,
					procsUserData);

	// Start
	start();
}

//----------------------------------------------------------------------------------------------------------------------
CAudioPlayerReaderThread::~CAudioPlayerReaderThread()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CThread methods

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayerReaderThread::run()
//----------------------------------------------------------------------------------------------------------------------
{
	// Run until shutdown
	while (!mInternals->mShutdownRequested) {
		// Check state
		switch (mInternals->mState) {
			case CAudioPlayerReaderThreadInternals::kStateStarting:
				// Starting
				if (mInternals->mResumeRequesed) {
					// Start reading
					mInternals->mState = CAudioPlayerReaderThreadInternals::kStateReading;
					break;
				} else {
					// Wait for resume
					mInternals->mState = CAudioPlayerReaderThreadInternals::kStateWaiting;

					// Fall through
				}

			case CAudioPlayerReaderThreadInternals::kStateWaiting:
				// Waiting
				mInternals->mSemaphore.waitFor();

				// Check if reset requested
				if (!mInternals->mStopReadingRequested)
					// Go for reading
					mInternals->mState = CAudioPlayerReaderThreadInternals::kStateReading;
				break;

			case CAudioPlayerReaderThreadInternals::kStateReading:
				// Reading
				if (mInternals->mStopReadingRequested || !mInternals->tryRead())
					// Go to waiting
					mInternals->mState = CAudioPlayerReaderThreadInternals::kStateWaiting;
				break;
		}
	}
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayerReaderThread::seek(UniversalTimeInterval timeInterval, UInt32 maxFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update
	mInternals->mMediaPosition = SMediaPosition::fromStart(timeInterval);
	mInternals->mFramesToRead = maxFrames;
	mInternals->mReachedEndOfData = false;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayerReaderThread::resume()
//----------------------------------------------------------------------------------------------------------------------
{
	// Update
	mInternals->mResumeRequesed = true;
	mInternals->mStopReadingRequested = false;

	// Signal
	mInternals->mSemaphore.signal();
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayerReaderThread::noteQueueReadComplete()
//----------------------------------------------------------------------------------------------------------------------
{
	// Signal
	mInternals->mSemaphore.signal();
}

//----------------------------------------------------------------------------------------------------------------------
bool CAudioPlayerReaderThread::getDidReachEndOfData() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mReachedEndOfData;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayerReaderThread::stopReading()
//----------------------------------------------------------------------------------------------------------------------
{
	// Request reset
	mInternals->mStopReadingRequested = true;

	// Wait until waiting
	while (mInternals->mState != CAudioPlayerReaderThreadInternals::kStateWaiting)
		// Sleep
		CThread::sleepFor(0.001);
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayerReaderThread::shutdown()
//----------------------------------------------------------------------------------------------------------------------
{
	// Request shutdown
	mInternals->mShutdownRequested = true;

	// Signal if waiting
	mInternals->mSemaphore.signal();

	// Wait until is no lonnger running
	while (getIsRunning())
		// Sleep
		CThread::sleepFor(0.001);
}
