//----------------------------------------------------------------------------------------------------------------------
//	CMediaPlayer.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMediaPlayer.h"

#include "CAudioPlayer.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaPlayer implementation notes
/*
	The header (CMediaPlayer.h) is the behavioral contract - what a caller should expect.  These notes are how that is
	achieved, for someone maintaining this file.

	CMediaPlayer turns the low-level primitives of its CAudioPlayers (and CVideoFrameStores) into those flows.  The
	primitives it leans on (see CAudioPlayer):

		play()			render unbounded from the current buffered position; report positionUpdated as it advances;
						report endOfData -> finished() only when the source truly reaches its end.
		pause()			stop sending frames, keep position.
		seek(t)			flush, reposition the source to t, report positionUpdated(t).  While NOT playing it renders a
						short mPreviewDuration "burst" (an audition) from t; while playing it continues unbounded.  A
						burst that finishes mid-source goes silent WITHOUT reporting endOfData - reaching the actual
						source end is what reports it.  While seeking, the streaming position crawl is suppressed.
		startSeek()		enter preview mode: stop sending frames, remember the current position, and stop reporting
						endOfData (so an audition that runs off the source end is not mistaken for finishing).
		finishSeek()	flush, reposition to the last seek target, restore endOfData reporting, resume iff playing.

	Interactions come in two brackets, both wrapped by beginInteraction() / endInteraction():
		Playhead (kMovingPlayhead):
			startSeek()...
			seek(t) [0..n]...
			finishSeek()
		Segment (kChangingMediaSegment):
			mediaSegmentWillChange()...
			setMediaSegment(seg) [0..n]...
			mediaSegmentDidChange()

		beginInteraction() remembers whether real playback was active (mInteractionWasPlaying), then SUSPENDS it -
			startSeek()s the players AND pause()s them.  Paused, each subsequent seek() auditions a mPreviewDuration
			burst instead of continuing, the crawl is suppressed, and the audition's end is not reported.

		endInteraction(repositionToTarget) restores.  It re-seeks to the last auditioned target when repositionToTarget
			is set, or when NOT resuming (to park there).  Otherwise - resuming a playhead interaction, whose source
			scope is unchanged - it SKIPS the re-seek and simply play()s, letting the last audition flow straight into
			playback.  That is what avoids "double audio": re-seeking would replay the ~mPreviewDuration the audition
			just played.  Because play() resumes from the current buffered position and finishes only at the true source
			end, a mid-file audition continues to the end of the source, while an audition that reached the source end
			finishes (its tail heard once).

	setMediaSegment(seg):
		- Inside a segment interaction: record the RAW in-progress segment (normalized only on commit) and audition its
			start when the start moves, pushing a transient [start, mPreviewDuration] window straight to the source so
			the audition is never clipped by a momentarily-tiny selection.
		- Outside one (a direct/programmatic call): a positive-duration segment commits
			(CMediaDestination::setMediaSegment) then seek()s its start; a zero-duration point (a waveform click - a
			seek, not a selection) clears the segment then seek()s the point; no segment just clears, keeping the
			position.  seek() does the work - a preview burst when paused, a reposition-and-continue when playing - so
			no one-shot bracket is used here (a bracket's finishSeek() would flush the burst before it sounds).

		mediaSegmentDidChange() commits with endInteraction(true): the commit swaps the source scope out from under the
			audition (transient window -> committed segment), so the buffer must be reconciled with a re-seek.

	play() / pause() / seek() / startSeek() / stop() each first commit any open segment interaction
		(mediaSegmentDidChange()) as a safety net, so a stray transport/position call never operates on the transient
		preview window.

	stop() halts the players and returns the playhead to the segment start (the audio players release their engine
		slot and clear their playing flag).  restart() is the loop primitive: while playing, it seek()s to the segment
		start, so playback continues seamlessly from the top with no engine teardown - the players stay engaged, the
		reader's end-of-data latch is cleared by the seek's reader resume(), and, being a seek-while-playing, no preview
		burst sounds.

	Events (via Info) come straight from the players: positionUpdated -> audioPositionUpdated (only targets during a
		bracket, since the crawl is suppressed); endOfData -> aggregated into finished() (suppressed during a bracket).
*/

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local Data

