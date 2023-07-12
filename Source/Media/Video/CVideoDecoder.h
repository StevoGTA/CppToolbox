//----------------------------------------------------------------------------------------------------------------------
//	CVideoDecoder.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CVideoCodec.h"
#include "CVideoProcessor.h"
#include "SVideo.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CVideoDecoder

class CVideoDecoder : public CVideoSource {
	// Classes
	private:
		class Internals;

	// Methods
	public:
							// Lifecycle methods
							CVideoDecoder(const SVideo::Format& videoFormat, const I<CDecodeVideoCodec>& videoCodec,
									const CVideoProcessor::Format& videoProcessorFormat, const CString& identifier);
							CVideoDecoder(const CVideoDecoder& other);
							~CVideoDecoder();

							// CVideoProcessor methods
		TArray<CString>		getSetupDescription(const CString& indent);

		void				setSourceWindow(UniversalTimeInterval startTimeInterval,
									const OV<UniversalTimeInterval>& durationTimeInterval);
		void				seek(UniversalTimeInterval timeInterval);

		PerformResult		perform();
		void				reset();

	// Properties
	private:
		Internals*	mInternals;
};
