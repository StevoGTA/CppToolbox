//----------------------------------------------------------------------------------------------------------------------
//	CPCMAudioCodec.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioCodec.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CPCMAudioCodec

class CPCMAudioCodec {
	// Format
	public:
		enum Format {
			kFormatBigEndian,
			kFormatLittleEndian,
			kFormat8BitSigned,
			kFormat8BitUnsigned,
		};

	// Methods
	public:
										// Class methods
		static	OI<SAudioStorageFormat>	composeAudioStorageFormat(bool isFloat, UInt8 bits, Float32 sampleRate,
												EAudioChannelMap channelMap);
		static	OI<SAudioStorageFormat>	composeAudioStorageFormat(bool isFloat, UInt8 bits, Float32 sampleRate,
												UInt8 channels)
											{ return composeAudioStorageFormat(isFloat, bits, sampleRate,
													AUDIOCHANNELMAP_FORUNKNOWN(channels)); }
		static	UInt64					composeFrameCount(const SAudioStorageFormat& audioStorageFormat,
												UInt64 byteCount);
		static	I<CDecodeAudioCodec>	create(const SAudioStorageFormat& audioStorageFormat,
												const I<CSeekableDataSource>& seekableDataSource,
												UInt64 startByteOffset, UInt64 byteCount, Format format);

	// Properties
	public:
		static	OSType	mFloatID;
		static	OSType	mIntegerID;
};
