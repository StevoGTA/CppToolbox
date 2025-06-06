//----------------------------------------------------------------------------------------------------------------------
//	CMediaFoundationDecodeVideoCodec.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CMediaPacketSource.h"
#include "CVideoCodec.h"
#include "TResult-Windows.h"

#undef Delete

#include <mfapi.h>

#define Delete(x)	{ delete x; x = nil; }

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaFoundationDecodeVideoCodec

class CMediaFoundationDecodeVideoCodecInternals;
class CMediaFoundationDecodeVideoCodec : public CDecodeVideoCodec {
	// Procs
	public:
		typedef TCIResult<IMFSample>	(*ReadInputSampleProc)(
												CMediaFoundationDecodeVideoCodec& mediaFoundationDecodeVideoCodec);

	// Methods
	public:
										// Lifecycle methods
										~CMediaFoundationDecodeVideoCodec();

										// CDecodeVideoCodec methods
				OV<SError>				setup(const CVideoProcessor::Format& videoProcessorFormat);
				void					seek(UniversalTimeInterval timeInterval);
				TIResult<CVideoFrame>	decode();

										// Instance methods
				UInt32					getTimeScale() const;

	protected:
										// Lifecycle methods
										CMediaFoundationDecodeVideoCodec(OSType codecID,
												const I<CMediaPacketSource>& mediaPacketSource, UInt32 timeScale,
												const TNumberArray<UInt32>& keyframeIndexes,
												ReadInputSampleProc readInputSampleProc);

										// Subclass methods
		virtual	OR<const GUID>			getGUID() const = 0;
		virtual	void					seek(UInt64 frameTime) {}

	// Properties
	private:
		CMediaFoundationDecodeVideoCodecInternals*	mInternals;
};
