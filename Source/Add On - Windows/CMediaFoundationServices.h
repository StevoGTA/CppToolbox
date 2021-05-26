//----------------------------------------------------------------------------------------------------------------------
//	CMediaFoundationServices.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioFrames.h"
#include "CByteParceller.h"
#include "SAudioFormats.h"
#include "TOptional-Windows.h"

#undef Delete

#include <mftransform.h>

#define Delete(x)	{ delete x; x = nil; }

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaFoundationServices

class CMediaFoundationServices {
	// Procs
	typedef	OI<SError>	ReadInputProc(IMFMediaBuffer* mediaBuffer, void* userData);

	// Methods
	public:
									// Class methods
		static	OCI<IMFTransform>	createAudioDecoder(const GUID& guid,
											const SAudioProcessingFormat& audioProcessingFormat,
											const OI<CData>& userData = OI<CData>());
		static	OCI<IMFSample>		createSample(UInt32 size);

		static	OI<SError>			flush(IMFTransform* transform);
		static	OI<SError>			readSample(CByteParceller& byteParceller, SInt64 position, UInt64 byteCount,
											IMFMediaBuffer* mediaBuffer);

		static	OI<SError>			processOutput(IMFTransform* transform, IMFSample* inputSample,
											IMFSample* outputSample, ReadInputProc readInputProc, void* userData);

		static	OI<SError>			copySample(IMFSample* sample, CAudioFrames& audioFrames,
											const SAudioProcessingFormat& audioProcessingFormat);
};
