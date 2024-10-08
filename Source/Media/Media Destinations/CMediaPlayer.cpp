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
// MARK: - CMediaPlayer::Internals

class CMediaPlayer::Internals {
	public:
		class AudioPlayerPositionUpdatedMessage : public CSRSWMessageQueue::ProcMessage {
			public:
				AudioPlayerPositionUpdatedMessage(Proc proc, void* userData, const CAudioPlayer& audioPlayer,
						UniversalTimeInterval position) :
					CSRSWMessageQueue::ProcMessage(sizeof(AudioPlayerPositionUpdatedMessage), proc, userData),
							mAudioPlayer(audioPlayer), mPosition(position)
					{}

				const	CAudioPlayer&			mAudioPlayer;
						UniversalTimeInterval	mPosition;
		};

		class AudioPlayerEndOfDataMessage : public CSRSWMessageQueue::ProcMessage {
			public:
				AudioPlayerEndOfDataMessage(Proc proc, void* userData, const CAudioPlayer& audioPlayer) :
					CSRSWMessageQueue::ProcMessage(sizeof(AudioPlayerEndOfDataMessage), proc, userData),
							mAudioPlayer(audioPlayer)
					{}

				const	CAudioPlayer&	mAudioPlayer;
		};

		class AudioPlayerErrorMessage : public CSRSWMessageQueue::ProcMessage {
			public:
				AudioPlayerErrorMessage(Proc proc, void* userData, const CAudioPlayer& audioPlayer,
						const SError& error) :
					CSRSWMessageQueue::ProcMessage(sizeof(AudioPlayerErrorMessage), proc, userData),
							mAudioPlayer(audioPlayer), mError(error)
					{}

				const	CAudioPlayer&	mAudioPlayer;
						SError			mError;
		};

		class VideoFrameStoreErrorMessage : public CSRSWMessageQueue::ProcMessage {
			public:
				VideoFrameStoreErrorMessage(Proc proc, void* userData, const CVideoFrameStore& videoFrameStore,
						const SError& error) :
					CSRSWMessageQueue::ProcMessage(sizeof(VideoFrameStoreErrorMessage), proc, userData),
							mVideoFrameStore(videoFrameStore), mError(error)
					{}

				const	CVideoFrameStore&	mVideoFrameStore;
						SError				mError;
		};

						Internals(CMediaPlayer& mediaPlayer, CSRSWMessageQueues& messageQueues,
								const CMediaPlayer::Info& info) :
							mMediaPlayer(mediaPlayer), mMessageQueues(messageQueues), mInfo(info),
									mCurrentTimeInterval(0.0), mEndOfDataCount(0), mCurrentLoopCount(0)
							{}

		static	void	audioPlayerPositionUpdated(const CAudioPlayer& audioPlayer, UniversalTimeInterval position,
								void* userData)
							{
								// Queue message
								((CMediaPlayerAudioPlayer&) audioPlayer).mMessageQueue.submit(
										AudioPlayerPositionUpdatedMessage(
												(CSRSWMessageQueue::ProcMessage::Proc) handleAudioPlayerPositionUpdated,
												userData, audioPlayer, position));
							}
		static	void	handleAudioPlayerPositionUpdated(
								AudioPlayerPositionUpdatedMessage& audioPlayerPositionUpdatedMessage,
								Internals* internals)
							{
								// Setup
								if (!mActiveInternals.contains(*internals))
									return;

								// Update
								internals->mCurrentTimeInterval = audioPlayerPositionUpdatedMessage.mPosition;

								// Iterate all video frame stores
								for (UInt32 i = 0; i < internals->mMediaPlayer.getVideoTrackCount(); i++)
									// Update video decoder
									internals->mMediaPlayer.getVideoDestination(i)->notePositionUpdated(
											internals->mCurrentTimeInterval);

								// Call proc
								internals->mInfo.audioPositionUpdated(audioPlayerPositionUpdatedMessage.mPosition);
							}
		static	void	audioPlayerEndOfData(const CAudioPlayer& audioPlayer, void* userData)
							{
								// Submit
								((CMediaPlayerAudioPlayer&) audioPlayer).mMessageQueue.submit(
										AudioPlayerEndOfDataMessage(
												(CSRSWMessageQueue::ProcMessage::Proc) handleAudioPlayerEndOfData,
												userData, audioPlayer));
							}
		static	void	handleAudioPlayerEndOfData(CSRSWMessageQueue::ProcMessage& message, Internals* internals)
							{
								// Setup
								if (!mActiveInternals.contains(*internals))
									return;

								// One more at end
								if (++internals->mEndOfDataCount == internals->mMediaPlayer.getAudioTrackCount()) {
									// All at the end
									internals->mCurrentLoopCount++;

									// Check if have loop count
									if (internals->mLoopCount.hasValue() &&
											((*internals->mLoopCount == 0) ||
													(internals->mCurrentLoopCount < *internals->mLoopCount))) {
										// Reset and go again
										internals->mEndOfDataCount = 0;

										// Reset and start playback again
										for (UInt32 i = 0; i < internals->mMediaPlayer.getAudioTrackCount(); i++) {
											// Reset and play
											internals->mMediaPlayer.getAudioDestination(i)->reset();
											internals->mMediaPlayer.getAudioDestination(i)->play();
										}
										for (UInt32 i = 0; i < internals->mMediaPlayer.getVideoTrackCount(); i++) {
											// Reset and resume
											internals->mMediaPlayer.getVideoDestination(i)->reset();
											internals->mMediaPlayer.getVideoDestination(i)->resume();
										}
									} else {
										// Finished
										internals->mEndOfDataCount = 0;

										// Call proc
										internals->mInfo.finished();
									}
								}
							}
		static	void	audioPlayerError(const CAudioPlayer& audioPlayer, const SError& error, void* userData)
							{
								// Submit
								((CMediaPlayerAudioPlayer&) audioPlayer).mMessageQueue.submit(
										AudioPlayerErrorMessage(
												(CSRSWMessageQueue::ProcMessage::Proc) handleAudioPlayerError, userData,
												audioPlayer, error));
							}
		static	void	handleAudioPlayerError(CSRSWMessageQueue::ProcMessage& message, Internals* internals)
							{
								// Setup
								AudioPlayerErrorMessage&	errorMessage = (AudioPlayerErrorMessage&) message;
								if (!mActiveInternals.contains(*internals))
									return;

								// Handle
								internals->mInfo.audioError(errorMessage.mError);
							}

