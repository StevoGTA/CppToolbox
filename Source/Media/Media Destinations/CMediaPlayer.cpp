//----------------------------------------------------------------------------------------------------------------------
//	CMediaPlayer.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMediaPlayer.h"

#include "CAudioPlayer.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaPlayerInternals

class CMediaPlayerInternals {
	public:
		struct AudioPlayerPositionUpdatedMessage : public CSRSWMessageQueue::ProcMessage {
			// Methods
			AudioPlayerPositionUpdatedMessage(Proc proc, void* userData, const CAudioPlayer& audioPlayer,
					UniversalTime position) :
				CSRSWMessageQueue::ProcMessage(sizeof(AudioPlayerPositionUpdatedMessage), proc, userData),
						mAudioPlayer(audioPlayer), mPosition(position)
				{}

			// Properties
			const	CAudioPlayer&	mAudioPlayer;
					UniversalTime	mPosition;
		};

		struct AudioPlayerEndOfDataMessage : public CSRSWMessageQueue::ProcMessage {
			// Methods
			AudioPlayerEndOfDataMessage(Proc proc, void* userData, const CAudioPlayer& audioPlayer) :
				CSRSWMessageQueue::ProcMessage(sizeof(AudioPlayerEndOfDataMessage), proc, userData),
						mAudioPlayer(audioPlayer)
				{}

			// Properties
			const	CAudioPlayer&	mAudioPlayer;
		};

		struct AudioPlayerErrorMessage : public CSRSWMessageQueue::ProcMessage {
			// Methods
			AudioPlayerErrorMessage(Proc proc, void* userData, const CAudioPlayer& audioPlayer,
					const SError& error) :
				CSRSWMessageQueue::ProcMessage(sizeof(AudioPlayerErrorMessage), proc, userData),
						mAudioPlayer(audioPlayer), mError(error)
				{}

			// Properties
			const	CAudioPlayer&	mAudioPlayer;
					SError			mError;
		};

						CMediaPlayerInternals(CMediaPlayer& mediaPlayer, CSRSWMessageQueues& messageQueues) :
							mMediaPlayer(mediaPlayer), mMessageQueues(messageQueues),
									mEndOfDataCount(0), mCurrentLoopCount(0)
							{}

		static	void	audioPlayerPositionUpdated(const CAudioPlayer& audioPlayer, UniversalTime position,
								void* userData)
							{
								// Setup
								CMediaPlayerInternals&	internals = *((CMediaPlayerInternals*) userData);
								CSRSWMessageQueue&		messageQueue =
																*internals.mAudioPlayerMessageQueueMap[audioPlayer
																		.getIdentifier()];

								// Submit
								messageQueue.submit(
										AudioPlayerPositionUpdatedMessage(handleAudioPlayerPositionUpdated, userData,
												audioPlayer, position));
							}
		static	void	handleAudioPlayerPositionUpdated(const CSRSWMessageQueue::ProcMessage& message,
								void* userData)
							{
								// Setup
								CMediaPlayerInternals&				internals = *((CMediaPlayerInternals*) userData);
//								AudioPlayerPositionUpdatedMessage&	positionUpdatedMessage =
//																			(AudioPlayerPositionUpdatedMessage&)
//																					message;
								if (!mActiveInternals.contains(internals))
									return;
							}
		static	void	audioPlayerEndOfData(const CAudioPlayer& audioPlayer, void* userData)
							{
								// Setup
								CMediaPlayerInternals&	internals = *((CMediaPlayerInternals*) userData);
								CSRSWMessageQueue&		messageQueue =
																*internals.mAudioPlayerMessageQueueMap[audioPlayer
																		.getIdentifier()];

								// Submit
								messageQueue.submit(
										AudioPlayerEndOfDataMessage(handleAudioPlayerEndOfData, userData, audioPlayer));
							}
		static	void	handleAudioPlayerEndOfData(const CSRSWMessageQueue::ProcMessage& message, void* userData)
							{
								// Setup
								CMediaPlayerInternals&			internals = *((CMediaPlayerInternals*) userData);
//								AudioPlayerEndOfDataMessage&	endOfDataMessage =
//																		(AudioPlayerEndOfDataMessage&) message;
								if (!mActiveInternals.contains(internals))
									return;

								// One more at end
								if (++internals.mEndOfDataCount == internals.mMediaPlayer.getAudioTrackCount()) {
									// All at the end
									internals.mCurrentLoopCount++;

									// Check if have loop count
									if (internals.mLoopCount.hasValue() &&
											((internals.mLoopCount.getValue() == 0) ||
													(internals.mCurrentLoopCount < internals.mLoopCount.getValue()))) {
										// Reset and go again
										internals.mEndOfDataCount = 0;

										for (UInt32 i = 0; i < internals.mMediaPlayer.getAudioTrackCount(); i++) {
											// Setup
											OR<CAudioPlayer>	audioPlayer =
																		internals.mMediaPlayer.getAudioProcessor(i);

											// Reset and play
											audioPlayer->reset();
											audioPlayer->play();
										}
									}
								}
							}
		static	void	audioPlayerError(const CAudioPlayer& audioPlayer, const SError& error, void* userData)
							{
								// Setup
								CMediaPlayerInternals&	internals = *((CMediaPlayerInternals*) userData);
								CSRSWMessageQueue&		messageQueue =
																*internals.mAudioPlayerMessageQueueMap[audioPlayer
																		.getIdentifier()];

								// Submit
								messageQueue.submit(
										AudioPlayerErrorMessage(handleAudioPlayerError, userData, audioPlayer, error));
							}
		static	void	handleAudioPlayerError(const CSRSWMessageQueue::ProcMessage& message, void* userData)
							{
								// Setup
								CMediaPlayerInternals&		internals = *((CMediaPlayerInternals*) userData);
//								AudioPlayerErrorMessage&	errorMessage = (AudioPlayerErrorMessage&) message;
								if (!mActiveInternals.contains(internals))
									return;
							}

