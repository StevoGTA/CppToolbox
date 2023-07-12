//----------------------------------------------------------------------------------------------------------------------
//	CMediaEngine.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioConverter.h"
#include "CMediaDestination.h"
#include "SMediaTracks.h"
#include "TResult.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaEngine

class CMediaEngine {
	// Methods
	public:
													// Lifecycle methods
													CMediaEngine() {}
		virtual										~CMediaEngine() {}

													// Instance methods
				I<CAudioSource>						getAudioSource(
															const CMediaTrackInfos::AudioTrackInfo& audioTrackInfo,
															const CString& identifier) const;
				SAudio::ProcessingFormat			composeAudioProcessingFormat(const CAudioSource& audioSource,
															const CAudioDestination& audioDestination,
															const OV<Float32>& processingSampleRate = OV<Float32>())
															const;
				TVResult<SAudio::ProcessingFormat>	connect(const I<CAudioProcessor>& audioProcessorSource,
															const I<CAudioProcessor>& audioProcessorDestination,
															const SAudio::ProcessingFormat& audioProcessingFormat) const;

				I<CVideoSource>						getVideoSource(
															const CMediaTrackInfos::VideoTrackInfo& videoTrackInfo,
															const CString& identifier) const;

	protected:
													// Subclass methods
		virtual	I<CAudioConverter>					createAudioConverter() const = 0;
};
