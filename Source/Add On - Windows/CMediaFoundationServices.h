//----------------------------------------------------------------------------------------------------------------------
//	CMediaFoundationServices.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioFrames.h"
#include "SMediaPacket.h"
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
		static	OCI<IMFTransform>	createTransformForAudioDecode(const GUID& guid,
											const SAudioProcessingFormat& audioProcessingFormat,
											const OI<CData>& userData = OI<CData>());
		static	OCI<IMFTransform>	createTransformForVideoDecode(const GUID& guid);
		static	OI<SError>			flush(IMFTransform* transform);

		static	OCI<IMFSample>		createSample(UInt32 size);
		static	OCI<IMFSample>		createSample(const CData& data);
		static	OI<SError>			completeAudioFramesWrite(IMFSample* sample, CAudioFrames& audioFrames,
											const SAudioProcessingFormat& audioProcessingFormat);

		static	OI<SError>			processOutput(IMFTransform* transform, IMFSample* inputSample,
											IMFSample* outputSample, ReadInputProc readInputProc, void* userData);

		static	OI<SError>			load(IMFMediaBuffer* mediaBuffer, CPacketMediaReader& packetMediaReader);
};
