//----------------------------------------------------------------------------------------------------------------------
//	CMediaPlayer.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMediaPlayer.h"

#include "CAudioPlayer.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaPlayerAudioPlayer

class CMediaPlayerAudioPlayer : public CAudioPlayer {
	public:
		CMediaPlayerAudioPlayer(const CString& identifier, const Info& info) :
			CAudioPlayer(identifier, info), mMessageQueue(10 * 1024)
			{}

		CSRSWMessageQueue	mMessageQueue;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaPlayerVideoFrameStore

class CMediaPlayerVideoFrameStore : public CVideoFrameStore {
	public:
		CMediaPlayerVideoFrameStore(const CString& identifier, const Info& info) :
			CVideoFrameStore(identifier, info), mMessageQueue(10 * 1024)
			{}

		CSRSWMessageQueue	mMessageQueue;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaPlayerInternals

class CMediaPlayerInternals {
	public:
		struct AudioPlayerPositionUpdatedMessage : public CSRSWMessageQueue::ProcMessage {
			// Lifecycle Methods
			AudioPlayerPositionUpdatedMessage(Proc proc, void* userData, const CAudioPlayer& audioPlayer,
					UniversalTimeInterval position) :
				CSRSWMessageQueue::ProcMessage(sizeof(AudioPlayerPositionUpdatedMessage), proc, userData),
						mAudioPlayer(audioPlayer), mPosition(position)
				{}

			// Properties
			const	CAudioPlayer&			mAudioPlayer;
					UniversalTimeInterval	mPosition;
		};

		struct AudioPlayerEndOfDataMessage : public CSRSWMessageQueue::ProcMessage {
			// Lifecycle Methods
			AudioPlayerEndOfDataMessage(Proc proc, void* userData, const CAudioPlayer& audioPlayer) :
				CSRSWMessageQueue::ProcMessage(sizeof(AudioPlayerEndOfDataMessage), proc, userData),
						mAudioPlayer(audioPlayer)
				{}

			// Properties
			const	CAudioPlayer&	mAudioPlayer;
		};

		struct AudioPlayerErrorMessage : public CSRSWMessageQueue::ProcMessage {
			// Lifecycle Methods
			AudioPlayerErrorMessage(Proc proc, void* userData, const CAudioPlayer& audioPlayer,
					const SError& error) :
				CSRSWMessageQueue::ProcMessage(sizeof(AudioPlayerErrorMessage), proc, userData),
						mAudioPlayer(audioPlayer), mError(error)
				{}

			// Properties
			const	CAudioPlayer&	mAudioPlayer;
					SError			mError;
		};

		struct VideoFrameStoreErrorMessage : public CSRSWMessageQueue::ProcMessage {
			// Lifecycle Methods
			VideoFrameStoreErrorMessage(Proc proc, void* userData, const CVideoFrameStore& videoFrameStore,
					const SError& error) :
				CSRSWMessageQueue::ProcMessage(sizeof(VideoFrameStoreErrorMessage), proc, userData),
						mVideoFrameStore(videoFrameStore), mError(error)
				{}

			// Properties
			const	CVideoFrameStore&	mVideoFrameStore;
					SError				mError;
		};

						CMediaPlayerInternals(CMediaPlayer& mediaPlayer, CSRSWMessageQueues& messageQueues,
								const CMediaPlayer::Info& info) :
							mMediaPlayer(mediaPlayer), mMessageQueues(messageQueues), mInfo(info),
									mCurrentPosition(0.0), mEndOfDataCount(0), mCurrentLoopCount(0)
							{}

