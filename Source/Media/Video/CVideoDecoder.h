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
								const I<CCodec::DecodeInfo>& codecDecodeInfo,
								const I<CSeekableDataSource>& seekableDataSource,
								CVideoFrame::Compatibility compatibility);
						CVideoDecoder(const CVideoDecoder& other);
						~CVideoDecoder();

						// CVideoProcessor methods
		PerformResult	perform(const SMediaPosition& mediaPosition);
		OI<SError>		reset();

	// Properties
	private:
		CVideoDecoderInternals*	mInternals;
};
