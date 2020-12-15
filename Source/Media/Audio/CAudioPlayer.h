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
	// Notifications
	public:
		/*
			Sent when the playback position has been updated
				senderRef is CAudioPlayer*
				info contains the following keys:
					mPlaybackPositionKey
		*/
		static	CString mPlaybackPositionUpdatedNotificationName;

		static	CString	mPlaybackPositionKey;	// UniversalTime

		/*
			Sent when end of data has been reached
				senderRef is CAudioPlayer*
		*/
		static	CString mEndOfDataNotificationName;

	// Structs
	public:
		struct Procs {
			// Procs
			typedef	void	(*ErrorProc)(const SError& error, void* userData);

						// Lifecycle methods
						Procs(ErrorProc errorProc, void* userData) : mErrorProc(errorProc), mUserData(userData) {}
						Procs(const Procs& other) : mErrorProc(other.mErrorProc), mUserData(other.mUserData) {}

						// Instance methods
				void	error(const SError& error) const
							{ mErrorProc(error, mUserData); }

			// Properties
			private:
				ErrorProc	mErrorProc;
				void*		mUserData;
		};

	// Methods
	public:
														// Lifecycle methods
														CAudioPlayer(const CString& identifier,
																CNotificationCenter& notificationCenter,
																const Procs& procs);
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

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioPlayerPlaybackPositionUpdatedNotificationThread

class CAudioPlayerPlaybackPositionUpdatedNotificationThreadInternals;
class CAudioPlayerPlaybackPositionUpdatedNotificationThread : public CThread {
	// Methods
	public:
				// Lifecycle methods
				CAudioPlayerPlaybackPositionUpdatedNotificationThread(CNotificationCenter& notificationCenter,
						CAudioPlayer& audioPlayer);
				~CAudioPlayerPlaybackPositionUpdatedNotificationThread();

				// CThread methods
		void	run();

				// Instance methods
		void	notePlaybackPositionUpdated(UniversalTimeInterval timeInterval);
		void	shutdown();

	// Properties
	private:
		CAudioPlayerPlaybackPositionUpdatedNotificationThreadInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioPlayerEndOfDataNotificationThread

class CAudioPlayerEndOfDataNotificationThreadInternals;
class CAudioPlayerEndOfDataNotificationThread : public CThread {
	// Methods
	public:
				// Lifecycle methods
				CAudioPlayerEndOfDataNotificationThread(CNotificationCenter& notificationCenter,
						CAudioPlayer& audioPlayer);
				~CAudioPlayerEndOfDataNotificationThread();

				// CThread methods
		void	run();

				// Instance methods
		void	noteEndOfData();
		void	shutdown();

	// Properties
	private:
		CAudioPlayerEndOfDataNotificationThreadInternals*	mInternals;
};
