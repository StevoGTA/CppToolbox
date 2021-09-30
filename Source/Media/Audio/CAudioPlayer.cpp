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
// MARK: - CAudioPlayerBufferThreadInternals

class CAudioPlayerBufferThreadInternals {
	public:
		enum State {
			kStateStarting,
			kStateWaiting,
			kStateFilling,
		};

				CAudioPlayerBufferThreadInternals(CAudioPlayer& audioPlayer, CSRSWBIPSegmentedQueue& queue,
						UInt32 bytesPerFrame, UInt32 maxOutputFrames, CAudioPlayerBufferThread::ErrorProc errorProc,
						void* procsUserData) :
					mAudioPlayer(audioPlayer), mErrorProc(errorProc), mQueue(queue), mBytesPerFrame(bytesPerFrame),
							mMaxOutputFrames(maxOutputFrames), mProcsUserData(procsUserData),
							mResumeRequested(false), mStopFillingRequested(false),
							mShutdownRequested(false), mReachedEnd(false), mState(kStateStarting),
							mMediaPosition(SMediaPosition::fromStart(0.0)), mFramesToRead(~0)
					{}

		bool	tryFill()
					{
						// Check if have frames to read
						if (mFramesToRead == 0)
							// No frames to read
							return false;

						// Request write
						UInt32									bytesToRequest =
																		std::min<UInt32>(mFramesToRead,
																						mMaxOutputFrames * 2) *
																				mBytesPerFrame;
						CSRSWBIPSegmentedQueue::WriteBufferInfo	writeBufferInfo = mQueue.requestWrite(bytesToRequest);
						if (writeBufferInfo.mSize < mBytesPerFrame)
							// No space
							return false;

						// If the media position is "from start", we queue 2 * max output frames to get us started.
						//	Subsequently (i.e. if media position is "from current"), we try to fill the buffer as much
						//	as possible.
						UInt32	bytesToRead =
										(mMediaPosition.getMode() == SMediaPosition::kFromStart) ?
												bytesToRequest : writeBufferInfo.mSize;
						if ((bytesToRead / mBytesPerFrame) > mFramesToRead)
							// Limit to the frames to read
							bytesToRead = mFramesToRead * mBytesPerFrame;

						// Perform read
						CAudioFrames		audioFrames(writeBufferInfo.mBuffer, mQueue.getSegmentCount(),
													writeBufferInfo.mSegmentSize / mBytesPerFrame,
													bytesToRead / mBytesPerFrame, mBytesPerFrame);
						SAudioSourceStatus	audioSourceStatus = mAudioPlayer.perform(mMediaPosition, audioFrames);
						if (audioSourceStatus.isSuccess()) {
							// Success
							mQueue.commitWrite(audioFrames.getCurrentFrameCount() * mBytesPerFrame);
							mMediaPosition = SMediaPosition::fromCurrent();
							mFramesToRead -= audioFrames.getCurrentFrameCount();
							if (mFramesToRead == 0)
								// Reached the end
								mReachedEnd = true;

							return true;
						} else {
							// Finished or error
							mReachedEnd = true;
							
							if (*audioSourceStatus.getError() != SError::mEndOfData)
								// Error
								mErrorProc(*audioSourceStatus.getError(), mProcsUserData);

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

		bool								mResumeRequested;
		bool								mStopFillingRequested;
		bool								mShutdownRequested;
		bool								mReachedEnd;
		State								mState;
		SMediaPosition						mMediaPosition;
		UInt32								mFramesToRead;
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
	mInternals =
			new CAudioPlayerBufferThreadInternals(audioPlayer, queue, bytesPerFrame, maxOutputFrames, errorProc,
					procsUserData);

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
			case CAudioPlayerBufferThreadInternals::kStateStarting:
				// Starting
				if (mInternals->mResumeRequested) {
					// Start filling
					mInternals->mState = CAudioPlayerBufferThreadInternals::kStateFilling;
					break;
				} else {
					// Wait for resume
					mInternals->mState = CAudioPlayerBufferThreadInternals::kStateWaiting;

					// Fall through
				}

			case CAudioPlayerBufferThreadInternals::kStateWaiting:
				// Waiting
				mInternals->mSemaphore.waitFor();

				// Check if stop filling requested
				if (!mInternals->mStopFillingRequested)
					// Go for filling
					mInternals->mState = CAudioPlayerBufferThreadInternals::kStateFilling;
				break;

			case CAudioPlayerBufferThreadInternals::kStateFilling:
				// Filling
				if (mInternals->mStopFillingRequested || !mInternals->tryFill())
					// Go to waiting
					mInternals->mState = CAudioPlayerBufferThreadInternals::kStateWaiting;
				break;
		}
	}
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayerBufferThread::seek(UniversalTimeInterval timeInterval, UInt32 maxFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update
	mInternals->mMediaPosition = SMediaPosition::fromStart(timeInterval);
	mInternals->mFramesToRead = maxFrames;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayerBufferThread::resume()
//----------------------------------------------------------------------------------------------------------------------
{
	// Update
	mInternals->mResumeRequested = true;
	mInternals->mStopFillingRequested = false;
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
void CAudioPlayerBufferThread::stopReading()
//----------------------------------------------------------------------------------------------------------------------
{
	// Request stop filling
	mInternals->mStopFillingRequested = true;

	// Wait until waiting
	while (mInternals->mState != CAudioPlayerBufferThreadInternals::kStateWaiting)
		// Sleep
		CThread::sleepFor(0.001);
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayerBufferThread::shutdown()
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