		static	void	audioPlayerPositionUpdated(const CAudioPlayer& audioPlayer, UniversalTimeInterval position,
								void* userData)
							{
								// Queue message
								((CMediaPlayerAudioPlayer&) audioPlayer).mMessageQueue.submit(
										AudioPlayerPositionUpdatedMessage(handleAudioPlayerPositionUpdated, userData,
												audioPlayer, position));
							}
		static	void	handleAudioPlayerPositionUpdated(CSRSWMessageQueue::ProcMessage& message, void* userData)
							{
								// Setup
								AudioPlayerPositionUpdatedMessage&	audioPlayerPositionUpdatedMessage =
																			(AudioPlayerPositionUpdatedMessage&)
																					message;
								CMediaPlayerInternals&				internals = *((CMediaPlayerInternals*) userData);
								if (!mActiveInternals.contains(internals))
									return;

								// Store
								internals.mCurrentPosition = audioPlayerPositionUpdatedMessage.mPosition;

								// Iterate all video frame stores
								for (UInt32 i = 0; i < internals.mMediaPlayer.getVideoTrackCount(); i++)
									// Update video decoder
									internals.mMediaPlayer.getVideoProcessor(i)->notePositionUpdated(
											internals.mCurrentPosition);

								// Call proc
								internals.mInfo.audioPositionUpdated(audioPlayerPositionUpdatedMessage.mPosition);
							}
		static	void	audioPlayerEndOfData(const CAudioPlayer& audioPlayer, void* userData)
							{
								// Submit
								((CMediaPlayerAudioPlayer&) audioPlayer).mMessageQueue.submit(
										AudioPlayerEndOfDataMessage(handleAudioPlayerEndOfData, userData, audioPlayer));
							}
		static	void	handleAudioPlayerEndOfData(CSRSWMessageQueue::ProcMessage& message, void* userData)
							{
								// Setup
								CMediaPlayerInternals&	internals = *((CMediaPlayerInternals*) userData);
								if (!mActiveInternals.contains(internals))
									return;

								// One more at end
								if (++internals.mEndOfDataCount == internals.mMediaPlayer.getAudioTrackCount()) {
									// Reset
									for (UInt32 i = 0; i < internals.mMediaPlayer.getAudioTrackCount(); i++)
										// Reset
										internals.mMediaPlayer.getAudioProcessor(i)->reset();
									for (UInt32 i = 0; i < internals.mMediaPlayer.getVideoTrackCount(); i++)
										// Reset
										internals.mMediaPlayer.getVideoProcessor(i)->reset();

									// All at the end
									internals.mCurrentLoopCount++;

									// Check if have loop count
									if (internals.mLoopCount.hasValue() &&
											((internals.mLoopCount.getValue() == 0) ||
													(internals.mCurrentLoopCount < internals.mLoopCount.getValue()))) {
										// Reset and go again
										internals.mEndOfDataCount = 0;

										// Start playback for all audio players
										for (UInt32 i = 0; i < internals.mMediaPlayer.getAudioTrackCount(); i++)
											// Play
											internals.mMediaPlayer.getAudioProcessor(i)->play();
									} else {
										// Finished
										internals.mEndOfDataCount = 0;

										// Call proc
										internals.mInfo.finished();
									}
								}
							}
		static	void	audioPlayerError(const CAudioPlayer& audioPlayer, const SError& error, void* userData)
							{
								// Submit
								((CMediaPlayerAudioPlayer&) audioPlayer).mMessageQueue.submit(
										AudioPlayerErrorMessage(handleAudioPlayerError, userData, audioPlayer, error));
							}
		static	void	handleAudioPlayerError(CSRSWMessageQueue::ProcMessage& message, void* userData)
							{
								// Setup
								CMediaPlayerInternals&		internals = *((CMediaPlayerInternals*) userData);
								AudioPlayerErrorMessage&	errorMessage = (AudioPlayerErrorMessage&) message;
								if (!mActiveInternals.contains(internals))
									return;

								// Handle
								internals.mInfo.audioError(errorMessage.mError);
							}

		static	void	videoFrameStoreCurrentFrameUpdated(const CVideoFrameStore& videoFrameStore,
								const CVideoFrame& videoFrame, void* userData)
							{
								// Setup
								CMediaPlayerInternals&	internals = *((CMediaPlayerInternals*) userData);

								// Handle
								internals.mInfo.videoFrameUpdated(videoFrame);
							}
		static	void	videoFrameStoreError(const CVideoFrameStore& videoFrameStore, const SError& error,
								void* userData)
							{
								// Submit
								((CMediaPlayerVideoFrameStore&) videoFrameStore).mMessageQueue.submit(
										VideoFrameStoreErrorMessage(handleVideoFrameStoreError, userData,
												videoFrameStore, error));
							}
		static	void	handleVideoFrameStoreError(CSRSWMessageQueue::ProcMessage& message, void* userData)
							{
								// Setup
								CMediaPlayerInternals&			internals = *((CMediaPlayerInternals*) userData);
								VideoFrameStoreErrorMessage&	errorMessage = (VideoFrameStoreErrorMessage&) message;
								if (!mActiveInternals.contains(internals))
									return;

								// Handle
								internals.mInfo.videoError(errorMessage.mError);
							}

				CMediaPlayer&					mMediaPlayer;
				CSRSWMessageQueues&				mMessageQueues;
				CMediaPlayer::Info				mInfo;

				UniversalTimeInterval			mCurrentPosition;
				UInt32							mEndOfDataCount;
				OV<UInt32>						mLoopCount;
				UInt32							mCurrentLoopCount;

		static	TNArray<R<CMediaPlayerInternals> >	mActiveInternals;
};

TNArray<R<CMediaPlayerInternals> >	CMediaPlayerInternals::mActiveInternals;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaPlayer

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CMediaPlayer::CMediaPlayer(CSRSWMessageQueues& messageQueues, const Info& info)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CMediaPlayerInternals(*this, messageQueues, info);

	// Add
	CMediaPlayerInternals::mActiveInternals += R<CMediaPlayerInternals>(*mInternals);
}

//----------------------------------------------------------------------------------------------------------------------
CMediaPlayer::~CMediaPlayer()
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove
	CMediaPlayerInternals::mActiveInternals -= R<CMediaPlayerInternals>(*mInternals);

	// Reset
	reset();

