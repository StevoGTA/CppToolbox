//----------------------------------------------------------------------------------------------------------------------
//	CMediaPlayer.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMediaPlayer.h"

#include "CAudioPlayer.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaPlayerAudioPlayer

class CMediaPlayerAudioPlayer : public CAudioPlayer {
	public:
		CMediaPlayerAudioPlayer(const CString& identifier, const CAudioPlayer::Info& info) :
			CAudioPlayer(identifier, info), mMessageQueue(10 * 1024)
			{}

		CSRSWMessageQueue	mMessageQueue;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaPlayerVideoDecoder

class CMediaPlayerVideoDecoder : public CVideoDecoder {
	public:
		CMediaPlayerVideoDecoder(const SVideoStorageFormat& videoStorageFormat,
				const I<CCodec::DecodeInfo>& codecDecodeInfo, const I<CDataSource>& dataSource,
				const CString& identifier, UInt32 trackIndex, const DecodeInfo& decodeInfo,
				CVideoCodec::DecodeFrameInfo::Compatibility compatibility, const RenderInfo& renderInfo) :
			CVideoDecoder(videoStorageFormat, codecDecodeInfo, dataSource, identifier, trackIndex, decodeInfo,
					compatibility, renderInfo), mMessageQueue(10 * 1024)
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

		struct VideoDecoderFrameReadyMessage : public CSRSWMessageQueue::ProcMessage {
			// Lifecycle Methods
			VideoDecoderFrameReadyMessage(Proc proc, void* userData, CVideoDecoder& videoDecoder) :
				CSRSWMessageQueue::ProcMessage(sizeof(VideoDecoderFrameReadyMessage), proc, userData),
						mVideoDecoder(videoDecoder)
				{}

			// Properties
			CVideoDecoder&	mVideoDecoder;
		};

		struct VideoDecoderErrorMessage : public CSRSWMessageQueue::ProcMessage {
			// Lifecycle Methods
			VideoDecoderErrorMessage(Proc proc, void* userData, const CVideoDecoder& videoDecoder,
					const SError& error) :
				CSRSWMessageQueue::ProcMessage(sizeof(VideoDecoderErrorMessage), proc, userData),
						mVideoDecoder(videoDecoder), mError(error)
				{}

			// Properties
			const	CVideoDecoder&	mVideoDecoder;
					SError			mError;
		};

						CMediaPlayerInternals(CMediaPlayer& mediaPlayer, CSRSWMessageQueues& messageQueues,
								const CMediaPlayer::Info& info) :
							mMediaPlayer(mediaPlayer), mMessageQueues(messageQueues), mInfo(info),
									mEndOfDataCount(0), mCurrentLoopCount(0)
							{}

