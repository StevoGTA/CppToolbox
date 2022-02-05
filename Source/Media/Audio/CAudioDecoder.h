//----------------------------------------------------------------------------------------------------------------------
//	CAudioDecoder.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioProcessor.h"
#include "CAudioCodec.h"
#include "CDataSource.h"
#include "SAudioFormats.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioDecoder

class CAudioDecoderInternals;
class CAudioDecoder : public CAudioSource {
	// Methods
	public:
										// Lifecycle methods
										CAudioDecoder(const SAudioStorageFormat& audioStorageFormat,
												const I<CAudioCodec>& audioCodec,
												const I<CCodec::DecodeInfo>& codecDecodeInfo);
										CAudioDecoder(const CAudioDecoder& other);
										~CAudioDecoder();

										// CAudioProcessor methods
		TArray<SAudioProcessingSetup>	getOutputSetups() const;
		OI<SError>						setOutputFormat(const SAudioProcessingFormat& audioProcessingFormat);

		Requirements					queryRequirements() const;

		void							setSourceWindow(UniversalTimeInterval startTimeInterval,
												const OV<UniversalTimeInterval>& durationTimeInterval);
		void							seek(UniversalTimeInterval timeInterval);

		SAudioSourceStatus				performInto(CAudioFrames& audioFrames);
		void							reset();

	// Properties
	private:
		CAudioDecoderInternals*	mInternals;
};