	// Cleanup
	for (UInt32 i = 0; i < getAudioTrackCount(); i++) {
		// Remove message queue
		CMediaPlayerAudioPlayer&	audioPlayer = (CMediaPlayerAudioPlayer&) *getAudioProcessor(i);
		mInternals->mMessageQueues.remove(audioPlayer.mMessageQueue);
	}
	for (UInt32 i = 0; i < getVideoTrackCount(); i++) {
		// Remove message queue
		CMediaPlayerVideoFrameStore&	videoFrameStore = (CMediaPlayerVideoFrameStore&) *getVideoProcessor(i);
		mInternals->mMessageQueues.remove(videoFrameStore.mMessageQueue);
	}

	// Cleanup
	Delete(mInternals);
}

// MARK: CMediaDestination methods

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlayer::setupComplete()
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all audio tracks
	for (UInt32 i = 0; i < getAudioTrackCount(); i++)
		// Note setup is complete
		getAudioProcessor(i)->setupComplete();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
I<CAudioPlayer> CMediaPlayer::newAudioPlayer(const CString& identifier, UInt32 trackIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create Audio Player
	I<CMediaPlayerAudioPlayer>	audioPlayer(
										new CMediaPlayerAudioPlayer(identifier,
												CAudioPlayer::Info(CMediaPlayerInternals::audioPlayerPositionUpdated,
														CMediaPlayerInternals::audioPlayerEndOfData,
														CMediaPlayerInternals::audioPlayerError, mInternals)));

	// Add message queue
	mInternals->mMessageQueues.add(audioPlayer->mMessageQueue);

	// Add
	add((const I<CAudioProcessor>&) audioPlayer, trackIndex);

	return *((I<CAudioPlayer>*) &audioPlayer);
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlayer::setAudioGain(Float32 audioGain)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all audio tracks
	for (UInt32 i = 0; i < getAudioTrackCount(); i++)
		// Set audio gain to this audio track
		getAudioProcessor(i)->setGain(audioGain);
}

//----------------------------------------------------------------------------------------------------------------------
I<CVideoFrameStore> CMediaPlayer::newVideoFrameStore(const CString& identifier, UInt32 trackIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create Video Frame Store
	I<CMediaPlayerVideoFrameStore>	videoFrameStore(
											new CMediaPlayerVideoFrameStore(identifier,
													CVideoFrameStore::Info(
															CMediaPlayerInternals::videoFrameStoreCurrentFrameUpdated,
															CMediaPlayerInternals::videoFrameStoreError,
															mInternals)));

	// Add message queue
	mInternals->mMessageQueues.add(videoFrameStore->mMessageQueue);

	// Add
	add((const I<CVideoProcessor>&) videoFrameStore, trackIndex);

	return *((I<CVideoFrameStore>*) &videoFrameStore);
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlayer::setLoopCount(OV<UInt32> loopCount)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mLoopCount = loopCount;
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTimeInterval CMediaPlayer::getCurrentPosition() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mCurrentPosition;
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlayer::play()
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all audio tracks
	for (UInt32 i = 0; i < getAudioTrackCount(); i++)
		// Play
		getAudioProcessor(i)->play();

	// Iterate all video tracks
	for (UInt32 i = 0; i < getVideoTrackCount(); i++)
		// Resume
		getVideoProcessor(i)->resume();
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlayer::pause()
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all audio tracks
	for (UInt32 i = 0; i < getAudioTrackCount(); i++)
		// Pause
		getAudioProcessor(i)->pause();

	// Iterate all video tracks
	for (UInt32 i = 0; i < getVideoTrackCount(); i++)
		// Pause
		getVideoProcessor(i)->pause();
}

//----------------------------------------------------------------------------------------------------------------------
bool CMediaPlayer::isPlaying() const
//----------------------------------------------------------------------------------------------------------------------
{
	// We are playing if any track is playing
	for (UInt32 i = 0; i < getAudioTrackCount(); i++) {
		// Check if playing
		if (getAudioProcessor(i)->isPlaying())
			// Is playing
			return true;
	}

	return false;
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlayer::startSeek()
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all audio tracks
	for (UInt32 i = 0; i < getAudioTrackCount(); i++)
		// Start seek
		getAudioProcessor(i)->startSeek();

	// Iterate all video tracks
	for (UInt32 i = 0; i < getVideoTrackCount(); i++)
		// Start seek
		getVideoProcessor(i)->startSeek();
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlayer::finishSeek()
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all audio tracks
	for (UInt32 i = 0; i < getAudioTrackCount(); i++)
		// Finish seek
		getAudioProcessor(i)->finishSeek();

	// Iterate all video tracks
	for (UInt32 i = 0; i < getVideoTrackCount(); i++)
		// Finish seek
		getVideoProcessor(i)->finishSeek();
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlayer::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all audio tracks
	for (UInt32 i = 0; i < getAudioTrackCount(); i++)
		// Reset
		getAudioProcessor(i)->reset();

	// Iterate all video tracks
	for (UInt32 i = 0; i < getVideoTrackCount(); i++)
		// Reset
		getVideoProcessor(i)->reset();

	// Update internals
	mInternals->mCurrentPosition = 0.0;
	mInternals->mEndOfDataCount = 0;
}
