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
				SAudioProcessingFormat				composeAudioProcessingFormat(const CAudioSource& audioSource,
															const CAudioDestination& audioDestination,
															const OV<Float32>& processingSampleRate = OV<Float32>())
															const;
				TVResult<SAudioProcessingFormat>	connect(const I<CAudioProcessor>& audioProcessorSource,
															const I<CAudioProcessor>& audioProcessorDestination,
															const SAudioProcessingFormat& audioProcessingFormat) const;

				I<CVideoSource>						getVideoSource(
															const CMediaTrackInfos::VideoTrackInfo& videoTrackInfo,
															CVideoFrame::Compatibility compatibility,
															const CString& identifier) const;

	protected:
													// Subclass methods
		virtual	I<CAudioConverter>					createAudioConverter() const = 0;
};
