//----------------------------------------------------------------------------------------------------------------------
//	CMediaPlayer.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioPlayer.h"
#include "CMediaDestination.h"
#include "CVideoDecoder.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaPlayer

class CMediaPlayerInternals;
class CMediaPlayer : public TMediaDestination<CAudioPlayer, CVideoDecoder> {
	// Info
	public:
		struct Info {
			// Procs
			typedef	void	(*FinishedProc)(void* userData);

					// Lifecycle methods
					Info() : mFinishedProc(nil), mUserData(nil) {}
					Info(FinishedProc finishedProc, void* userData) :
						mFinishedProc(finishedProc), mUserData(userData)
						{}
					Info(const Info& other) : mFinishedProc(other.mFinishedProc), mUserData(other.mUserData) {}

					// Instance methods
			void	finished() const
						{
							// Check for proc
							if (mFinishedProc != nil)
								// Call proc
								mFinishedProc(mUserData);
						}


			// Properties
			FinishedProc	mFinishedProc;
			void*			mUserData;
		};

	// Methods
	public:
									// Lifecycle methods
									CMediaPlayer(CSRSWMessageQueues& messageQueues, const Info& info);
									~CMediaPlayer();

									// CMediaDestination methods
				void				setupComplete();

									// Instance methods
		virtual	I<CAudioPlayer>		newAudioPlayer(const CString& identifier, UInt32 trackIndex);
		virtual	void				setAudioGain(Float32 audioGain);

		virtual	I<CVideoDecoder>	newVideoDecoder(const SVideoStorageFormat& videoStorageFormat,
											const I<CCodec::DecodeInfo>& codecDecodeInfo,
											const I<CDataSource>& dataSource, const CString& identifier,
											UInt32 trackIndex,
											CVideoCodec::DecodeFrameInfo::Compatibility compatibility,
											const CVideoDecoder::RenderInfo& renderInfo);

		virtual	void				setLoopCount(OV<UInt32> loopCount = OV<UInt32>());

		virtual	void				play();
		virtual	void				pause();
		virtual	bool				isPlaying() const;

		virtual	OI<SError>			reset();

	// Properties
	private:
		CMediaPlayerInternals*	mInternals;
};
