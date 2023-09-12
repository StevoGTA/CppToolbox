//----------------------------------------------------------------------------------------------------------------------
//	CAudioPlayer.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioProcessor.h"
#include "CQueue.h"
#include "CThread.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioPlayer

class CAudioPlayer : public CAudioDestination {
	// Info
	public:
		struct Info {
			// Procs - called from the audio render thread so they must not allocate memory and must be QUICK.
			typedef	void	(*PositionUpdatedProc)(const CAudioPlayer& audioPlayer, UniversalTimeInterval position,
									void* userData);
			typedef	void	(*EndOfDataProc)(const CAudioPlayer& audioPlayer, void* userData);

			// Procs - called from the buffer thread so do whatever you want
			typedef	void	(*ErrorProc)(const CAudioPlayer& audioPlayer, const SError& error, void* userData);

					// Lifecycle methods
					Info(PositionUpdatedProc positionUpdatedProc, EndOfDataProc endOfDataProc, ErrorProc errorProc,
							void* userData) :
						mPositionUpdatedProc(positionUpdatedProc), mEndOfDataProc(endOfDataProc), mErrorProc(errorProc),
								mUserData(userData)
						{}
					Info(const Info& other) :
						mPositionUpdatedProc(other.mPositionUpdatedProc), mEndOfDataProc(other.mEndOfDataProc),
								mErrorProc(other.mErrorProc), mUserData(other.mUserData)
						{}

					// Instance methods
			void	positionUpdated(const CAudioPlayer& audioPlayer, UniversalTimeInterval position)
						{ mPositionUpdatedProc(audioPlayer, position, mUserData); }
			void	endOfData(const CAudioPlayer& audioPlayer) const
						{ mEndOfDataProc(audioPlayer, mUserData); }
			void	error(const CAudioPlayer& audioPlayer, const SError& error) const
						{ mErrorProc(audioPlayer, error, mUserData); }

			// Properties
			private:
				PositionUpdatedProc	mPositionUpdatedProc;
				EndOfDataProc		mEndOfDataProc;
				ErrorProc			mErrorProc;
				void*				mUserData;
		};

	// Classes
	private:
		class Internals;

	// Methods
	public:
														// Lifecycle methods
														CAudioPlayer(const CString& identifier, const Info& info);
														~CAudioPlayer();

														// CAudioProcessor methods
						OV<SError>						connectInput(const I<CAudioProcessor>& audioProcessor,
																const SAudio::ProcessingFormat& audioProcessingFormat);
						TArray<CString>					getSetupDescription(const CString& indent);

						void							setSourceWindow(UniversalTimeInterval startTimeInterval,
																const OV<UniversalTimeInterval>& durationTimeInterval);
						void							seek(UniversalTimeInterval timeInterval);

						void							reset();

						TArray<SAudio::ProcessingSetup>	getInputSetups() const;

														// CAudioDestination methods
						void							setupComplete();

														// Instance methods
				const	CString&						getIdentifier() const;

														// gain applies to any channel not referenced by channelGains
						void							setGain(Float32 gain);
						void							setGain(const TNumberArray<Float32>& channelGains);

						void							play();
						void							pause();
						bool							isPlaying() const;

						void							startSeek();
						void							finishSeek();

														// Class methods
		static			void							setMaxAudioPlayers(UInt32 maxAudioPlayers);
		static			UniversalTimeInterval			getPlaybackBufferDuration();
		static			void							setPlaybackBufferDuration(
																UniversalTimeInterval playbackBufferDuration);

	// Properties
	private:
		static	const	UniversalTimeInterval	kPreviewDuration;

						Internals*				mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioPlayerBufferThread

class CAudioPlayerBufferThread : public CThread {
	// Procs
	public:
		typedef	void	(*ErrorProc)(const SError& error, void* userData);

	// Classes
	private:
		class Internals;

	// Methods
	public:
				// Lifecycle methods
				CAudioPlayerBufferThread(CAudioPlayer& audioPlayer, CSRSWBIPSegmentedQueue& queue, UInt32 bytesPerFrame,
						UInt32 maxOutputFrames, ErrorProc errorProc, void* procsUserData);
				~CAudioPlayerBufferThread();

				// CThread methods
		void	run();

				// Instance methods
		void	pause();
		void	resume();

		void	noteQueueReadComplete();
		bool	getDidReachEnd() const;

		void	shutdown();

	// Properties
	private:
		Internals*	mInternals;
};
