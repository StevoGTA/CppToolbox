//----------------------------------------------------------------------------------------------------------------------
//	CAudioDecoder.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioProcessor.h"
#include "CAudioCodec.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioDecoder

class CAudioDecoder : public CAudioSource {
	// Classes
	private:
		class Internals;

	// Methods
	public:
										// Lifecycle methods
										CAudioDecoder(const SAudio::Format& audioFormat,
												const I<CDecodeAudioCodec>& audioCodec, const CString& identifier);
										~CAudioDecoder();

										// CAudioProcessor methods
		TArray<CString>					getSetupDescription(const CString& indent);

		CAudioFrames::Requirements		queryRequirements() const;

		void							setSourceWindow(UniversalTimeInterval startTimeInterval,
												const OV<UniversalTimeInterval>& durationTimeInterval);
		void							seek(UniversalTimeInterval timeInterval);

		TVResult<SMedia::SourceInfo>	performInto(CAudioFrames& audioFrames);

		void							reset();

		TArray<SAudio::ProcessingSetup>	getOutputSetups() const;
		OV<SError>						setOutputFormat(const SAudio::ProcessingFormat& audioProcessingFormat);

	// Properties
	private:
		Internals*	mInternals;
};
