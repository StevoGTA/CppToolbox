//----------------------------------------------------------------------------------------------------------------------
//	CIMAADPCMAudioCodec.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioCodec.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CIMAADPCMDecoder

class CIMAADPCMDecoder {
	// Classes
	private:
		class Internals;

	// Methods
	public:
				// Lifecycle methods
				CIMAADPCMDecoder(SInt16* framePtr, UInt8 channels);
				~CIMAADPCMDecoder();

				// Instance methods
		void	initChannel(UInt8 channel, SInt16 sample, SInt16 index);
		void	emitSamplesFromState();
		void	decodeInterleaved(const UInt8* packetPtr, UInt32 channelHeaderByteCount, UInt32 samplesPerGroup,
						UInt32 groupCount);
		void	decodeNoninterleaved(const UInt8* packetPtr, UInt32 channelHeaderByteCount, UInt32 samplesPerChannel);

	// Properties
	private:
		Internals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDVIIntelIMAADPCMAudioCodec

class CDVIIntelIMAADPCMAudioCodec {
	// Methods
	public:
										// Class methods
		static	SAudio::Format			composeAudioFormat(Float32 sampleRate,
												const SAudio::ChannelMap& audioChannelMap);
		static	SMedia::SegmentInfo		composeMediaSegmentInfo(const SAudio::Format& audioFormat, UInt64 byteCount,
												UInt16 blockAlign);
		static	I<CDecodeAudioCodec>	create(const SAudio::Format& audioFormat,
												const I<CRandomAccessDataSource>& randomAccessDataSource,
												UInt64 startByteOffset, UInt64 byteCount, UInt16 blockAlign);

	// Properties
	public:
		static	const	OSType	mID;
};