		static	void	audioPlayerPositionUpdated(const CAudioPlayer& audioPlayer, UniversalTimeInterval position,
								void* userData)
							{
								// Submit
								((CMediaPlayerAudioPlayer&) audioPlayer).mMessageQueue.submit(
										AudioPlayerPositionUpdatedMessage(handleAudioPlayerPositionUpdated, userData,
												audioPlayer, position));
							}
		static	void	handleAudioPlayerPositionUpdated(const CSRSWMessageQueue::ProcMessage& message,
								void* userData)
							{
								// Setup
								CMediaPlayerInternals&				internals = *((CMediaPlayerInternals*) userData);
								AudioPlayerPositionUpdatedMessage&	positionUpdatedMessage =
																			(AudioPlayerPositionUpdatedMessage&)
																					message;
								if (!mActiveInternals.contains(internals))
									return;

								// Iterate all video decoders
								for (UInt32 i = 0; i < internals.mMediaPlayer.getVideoTrackCount(); i++)
									// Update video decoder
									internals.mMediaPlayer.getVideoProcessor(i)->notePositionUpdated(
											positionUpdatedMessage.mPosition);
							}
		static	void	audioPlayerEndOfData(const CAudioPlayer& audioPlayer, void* userData)
							{
								// Submit
								((CMediaPlayerAudioPlayer&) audioPlayer).mMessageQueue.submit(
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
		static	void	handleAudioPlayerError(const CSRSWMessageQueue::ProcMessage& message, void* userData)
							{
								// Setup
								CMediaPlayerInternals&		internals = *((CMediaPlayerInternals*) userData);
//								AudioPlayerErrorMessage&	errorMessage = (AudioPlayerErrorMessage&) message;
								if (!mActiveInternals.contains(internals))
									return;
							}

		static	void	videoDecoderFrameReady(CVideoDecoder& videoDecoder, void* userData)
							{
								// Submit
								((CMediaPlayerVideoDecoder&) videoDecoder).mMessageQueue.submit(
										VideoDecoderFrameReadyMessage(handleVideoDeciderFrameReady, userData,
												videoDecoder));
							}
		static	void	handleVideoDeciderFrameReady(const CSRSWMessageQueue::ProcMessage& message, void* userData)
							{
								// Setup
								CMediaPlayerInternals&			internals = *((CMediaPlayerInternals*) userData);
								VideoDecoderFrameReadyMessage&	frameReadyMessage =
																		(VideoDecoderFrameReadyMessage&) message;
								if (!mActiveInternals.contains(internals))
									return;

								// Handle
								frameReadyMessage.mVideoDecoder.handleFrameReady();
							}
		static	void	videoDecoderError(const CVideoDecoder& videoDecoder, const SError& error, void* userData)
							{
								// Submit
								((CMediaPlayerVideoDecoder&) videoDecoder).mMessageQueue.submit(
										VideoDecoderErrorMessage(handleVideoDecoderError, userData, videoDecoder,
												error));
							}
		static	void	handleVideoDecoderError(const CSRSWMessageQueue::ProcMessage& message, void* userData)
							{
								// Setup
								CMediaPlayerInternals&		internals = *((CMediaPlayerInternals*) userData);
//								VideoDecoderErrorMessage&	errorMessage = (VideoDecoderErrorMessage&) message;
								if (!mActiveInternals.contains(internals))
									return;
							}

				CMediaPlayer&					mMediaPlayer;
				CSRSWMessageQueues&				mMessageQueues;
				CMediaPlayer::Info				mInfo;

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
CMediaPlayer::CMediaPlayer(CSRSWMessageQueues& messageQueues, const Info& info)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CMediaPlayerInternals(*this, messageQueues, info);

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
	for (UInt32 i = 0; i < getAudioTrackCount(); i++) {
		// Remove message queue
		CMediaPlayerAudioPlayer&	audioPlayer = (CMediaPlayerAudioPlayer&) *getAudioProcessor(i);
		mInternals->mMessageQueues.remove(audioPlayer.mMessageQueue);
	}

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
I<CVideoDecoder> CMediaPlayer::newVideoDecoder(const SVideoStorageFormat& videoStorageFormat,
		const I<CCodec::DecodeInfo>& codecDecodeInfo, const I<CDataSource>& dataSource, const CString& identifier,
		UInt32 trackIndex, CVideoCodec::DecodeFrameInfo::Compatibility compatibility,
		const CVideoDecoder::RenderInfo& renderInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create Video Decoder
	I<CMediaPlayerVideoDecoder>	videoDecoder(
										new CMediaPlayerVideoDecoder(videoStorageFormat, codecDecodeInfo, dataSource,
												identifier, trackIndex,
												CVideoDecoder::DecodeInfo(CMediaPlayerInternals::videoDecoderFrameReady,
														CMediaPlayerInternals::videoDecoderError, mInternals),
												compatibility, renderInfo));

	// Add message queue
	mInternals->mMessageQueues.add(videoDecoder->mMessageQueue);

	// Add
	add((const I<CVideoProcessor>&) videoDecoder, trackIndex);

	return *((I<CVideoDecoder>*) &videoDecoder);
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

	// Iterate all video tracks
	for (UInt32 i = 0; i < getVideoTrackCount(); i++) {
		// Reset
		OI<SError>	error = getVideoProcessor(i)->reset();
		ReturnErrorIfError(error);
	}

	// Update internals
	mInternals->mEndOfDataCount = 0;

	return OI<SError>();
}
