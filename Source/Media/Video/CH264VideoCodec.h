//----------------------------------------------------------------------------------------------------------------------
//	CH264VideoCodec.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CBitReader.h"
#include "CVideoCodec.h"
#include "SMediaPacket.h"
#include "SVideoFormats.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CH264VideoCodec

class CH264VideoCodec {
	// Methods
	public:
											// Class methods
		static	OV<SVideoStorageFormat>		composeVideoStorageFormat(const S2DSizeU16& frameSize, Float32 framerate);
		static	OV<I<CDecodeVideoCodec> >	create(const I<CRandomAccessDataSource>& randomAccessDataSource,
													const TArray<SMediaPacketAndLocation>& packetAndLocations,
													const CData& configurationData, UInt32 timeScale,
													const TNumberArray<UInt32>& keyframeIndexes);

	// Properties
	public:
		static	const	OSType	mID;
		static	const	CString	mName;
};
