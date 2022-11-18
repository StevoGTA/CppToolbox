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
												const SAudioProcessingFormat& audioProcessingFormat);
		TNArray<CString>				getSetupDescription(const CString& indent);

		SAudioSourceStatus				performInto(CAudioFrames& audioFrames);
		void							reset();

		TArray<SAudioProcessingSetup>	getInputSetups() const;

										// CAudioConverter methods
		bool							supportsNoninterleaved() const
											{ return false; }

	// Properties
	private:
		CMediaFoundationResamplerInternals*	mInternals;
};
