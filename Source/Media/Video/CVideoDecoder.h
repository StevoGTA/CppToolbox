//----------------------------------------------------------------------------------------------------------------------
//	CVideoDecoder.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CVideoCodec.h"
#include "CVideoProcessor.h"
#include "SVideoFormats.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CVideoDecoder

class CVideoDecoderInternals;
class CVideoDecoder : public CVideoSource {
	// Methods
	public:
							// Lifecycle methods
						CVideoDecoder(const SVideoStorageFormat& videoStorageFormat,
								const I<CVideoCodec>& videoCodec,
								const I<CCodec::DecodeInfo>& codecDecodeInfo,
								const SVideoProcessingFormat& videoProcessingFormat);
						CVideoDecoder(const CVideoDecoder& other);
						~CVideoDecoder();

						// CVideoProcessor methods
		void			setSourceWindow(UniversalTimeInterval startTimeInterval,
								const OV<UniversalTimeInterval>& durationTimeInterval);
		void			seek(UniversalTimeInterval timeInterval);

		PerformResult	perform();
		void			reset();

	// Properties
	private:
		CVideoDecoderInternals*	mInternals;
};
