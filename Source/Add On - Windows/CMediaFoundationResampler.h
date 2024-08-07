//----------------------------------------------------------------------------------------------------------------------
//	CMediaFoundationResampler.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioConverter.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaFoundationResampler

class CMediaFoundationResamplerInternals;
class CMediaFoundationResampler : public CAudioConverter {
	public:
										// Lifecycle methods
										CMediaFoundationResampler();
										~CMediaFoundationResampler();

										// CAudioProcessor methods
		OV<SError>						connectInput(const I<CAudioProcessor>& audioProcessor,
												const SAudio::ProcessingFormat& audioProcessingFormat);
		TArray<CString>					getSetupDescription(const CString& indent);

		TVResult<SourceInfo>			performInto(CAudioFrames& audioFrames);
		void							reset();

		TArray<SAudio::ProcessingSetup>	getInputSetups() const;

										// CAudioConverter methods
		bool							supportsNoninterleaved() const
											{ return false; }

	// Properties
	private:
		CMediaFoundationResamplerInternals*	mInternals;
};