		static	void	videoFrameStoreCurrentFrameUpdated(const CVideoFrameStore& videoFrameStore,
								const CVideoFrame& videoFrame, Internals* internals)
							{
								// Update
								internals->mCurrentFrameIndex.setValue(videoFrame.getIndex());

								// Handle
								internals->mInfo.videoFrameUpdated(videoFrame);
							}
		static	void	videoFrameStoreError(const CVideoFrameStore& videoFrameStore, const SError& error,
								void* userData)
							{
								// Submit
								((CMediaPlayerVideoFrameStore&) videoFrameStore).mMessageQueue.submit(
										VideoFrameStoreErrorMessage(
												(CSRSWMessageQueue::ProcMessage::Proc) handleVideoFrameStoreError,
												userData, videoFrameStore, error));
							}
		static	void	handleVideoFrameStoreError(CSRSWMessageQueue::ProcMessage& message, Internals* internals)
							{
								// Setup
								VideoFrameStoreErrorMessage&	errorMessage = (VideoFrameStoreErrorMessage&) message;
								if (!mActiveInternals.contains(*internals))
									return;

								// Handle
								internals->mInfo.videoError(errorMessage.mError);
							}

				CMediaPlayer&			mMediaPlayer;
				CSRSWMessageQueues&		mMessageQueues;
				CMediaPlayer::Info		mInfo;

				OV<SMedia::Segment>		mMediaSegment;
				UniversalTimeInterval	mCurrentTimeInterval;
				OV<UInt32>				mCurrentFrameIndex;
				UInt32					mEndOfDataCount;
				OV<UInt32>				mLoopCount;
				UInt32					mCurrentLoopCount;

		static	TNArray<R<Internals> >	mActiveInternals;
};

TNArray<R<CMediaPlayer::Internals> >	CMediaPlayer::Internals::mActiveInternals;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaPlayer

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CMediaPlayer::CMediaPlayer(CSRSWMessageQueues& messageQueues, const Info& info) :
		TMediaDestination<CAudioPlayer, CVideoFrameStore>(CString(OSSTR("Media Player")))
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals(*this, messageQueues, info);

	// Add
	Internals::mActiveInternals += R<Internals>(*mInternals);
}

//----------------------------------------------------------------------------------------------------------------------
CMediaPlayer::~CMediaPlayer()
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove
	Internals::mActiveInternals -= R<Internals>(*mInternals);

	// Cleanup
	for (UInt32 i = 0; i < getAudioTrackCount(); i++) {
		// Remove message queue
		CMediaPlayerAudioPlayer&	audioPlayer = (CMediaPlayerAudioPlayer&) *getAudioDestination(i);
		mInternals->mMessageQueues.remove(audioPlayer.mMessageQueue);
	}
	removeAllAudioDestinations();

	for (UInt32 i = 0; i < getVideoTrackCount(); i++) {
		// Remove message queue
		CMediaPlayerVideoFrameStore&	videoFrameStore = (CMediaPlayerVideoFrameStore&) *getVideoDestination(i);
		mInternals->mMessageQueues.remove(videoFrameStore.mMessageQueue);
	}
	removeAllVideoDestinations();

	// Cleanup
	Delete(mInternals);
}

