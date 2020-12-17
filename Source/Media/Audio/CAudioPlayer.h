//----------------------------------------------------------------------------------------------------------------------
//	CAudioPlayer.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioProcessor.h"
#include "CNotificationCenter.h"
#include "CQueue.h"
#include "CThread.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioPlayer

class CAudioPlayerInternals;
class CAudioPlayer : public CAudioDestination {
	// Structs
	public:
		struct Procs {
			// Procs - called from the audio render thread so they must not allocate memory and must be QUICK.
			typedef	void	(*PositionUpdatedProc)(const CAudioPlayer& audioPlayer, UniversalTime position,
									void* userData);
			typedef	void	(*EndOfDataProc)(const CAudioPlayer& audioPlayer, void* userData);

			// Proces - called from the reader thread so do whatever you want
			typedef	void	(*ErrorProc)(const CAudioPlayer& audioPlayer, const SError& error, void* userData);

						// Lifecycle methods
						Procs(PositionUpdatedProc positionUpdatedProc, EndOfDataProc endOfDataProc, ErrorProc errorProc,
								void* userData) :
							mPositionUpdatedProc(positionUpdatedProc), mEndOfDataProc(endOfDataProc),
									mErrorProc(errorProc), mUserData(userData)
							{}
						Procs(const Procs& other) :
							mPositionUpdatedProc(other.mPositionUpdatedProc), mEndOfDataProc(other.mEndOfDataProc),
									mErrorProc(other.mErrorProc), mUserData(other.mUserData)
							{}

						// Instance methods
				void	positionUpdated(const CAudioPlayer& audioPlayer, UniversalTime position)
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

	// Methods
	public:
														// Lifecycle methods
														CAudioPlayer(const CString& identifier, const Procs& procs);
														~CAudioPlayer();

														// CAudioProcessor methods
						TArray<SAudioProcessingSetup>	getInputSetups() const;
						OI<SError>						connectInput(const I<CAudioProcessor>& audioProcessor,
																const SAudioProcessingFormat& audioProcessingFormat);

						OI<SError>						reset();

														// CAudioDestination methods
						void							setupComplete();

														// Instance methods
				const	CString&						getIdentifier() const;

						void							setGain(Float32 gain);

						void							play();
						void							pause();
						bool							isPlaying() const;

						void							startSeek();
						void							seek(UniversalTimeInterval timeInterval,
																OV<UniversalTimeInterval> durationTimeInterval,
																bool playPreview);
						void							finishSeek();

														// Class methods
		static			void							setMaxAudioPlayers(UInt32 maxAudioPlayers);
		static			UniversalTimeInterval			getPlaybackBufferDuration();
		static			void							setPlaybackBufferDuration(
																UniversalTimeInterval playbackBufferDuration);

	// Properties
	private:
		static	const	UniversalTimeInterval	kMinBufferDuration;
		static	const	UniversalTimeInterval	kMaxBufferDuration;
		static	const	UniversalTimeInterval	kPreviewDuration;

						CAudioPlayerInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioPlayerReaderThread

class CAudioPlayerReaderThreadInternals;
class CAudioPlayerReaderThread : public CThread {
	// Procs
	public:
		typedef	void	(*ErrorProc)(const SError& error, void* userData);

	// Methods
	public:
				// Lifecycle methods
				CAudioPlayerReaderThread(CAudioPlayer& audioPlayer, CSRSWBIPSegmentedQueue& queue, UInt32 bytesPerFrame,
						UInt32 maxOutputFrames, ErrorProc errorProc, void* procsUserData);
				~CAudioPlayerReaderThread();

				// CThread methods
		void	run();

				// Instance methods
		void	seek(UniversalTimeInterval timeInterval, UInt32 maxFrames);
		void	resume();
		void	noteQueueReadComplete();
		bool	getDidReachEndOfData() const;
		void	stopReading();
		void	shutdown();

	// Properties
	private:
		CAudioPlayerReaderThreadInternals*	mInternals;
};