				CMediaPlayer&					mMediaPlayer;
				CSRSWMessageQueues&				mMessageQueues;

				TDictionary<CSRSWMessageQueue>	mAudioPlayerMessageQueueMap;

				UInt32							mEndOfDataCount;
				OV<UInt32>						mLoopCount;
				UInt32							mCurrentLoopCount;

		static	TIArray<CMediaPlayerInternals>	mActiveInternals;
};

TIArray<CMediaPlayerInternals>	CMediaPlayerInternals::mActiveInternals;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaPlayer

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CMediaPlayer::CMediaPlayer(CSRSWMessageQueues& messageQueues)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CMediaPlayerInternals(*this, messageQueues);

	// Add to array
	CMediaPlayerInternals::mActiveInternals += mInternals;
}

//----------------------------------------------------------------------------------------------------------------------
CMediaPlayer::~CMediaPlayer()
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset
	reset();

	// Cleanup
	for (UInt32 i = 0; i < getAudioTrackCount(); i++)
		// Remove message queue
		mInternals->mMessageQueues.remove(
				*mInternals->mAudioPlayerMessageQueueMap[getAudioProcessor(i)->getIdentifier()]);

	CMediaPlayerInternals::mActiveInternals -= *mInternals;
}

// MARK: - CMediaDestination methods

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
I<CAudioPlayer> CMediaPlayer::newAudioPlayer(const CString& identifier)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create Audio Player
	I<CAudioPlayer>	audioPlayer(
			new CAudioPlayer(identifier,
					CAudioPlayer::Procs(CMediaPlayerInternals::audioPlayerPositionUpdated,
							CMediaPlayerInternals::audioPlayerEndOfData, CMediaPlayerInternals::audioPlayerError,
							mInternals)));

	// Setup message queue
	CSRSWMessageQueue	messageQueue(10 * 1024);
	mInternals->mAudioPlayerMessageQueueMap.set(identifier, messageQueue);
	mInternals->mMessageQueues.add(messageQueue);

	return audioPlayer;
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
void CMediaPlayer::setLoopCount(OV<UInt32> loopCount)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mLoopCount = loopCount;
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlayer::play()
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all audio tracks
	for (UInt32 i = 0; i < getAudioTrackCount(); i++)
		// Play
		getAudioProcessor(i)->play();
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaPlayer::pause()
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all audio tracks
	for (UInt32 i = 0; i < getAudioTrackCount(); i++)
		// Play
		getAudioProcessor(i)->pause();
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
OI<SError> CMediaPlayer::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all audio tracks
	for (UInt32 i = 0; i < getAudioTrackCount(); i++) {
		// Reset
		OI<SError>	error = getAudioProcessor(i)->reset();
		ReturnErrorIfError(error);
	}

	// Update internals
	mInternals->mEndOfDataCount = 0;

	return OI<SError>();
}
