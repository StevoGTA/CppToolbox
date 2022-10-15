//----------------------------------------------------------------------------------------------------------------------
//	CMediaEngine.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioProcessor.h"
#include "CMediaDestination.h"
#include "SMediaTracks.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaEngine

class CMediaEngine {
	// Structs
	public:
		struct ConnectResult {

												// Lifecycle methods
												ConnectResult(const SAudioProcessingFormat& audioProcessingFormat) :
													mAudioProcessingFormat(
															OV<SAudioProcessingFormat>(audioProcessingFormat))
													{}
												ConnectResult(const SError& error) : mError(OV<SError>(error)) {}

												// Instance methods
					bool						isSuccess() const
													{ return !mError.hasValue(); }
			const	OV<SAudioProcessingFormat>&	getAudioProcessingFormat() const
													{ return mAudioProcessingFormat; }
			const	OV<SError>&					getError() const
													{ return mError; }

			// Properties
			private:
				OV<SAudioProcessingFormat>	mAudioProcessingFormat;
				OV<SError>					mError;
		};

	// Methods
	public:
										// Lifecycle methods
										CMediaEngine() {}
		virtual							~CMediaEngine() {}

										// Instance methods
				I<CAudioSource>			getAudioSource(const CMediaTrackInfos::AudioTrackInfo& audioTrackInfo,
												const CString& identifier) const;
				SAudioProcessingFormat	composeAudioProcessingFormat(const CAudioSource& audioSource,
												const CAudioDestination& audioDestination,
												const OV<Float32>& processingSampleRate = OV<Float32>()) const;
				ConnectResult			connect(const I<CAudioProcessor>& audioProcessorSource,
												const I<CAudioProcessor>& audioProcessorDestination,
												const SAudioProcessingFormat& audioProcessingFormat) const;

				I<CVideoSource>			getVideoSource(const CMediaTrackInfos::VideoTrackInfo& videoTrackInfo,
												CVideoFrame::Compatibility compatibility, const CString& identifier)
												const;

	protected:
										// Subclass methods
		virtual	I<CAudioConverter>		createAudioConverter() const = 0;
};