// MARK: CMediaDestination methods

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlayer::add(const I<CAudioDestination>& audioDestination, UInt32 trackIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Do super
	TMediaDestination<CAudioPlayer, CVideoFrameStore>::add(audioDestination, trackIndex);

	// Add message queue
	mInternals->mMessageQueues.add(((const I<CMediaPlayerAudioPlayer>&) audioDestination)->mMessageQueue);
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlayer::add(const I<CVideoDestination>& videoDestination, UInt32 trackIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Do super
	TMediaDestination<CAudioPlayer, CVideoFrameStore>::add(videoDestination, trackIndex);

	// Add message queue
	mInternals->mMessageQueues.add(((const I<CMediaPlayerVideoFrameStore>&) videoDestination)->mMessageQueue);
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlayer::setMediaSegment(const OV<SMedia::Segment>& mediaSegment)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	bool	performSeek =
					mediaSegment.hasValue() &&
							(mediaSegment->getStartTimeInterval() != mInternals->mCurrentTimeInterval);

	// Check Media Segment
	if (mediaSegment.hasValue() && (mediaSegment->getDurationTimeInterval() > 0.0))
		// Store
		mInternals->mMediaSegment = mediaSegment;
	else
		// Clear
		mInternals->mMediaSegment.removeValue();

	// Do super
	CMediaDestination::setMediaSegment(mInternals->mMediaSegment);

	// Check for seek
	if (performSeek)
		// Seek
		seek(mediaSegment->getStartTimeInterval());
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlayer::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Do super
	TMediaDestination<CAudioPlayer, CVideoFrameStore>::seek(timeInterval);

	// Store
	mInternals->mCurrentTimeInterval = timeInterval;
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
I<CAudioPlayer> CMediaPlayer::newAudioPlayer(const CString& identifier, UInt32 trackIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	return I<CAudioPlayer>(
			new CMediaPlayerAudioPlayer(identifier,
					CAudioPlayer::Info(Internals::audioPlayerPositionUpdated, Internals::audioPlayerEndOfData,
							Internals::audioPlayerError, mInternals)));
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlayer::setAudioGain(Float32 audioGain)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all audio tracks
	for (UInt32 i = 0; i < getAudioTrackCount(); i++)
		// Set audio gain to this audio track
		getAudioDestination(i)->setGain(audioGain);
}

//----------------------------------------------------------------------------------------------------------------------
I<CVideoFrameStore> CMediaPlayer::newVideoFrameStore(const CString& identifier, UInt32 trackIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	return I<CVideoFrameStore>(
			new CMediaPlayerVideoFrameStore(identifier,
					CVideoFrameStore::Info(
							(CVideoFrameStore::Info::CurrentFrameUpdatedProc)
									Internals::videoFrameStoreCurrentFrameUpdated,
							Internals::videoFrameStoreError, mInternals)));
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlayer::setLoopCount(OV<UInt32> loopCount)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mLoopCount = loopCount;
}

//----------------------------------------------------------------------------------------------------------------------
const OV<SMedia::Segment>& CMediaPlayer::getMediaSegment() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mMediaSegment;
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTimeInterval CMediaPlayer::getCurrentTimeInterval() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mCurrentTimeInterval;
}

//----------------------------------------------------------------------------------------------------------------------
const OV<UInt32>& CMediaPlayer::getCurrentFrameIndex() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mCurrentFrameIndex;
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlayer::play()
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all audio tracks
	for (UInt32 i = 0; i < getAudioTrackCount(); i++)
		// Play
		getAudioDestination(i)->play();

	// Iterate all video tracks
	for (UInt32 i = 0; i < getVideoTrackCount(); i++)
		// Resume
		getVideoDestination(i)->resume();
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlayer::pause()
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all audio tracks
	for (UInt32 i = 0; i < getAudioTrackCount(); i++)
		// Pause
		getAudioDestination(i)->pause();

	// Iterate all video tracks
	for (UInt32 i = 0; i < getVideoTrackCount(); i++)
		// Pause
		getVideoDestination(i)->pause();
}

//----------------------------------------------------------------------------------------------------------------------
bool CMediaPlayer::isPlaying() const
//----------------------------------------------------------------------------------------------------------------------
{
	// We are playing if any track is playing
	for (UInt32 i = 0; i < getAudioTrackCount(); i++) {
		// Check if playing
		if (getAudioDestination(i)->isPlaying())
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
		getAudioDestination(i)->startSeek();

	// Iterate all video tracks
	for (UInt32 i = 0; i < getVideoTrackCount(); i++)
		// Start seek
		getVideoDestination(i)->startSeek();
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlayer::finishSeek()
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all audio tracks
	for (UInt32 i = 0; i < getAudioTrackCount(); i++)
		// Finish seek
		getAudioDestination(i)->finishSeek();

	// Iterate all video tracks
	for (UInt32 i = 0; i < getVideoTrackCount(); i++)
		// Finish seek
		getVideoDestination(i)->finishSeek();
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlayer::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all audio tracks
	for (UInt32 i = 0; i < getAudioTrackCount(); i++)
		// Reset
		getAudioDestination(i)->reset();

	// Iterate all video tracks
	for (UInt32 i = 0; i < getVideoTrackCount(); i++)
		// Reset
		getVideoDestination(i)->reset();

	// Update internals
	mInternals->mCurrentTimeInterval =
			mInternals->mMediaSegment.hasValue() ? mInternals->mMediaSegment->getStartTimeInterval() : 0.0;
	mInternals->mCurrentFrameIndex.removeValue();
	mInternals->mEndOfDataCount = 0;

	// Call proc
	mInternals->mInfo.audioPositionUpdated(mInternals->mCurrentTimeInterval);
}
