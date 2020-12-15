//----------------------------------------------------------------------------------------------------------------------
//	CMediaPlayer.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMediaPlayer.h"

#include "CAudioPlayer.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaPlayerInternals

class CMediaPlayerInternals {
	public:
						CMediaPlayerInternals(CMediaPlayer& mediaPlayer, CNotificationCenter& notificationCenter) :
							mMediaPlayer(mediaPlayer), mNotificationCenter(notificationCenter),
									mEndOfDataCount(0), mCurrentLoopCount(0)
							{}

		static	void	audioPlayerPositionUpdated(const CString& notificationName, const void* senderRef,
								const CDictionary& info, void* userData)
							{
							}
		static	void	audioPlayerEndOfData(const CString& notificationName, const void* senderRef,
								const CDictionary& info, void* userData)
							{
								// Setup
								CMediaPlayerInternals&	internals = *((CMediaPlayerInternals*) userData);

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
		static	void	audioPlayerError(const SError& error, void* userData)
							{
							}

		CMediaPlayer&			mMediaPlayer;
		CNotificationCenter&	mNotificationCenter;

		UInt32					mEndOfDataCount;
		OV<UInt32>				mLoopCount;
		UInt32					mCurrentLoopCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaPlayer

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CMediaPlayer::CMediaPlayer(CNotificationCenter& notificationCenter)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CMediaPlayerInternals(*this, notificationCenter);
}

//----------------------------------------------------------------------------------------------------------------------
CMediaPlayer::~CMediaPlayer()
//----------------------------------------------------------------------------------------------------------------------
{
	// Cleanup
	mInternals->mNotificationCenter.unregisterObserver(this);

	Delete(mInternals);
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
			new CAudioPlayer(identifier, mInternals->mNotificationCenter,
					CAudioPlayer::Procs(CMediaPlayerInternals::audioPlayerError, mInternals)));

	// Setup notifications
	mInternals->mNotificationCenter.registerObserver(CAudioPlayer::mPlaybackPositionUpdatedNotificationName,
			&*audioPlayer,
			CNotificationCenter::ObserverInfo(this, CMediaPlayerInternals::audioPlayerPositionUpdated, mInternals));
	mInternals->mNotificationCenter.registerObserver(CAudioPlayer::mEndOfDataNotificationName, &*audioPlayer,
			CNotificationCenter::ObserverInfo(this, CMediaPlayerInternals::audioPlayerEndOfData, mInternals));

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
