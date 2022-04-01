//----------------------------------------------------------------------------------------------------------------------
//	CDVIIntelIMAADPCMAudioCodec.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioCodec.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDVIIntelIMAADPCMAudioCodec

class CDVIIntelIMAADPCMAudioCodec {
	// Methods
	public:
										// Class methods
		static	OI<SAudioStorageFormat>	composeAudioStorageFormat(Float32 sampleRate, EAudioChannelMap channelMap);
		static	UInt64					composeFrameCount(const SAudioStorageFormat& audioStorageFormat,
												UInt64 byteCount);
		static	I<CDecodeAudioCodec>	create(const SAudioStorageFormat& audioStorageFormat,
												const I<CSeekableDataSource>& seekableDataSource,
												UInt64 startByteOffset, UInt64 byteCount);

	// Properties
	public:
		static	OSType	mID;
};
