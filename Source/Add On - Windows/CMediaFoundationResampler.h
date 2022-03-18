//----------------------------------------------------------------------------------------------------------------------
//	CMediaFoundationResampler.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioProcessor.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaFoundationResampler

class CMediaFoundationResamplerInternals;
class CMediaFoundationResampler : public CAudioConverter {
	public:
							// Lifecycle methods
							CMediaFoundationResampler();
							~CMediaFoundationResampler();

							// CAudioProcessor methods
		OI<SError>			connectInput(const I<CAudioProcessor>& audioProcessor,
									const SAudioProcessingFormat& audioProcessingFormat);
		TNArray<CString>	getSetupDescription(const CString& indent);

		SAudioSourceStatus	performInto(CAudioFrames& audioFrames);
		void				reset();

	// Properties
	private:
		CMediaFoundationResamplerInternals*	mInternals;
};
