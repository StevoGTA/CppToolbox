//----------------------------------------------------------------------------------------------------------------------
//	CIMAADPCMAudioCodec.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioCodec.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CIMAADPCMDecoder

class CIMAADPCMDecoderInternals;
class CIMAADPCMDecoder {

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
		CIMAADPCMDecoderInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDVIIntelIMAADPCMAudioCodec

class CDVIIntelIMAADPCMAudioCodec {
	// Methods
	public:
											// Class methods
		static	OV<SAudioStorageFormat>		composeAudioStorageFormat(Float32 sampleRate,
													EAudioChannelMap audioChannelMap);
		static	OV<SAudioStorageFormat>		composeAudioStorageFormat(Float32 sampleRate, UInt8 channels)
												{ return composeAudioStorageFormat(sampleRate,
														AUDIOCHANNELMAP_FORUNKNOWN(channels)); }
		static	UInt64						composeFrameCount(const SAudioStorageFormat& audioStorageFormat,
													UInt64 byteCount, UInt16 blockAlign);
		static	OV<I<CDecodeAudioCodec> >	create(const SAudioStorageFormat& audioStorageFormat,
													const I<CRandomAccessDataSource>& randomAccessDataSource,
													UInt64 startByteOffset, UInt64 byteCount, UInt16 blockAlign);

	// Properties
	public:
		static	const	OSType	mID;
};
