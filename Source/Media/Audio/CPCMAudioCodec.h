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
		static	SAudio::Format			composeAudioFormat(bool isFloat, UInt8 bits, Float32 sampleRate,
												const SAudio::ChannelMap& audioChannelMap);
		static	SMedia::SegmentInfo		composeMediaSegmentInfo(const SAudio::Format& audioFormat, UInt64 byteCount);

		static	I<CDecodeAudioCodec>	create(const SAudio::Format& audioFormat,
												const I<CRandomAccessDataSource>& randomAccessDataSource,
												UInt64 startByteOffset, UInt64 byteCount, Format format);

	// Properties
	public:
		static	const	OSType	mFloatID;
		static	const	OSType	mIntegerID;
};
