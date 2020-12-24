//----------------------------------------------------------------------------------------------------------------------
//	CMediaPlayer.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioPlayer.h"
#include "CMediaDestination.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaPlayer

class CMediaPlayerInternals;
class CMediaPlayer : public TMediaDestination<CAudioPlayer> {
	// Methods
	public:
								// Lifecycle methods
								CMediaPlayer(CSRSWMessageQueues& messageQueues);
								~CMediaPlayer();

								// CMediaDestination methods
				void			setupComplete();

								// Instance methods
		virtual	I<CAudioPlayer>	newAudioPlayer(const CString& identifier);
		virtual	void			setAudioGain(Float32 audioGain);

		virtual	void			setLoopCount(OV<UInt32> loopCount = OV<UInt32>());

		virtual	void			play();
		virtual	void			pause();
		virtual	bool			isPlaying() const;

		virtual	OI<SError>		reset();

	// Properties
	private:
		CMediaPlayerInternals*	mInternals;
};
