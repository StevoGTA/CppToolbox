//----------------------------------------------------------------------------------------------------------------------
//	CMediaPlayer.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioPlayer.h"
#include "CMediaDestination.h"
#include "CVideoFrameStore.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaPlayer

class CMediaPlayerInternals;
class CMediaPlayer : public TMediaDestination<CAudioPlayer, CVideoFrameStore> {
	// Info
	public:
		struct Info {
			// Procs
			typedef	void	(*AudioPositionUpdatedProc)(UniversalTimeInterval position, void* userData);
			typedef	void	(*AudioErrorProc)(const SError& error, void* userData);

			typedef	void	(*VideoFrameUpdatedProc)(const CVideoFrame& videoFrame, void* userData);
			typedef	void	(*VideoErrorProc)(const SError& error, void* userData);

			typedef	void	(*FinishedProc)(void* userData);

					// Lifecycle methods
					Info() :
						mAudioPositionUpdatedProc(nil), mAudioErrorProc(nil), mVideoFrameUpdatedProc(nil),
								mVideoErrorProc(nil), mFinishedProc(nil), mUserData(nil)
						{}
					Info(AudioPositionUpdatedProc audioPositionUpdatedProc, AudioErrorProc audioErrorProc,
							VideoFrameUpdatedProc videoFrameUpdatedProc, VideoErrorProc videoErrorProc,
							FinishedProc finishedProc, void* userData) :
						mAudioPositionUpdatedProc(audioPositionUpdatedProc), mAudioErrorProc(audioErrorProc),
								mVideoFrameUpdatedProc(videoFrameUpdatedProc), mVideoErrorProc(videoErrorProc),
								mFinishedProc(finishedProc), mUserData(userData)
						{}
					Info(const Info& other) :
						mAudioPositionUpdatedProc(other.mAudioPositionUpdatedProc),
								mAudioErrorProc(other.mAudioErrorProc),
								mVideoFrameUpdatedProc(other.mVideoFrameUpdatedProc),
								mVideoErrorProc(other.mVideoErrorProc),
								mFinishedProc(other.mFinishedProc),
								mUserData(other.mUserData)
						{}

					// Instance methods
			void	audioPositionUpdated(UniversalTimeInterval position) const
						{
							// Check for proc
							if (mAudioPositionUpdatedProc != nil)
								// Call proc
								mAudioPositionUpdatedProc(position, mUserData);
						}
			void	audioError(const SError& error) const
						{
							// Check for proc
							if (mAudioErrorProc != nil)
								// Call proc
								mAudioErrorProc(error, mUserData);
						}

			void	videoFrameUpdated(const CVideoFrame& videoFrame) const
						{
							// Check for proc
							if (mVideoFrameUpdatedProc != nil)
								// Call proc
								mVideoFrameUpdatedProc(videoFrame, mUserData);
						}
			void	videoError(const SError& error) const
						{
							// Check for proc
							if (mVideoErrorProc != nil)
								// Call proc
								mVideoErrorProc(error, mUserData);
						}

			void	finished() const
						{
							// Check for proc
							if (mFinishedProc != nil)
								// Call proc
								mFinishedProc(mUserData);
						}

			// Properties
			private:
				AudioPositionUpdatedProc	mAudioPositionUpdatedProc;
				AudioErrorProc				mAudioErrorProc;

				VideoFrameUpdatedProc		mVideoFrameUpdatedProc;
				VideoErrorProc				mVideoErrorProc;

				FinishedProc				mFinishedProc;

				void*						mUserData;
		};

	// Methods
	public:
										// Lifecycle methods
										CMediaPlayer(CSRSWMessageQueues& messageQueues, const Info& info);
										~CMediaPlayer();

										// CMediaDestination methods
				void					seek(UniversalTimeInterval timeInterval);

				void					setupComplete();

										// Instance methods
		virtual	I<CAudioPlayer>			newAudioPlayer(const CString& identifier, UInt32 trackIndex);
		virtual	void					setAudioGain(Float32 audioGain);

		virtual	I<CVideoFrameStore>		newVideoFrameStore(const CString& identifier, UInt32 trackIndex);

		virtual	void					setLoopCount(OV<UInt32> loopCount = OV<UInt32>());

				UniversalTimeInterval	getCurrentPosition() const;

		virtual	void					play();
		virtual	void					pause();
		virtual	bool					isPlaying() const;

				void					startSeek();
				void					finishSeek();

		virtual	void					reset();

	// Properties
	private:
		CMediaPlayerInternals*	mInternals;
};
