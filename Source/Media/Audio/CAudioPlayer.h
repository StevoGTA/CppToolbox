//----------------------------------------------------------------------------------------------------------------------
//	CAudioPlayer.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioProcessor.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioPlayer

class CAudioPlayerInternals;
class CAudioPlayer : public CAudioDestination {
	// Structs
	public:
		struct Procs {
			// Procs
			typedef	void	(*PositionUpdatedProc)(UniversalTimeInterval timeInterval, void* userData);
			typedef	void	(*EndOfDataProc)(void* userData);
			typedef	void	(*ErrorProc)(const SError& error, void* userData);

						// Lifecycle methods
						Procs(PositionUpdatedProc positionUpdatedProc, EndOfDataProc endOfDataProc,
								ErrorProc errorProc, void* userData) :
							mPositionUpdatedProc(positionUpdatedProc), mEndOfDataProc(endOfDataProc),
									mErrorProc(errorProc), mUserData(userData)
							{}
						Procs(const Procs& other) :
							mPositionUpdatedProc(other.mPositionUpdatedProc), mEndOfDataProc(other.mEndOfDataProc),
									mErrorProc(other.mErrorProc), mUserData(other.mUserData)
							{}

						// Instance methods
				void	positionUpdated(UniversalTimeInterval timeInterval) const
							{ mPositionUpdatedProc(timeInterval, mUserData); }
				void	endOfData() const
							{ mEndOfDataProc(mUserData); }
				void	error(const SError& error) const
							{ mErrorProc(error, mUserData); }

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

#if TARGET_OS_MACOS
		static			void							logInfo();
#endif

	// Properties
	private:
		static	const	UniversalTimeInterval		kMinBufferDuration;
		static	const	UniversalTimeInterval		kMaxBufferDuration;
		static	const	UniversalTimeInterval		kPreviewDuration;

						CAudioPlayerInternals*		mInternals;
};
