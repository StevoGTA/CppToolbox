//----------------------------------------------------------------------------------------------------------------------
//	CDVIIntelIMAADPCMAudioCodec.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CCodecRegistry.h"
#include "SMediaPacket.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDVIIntelIMAADPCMAudioCodec

class CDVIIntelIMAADPCMAudioCodecInternals;
class CDVIIntelIMAADPCMAudioCodec : public CDecodeOnlyAudioCodec {
	// Methods
	public:
											// Lifecycle methods
											CDVIIntelIMAADPCMAudioCodec();
											~CDVIIntelIMAADPCMAudioCodec();

											// CAudioCodec methods - Decoding
				OI<SError>					setupForDecode(const SAudioProcessingFormat& audioProcessingFormat,
													const I<CCodec::DecodeInfo>& decodeInfo);
				CAudioFrames::Requirements	getRequirements() const;
				void						seek(UniversalTimeInterval timeInterval);
				OI<SError>					decodeInto(CAudioFrames& audioFrames);

											// Class methods
		static	OI<SAudioStorageFormat>		composeAudioStorageFormat(Float32 sampleRate, EAudioChannelMap channelMap);
		static	UInt64						composeFrameCount(const SAudioStorageFormat& audioStorageFormat,
													UInt64 byteCount);
		static	I<CCodec::DecodeInfo>		composeDecodeInfo(const SAudioStorageFormat& audioStorageFormat,
													const I<CSeekableDataSource>& seekableDataSource,
													UInt64 startByteOffset, UInt64 byteCount);

	// Properties
	public:
		static	OSType									mID;

	private:
				CDVIIntelIMAADPCMAudioCodecInternals*	mInternals;
};
