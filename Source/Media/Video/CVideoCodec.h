//----------------------------------------------------------------------------------------------------------------------
//	CVideoCodec.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CCodec.h"
#include "SVideoFormats.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CVideoCodec

class CVideoCodec : public CCodec {
	// Methods
	protected:
		// Lifecycle methods
		CVideoCodec() : CCodec() {}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDecodeVideoCodec

class CDecodeVideoCodec : public CVideoCodec {
	// Methods
	public:
										// Instance methods
		virtual	OV<SError>				setup(const SVideoProcessingFormat& videoProcessingFormat) = 0;
		virtual	void					seek(UniversalTimeInterval timeInterval) = 0;
		virtual	TIResult<CVideoFrame>	decode() = 0;

	protected:
										// Lifecycle methods
										CDecodeVideoCodec() : CVideoCodec() {}
};
