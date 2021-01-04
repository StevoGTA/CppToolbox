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
						// Setup
// Not sure yet the best way to approach this...
//	A) Just use maxOutputFrameCount
//	B) Use some multiple of maxOutputFrameCount (like 4)
//	C) Use a constant like 4096
// -We want the first read to go fast enough that we have the initial audio data ASAP, so not too big
// -But we also want to be efficient with system resources, so not too small
						UInt32	framesToRead = std::min<UInt32>(mMaxOutputFrames * 4, mFramesToRead);
						UInt32	bytesToRead = framesToRead * mBytesPerFrame;

						// Request write
						CSRSWBIPSegmentedQueue::WriteBufferInfo	writeBufferInfo = mQueue.requestWrite(bytesToRead);
						if (!writeBufferInfo.hasBuffer() || (writeBufferInfo.mSize < mBytesPerFrame))
							// No space
							return false;

						// Perform read
						CAudioData			audioData(writeBufferInfo.mBuffer, mQueue.getSegmentCount(),
													writeBufferInfo.mSegmentSize / mBytesPerFrame,
													writeBufferInfo.mSize / mBytesPerFrame, mBytesPerFrame);
						SAudioReadStatus	audioReadStatus = mAudioPlayer.perform(mMediaPosition, audioData);
						if (audioReadStatus.isSuccess()) {
							// Success
							mQueue.commitWrite(audioData.getCurrentFrameCount() * mBytesPerFrame);
							mMediaPosition = SMediaPosition::fromCurrent();
							mFramesToRead -= audioData.getCurrentFrameCount();

							return true;
						} else {
							// Finished
							mReachedEndOfData = true;
							if (*audioReadStatus.getError() != SError::mEndOfData)
								// Error
								mErrorProc(*audioReadStatus.getError(), mProcsUserData);

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
