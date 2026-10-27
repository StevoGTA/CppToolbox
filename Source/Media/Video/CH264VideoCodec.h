//----------------------------------------------------------------------------------------------------------------------
//	CH264VideoCodec.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CVideoCodec.h"
#include "SMedia.h"
#include "SVideo.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CH264VideoCodec

class CH264VideoCodec {
	// Methods
	public:
										// Class methods
		static	SVideo::Format			composeVideoTrackFormat(const S2DSizeU16& frameSize, UInt32 frameCount,
												UniversalTimeInterval duration);
		static	I<CDecodeVideoCodec>	create(const I<CRandomAccessDataSource>& randomAccessDataSource,
												const TArray<SMedia::PacketAndLocation>& packetAndLocations,
												const CData& configurationData, UInt32 timeScale,
												const TNumberArray<UInt32>& keyframeIndexes);

	// Properties
	public:
		static	const	OSType	mID;
		static	const	CString	mName;
};