enum InteractionState {
	kInteractionStateNone,
	kInteractionStateMovingPlayhead,
	kInteractionStateChangingMediaSegment,
};


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
									mCurrentTimeInterval(0.0), mEndOfDataCount(0), mCurrentLoopCount(0),
									mInteractionState(kInteractionStateNone), mInteractionWasPlaying(false)
							{}

				void	beginInteraction()
							{
								// Store is playing for use in endInteraction()
								mInteractionWasPlaying = mMediaPlayer.isPlaying();

								// Iterate all audio tracks
								for (UInt32 i = 0; i < mMediaPlayer.getAudioTrackCount(); i++) {
									// Start Seek
									mMediaPlayer.getAudioDestination(i)->startSeek();

									// Pause
									mMediaPlayer.getAudioDestination(i)->pause();
								}

								// Iterate all video tracks
								for (UInt32 i = 0; i < mMediaPlayer.getVideoTrackCount(); i++) {
									// Start Seek
									mMediaPlayer.getVideoDestination(i)->startSeek();

									// Pause
									mMediaPlayer.getVideoDestination(i)->pause();
								}
							}

				void	endInteraction(bool repositionToTarget)
							{
								// Check if need to finish seeking
								if (repositionToTarget || !mInteractionWasPlaying) {
									// Iterate all audio tracks
									for (UInt32 i = 0; i < mMediaPlayer.getAudioTrackCount(); i++)
										// Finish seek
										mMediaPlayer.getAudioDestination(i)->finishSeek();

									// Iterate all video tracks
									for (UInt32 i = 0; i < mMediaPlayer.getVideoTrackCount(); i++)
										// Finish seek
										mMediaPlayer.getVideoDestination(i)->finishSeek();
								}

								// Check if was playing before interaction
								if (mInteractionWasPlaying) {
									// Iterate all audio tracks
									for (UInt32 i = 0; i < mMediaPlayer.getAudioTrackCount(); i++)
										// Play
										mMediaPlayer.getAudioDestination(i)->play();

									// Iterate all video tracks
									for (UInt32 i = 0; i < mMediaPlayer.getVideoTrackCount(); i++)
										// Play
										mMediaPlayer.getVideoDestination(i)->resume();
								}
							}

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

								// End-of-data only means a finish while actually playing - a not-playing audition (a
								//	preview burst) can run off the source end too, and must not be mistaken for one.
								if (!internals->mMediaPlayer.isPlaying())
									return;

								// One more at end
								if (++internals->mEndOfDataCount == internals->mMediaPlayer.getAudioTrackCount()) {
									// All at the end
									internals->mCurrentLoopCount++;

									// Check if have loop count
									if (internals->mLoopCount.hasValue() &&
											((*internals->mLoopCount == 0) ||
													(internals->mCurrentLoopCount < *internals->mLoopCount))) {
										// Loop again
										internals->mEndOfDataCount = 0;

										// Seamlessly replay from the segment start (players stay engaged - no teardown)
										internals->mMediaPlayer.restart();
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

				UniversalTimeInterval	mCurrentTimeInterval;
				OV<UInt32>				mCurrentFrameIndex;
				UInt32					mEndOfDataCount;
				OV<UInt32>				mLoopCount;
				UInt32					mCurrentLoopCount;

				InteractionState		mInteractionState;
				OV<SMedia::Segment>		mPendingMediaSegment;
				bool					mInteractionWasPlaying;

		static	TNArray<R<Internals> >	mActiveInternals;
};

TNArray<R<CMediaPlayer::Internals> >	CMediaPlayer::Internals::mActiveInternals;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaPlayer::AudioSetup

// MARK: CAudioDestination::Setup methods

//----------------------------------------------------------------------------------------------------------------------
I<CAudioDestination> CMediaPlayer::AudioSetup::create(const CString& identifier, UInt32 trackIndex) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Create audio player
	I<CAudioPlayer>	audioPlayer =
							mMediaPlayer.newAudioPlayer(
									identifier + CString(OSSTR(", Audio Track ")) + CString(trackIndex + 1),
											trackIndex);

	return *((I<CAudioDestination>*) &audioPlayer);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaPlayer::VideoSetup

// MARK: CVideoDestination::Setup methods

//----------------------------------------------------------------------------------------------------------------------
I<CVideoDestination> CMediaPlayer::VideoSetup::create(const CString& identifier, UInt32 trackIndex) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Create video frame store
	I<CVideoFrameStore>	videoFrameStore =
							mMediaPlayer.newVideoFrameStore(
									identifier + CString(OSSTR(", Video Track ")) + CString(trackIndex + 1),
											trackIndex);

	return *((I<CVideoDestination>*) &videoFrameStore);
}

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
	// Check if current interaction is changing the media segment (a bracketed drag)
	if (mInternals->mInteractionState != kInteractionStateChangingMediaSegment) {
		// Apply change immediately - check situation
		OV<SMedia::Segment>	mediaSegmentUse =
									(mediaSegment.hasValue() && (mediaSegment->getDurationTimeInterval() > 0.0)) ?
											mediaSegment : OV<SMedia::Segment>();
		
		// Check situation
		if (mediaSegmentUse.hasValue()) {
			// A real (positive-duration) segment - set the media segment
			CMediaDestination::setMediaSegment(mediaSegmentUse);

			// Seek to media segment start
			seek(mediaSegmentUse->getStartTimeInterval());
		} else if (mediaSegment.hasValue()) {
			// A zero-duration segment could be the start of a selection , or a discrete playhead seek.  We don't know
			//	at this time, but we want to preview regardless.
			CMediaDestination::setMediaSegment(OV<SMedia::Segment>());
			seek(mediaSegment->getStartTimeInterval());
		} else
			// No segment: clear, keeping the current position.
			CMediaDestination::setMediaSegment(OV<SMedia::Segment>());
	} else {
		// In the middle of an interaction.
		OV<SMedia::Segment>	previousPendingMediaSegment = mInternals->mPendingMediaSegment;

		// Update pending media segment
		mInternals->mPendingMediaSegment = mediaSegment;

		// Check situation
		if (mediaSegment.hasValue() &&
				(!previousPendingMediaSegment.hasValue() ||
						(previousPendingMediaSegment->getStartTimeInterval() !=
								mediaSegment->getStartTimeInterval()))) {
			// The media segment start is changing - preview the new start.
			mInternals->mCurrentTimeInterval = mediaSegment->getStartTimeInterval();

			// Prepare to update all audio and video destinations
			SMedia::Segment	previewMediaSegment(mInternals->mCurrentTimeInterval, CAudioPlayer::mPreviewDuration);

			// Update all audio destinations
			for (UInt32 i = 0; i < getAudioTrackCount(); i++) {
				// Set the preview media segment
				getAudioDestination(i)->setMediaSegment(OV<SMedia::Segment>(previewMediaSegment));

				// Seek
				getAudioDestination(i)->seek(mInternals->mCurrentTimeInterval);
			}

			// Update all video destinations
			for (UInt32 i = 0; i < getVideoTrackCount(); i++) {
				// Set the preview media segment
				getVideoDestination(i)->setMediaSegment(OV<SMedia::Segment>(previewMediaSegment));

				// Seek
				getVideoDestination(i)->seek(mInternals->mCurrentTimeInterval);
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlayer::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Commit any in-progress media-segment interaction
	mediaSegmentDidChange();

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
	// Commit any in-progress media-segment interaction
	mediaSegmentDidChange();

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
	// Commit any in-progress media-segment interaction
	mediaSegmentDidChange();

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
	// Commit any in-progress media-segment interaction, then begin a playhead interaction (suspend + preview)
	mediaSegmentDidChange();

	mInternals->mInteractionState = kInteractionStateMovingPlayhead;
	mInternals->beginInteraction();
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlayer::finishSeek()
//----------------------------------------------------------------------------------------------------------------------
{
	// End the playhead interaction: resume seamlessly from the audition (no re-seek), or park at the target
	mInternals->endInteraction(false);

	mInternals->mInteractionState = kInteractionStateNone;
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlayer::mediaSegmentWillChange()
//----------------------------------------------------------------------------------------------------------------------
{
	// Idempotent - begin at most once per interaction
	if (mInternals->mInteractionState == kInteractionStateChangingMediaSegment)
		return;

	// Enter an interactive media-segment change: seed the in-progress segment from the committed one and put the
	//	players into seek/preview mode (which remembers whether playback was active, to be restored on commit)
	mInternals->mInteractionState = kInteractionStateChangingMediaSegment;
	mInternals->mPendingMediaSegment = getMediaSegment();
	mInternals->beginInteraction();
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlayer::mediaSegmentDidChange()
//----------------------------------------------------------------------------------------------------------------------
{
	// Idempotent - only commit an in-progress interaction
	if (mInternals->mInteractionState != kInteractionStateChangingMediaSegment)
		return;
	mInternals->mInteractionState = kInteractionStateNone;

	// Commit the in-progress segment (store + propagate to the source), replacing the transient preview window.
	//	Normalize here - a zero-duration click point commits as no segment (the whole source), so a click seeks without
	//	leaving a spurious empty selection.
	CMediaDestination::setMediaSegment(
			(mInternals->mPendingMediaSegment.hasValue() &&
							(mInternals->mPendingMediaSegment->getDurationTimeInterval() > 0.0)) ?
					mInternals->mPendingMediaSegment : OV<SMedia::Segment>());

	// End the interaction: the commit changed the source scope, so re-seek to the auditioned start, then restore
	mInternals->endInteraction(true);
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlayer::stop()
//----------------------------------------------------------------------------------------------------------------------
{
	// Commit any in-progress media-segment interaction so it takes effect before this call
	mediaSegmentDidChange();

	// Iterate all audio tracks
	for (UInt32 i = 0; i < getAudioTrackCount(); i++)
		// Stop
		getAudioDestination(i)->stop();

	// Iterate all video tracks
	for (UInt32 i = 0; i < getVideoTrackCount(); i++)
		// Stop
		getVideoDestination(i)->stop();

	// Update internals
	mInternals->mCurrentTimeInterval =
			getMediaSegment().hasValue() ? getMediaSegment()->getStartTimeInterval() : 0.0;
	mInternals->mCurrentFrameIndex.removeValue();
	mInternals->mEndOfDataCount = 0;

	// Call proc
	mInternals->mInfo.audioPositionUpdated(mInternals->mCurrentTimeInterval);
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlayer::restart()
//----------------------------------------------------------------------------------------------------------------------
{
	// Seek to the begining of the media segment, or beginning beginning
	seek(getMediaSegment().hasValue() ? getMediaSegment()->getStartTimeInterval() : 0.0);
}
