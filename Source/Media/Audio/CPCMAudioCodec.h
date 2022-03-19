//----------------------------------------------------------------------------------------------------------------------
//	CPCMAudioCodec.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioCodec.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CPCMAudioCodec

class CPCMAudioCodecInternals;
class CPCMAudioCodec : public CDecodeOnlyAudioCodec {
	// Decode info
	public:
		class DecodeInfo : public CAudioCodec::FrameSourceDecodeInfo {
			// Methods
			public:
						// Lifecycle methods
						DecodeInfo(const I<CSeekableDataSource>& seekableDataSource, UInt64 startByteOffset,
								UInt64 byteCount, UInt8 frameByteCount, bool _8BitIsUnsigned) :
							FrameSourceDecodeInfo(seekableDataSource, startByteOffset, byteCount, frameByteCount),
									m8BitIsUnsigned(_8BitIsUnsigned)
							{}

						// Instance methods
				bool	get8BitIsUnsigned() const
							{ return m8BitIsUnsigned; }

			// Properties
			private:
				bool	m8BitIsUnsigned;
		};

	// Methods
	public:
											// Lifecycle methods
											CPCMAudioCodec();
											~CPCMAudioCodec();

											// CAudioCodec methods - Decoding
				OI<SError>					setupForDecode(const SAudioProcessingFormat& audioProcessingFormat,
													const I<CCodec::DecodeInfo>& decodeInfo);
				CAudioFrames::Requirements	getRequirements() const
												{ return CAudioFrames::Requirements(1, 1); }
				void						seek(UniversalTimeInterval timeInterval);
				OI<SError>					decodeInto(CAudioFrames& audioFrames);

											// Class methods
		static	OI<SAudioStorageFormat>		composeAudioStorageFormat(bool isFloat, UInt8 bits, Float32 sampleRate,
													EAudioChannelMap channelMap);
		static	OI<SAudioStorageFormat>		composeAudioStorageFormat(bool isFloat, UInt8 bits, Float32 sampleRate,
													UInt8 channels)
												{ return composeAudioStorageFormat(isFloat, bits, sampleRate,
														AUDIOCHANNELMAP_FORUNKNOWN(channels)); }
		static	UInt64						composeFrameCount(const SAudioStorageFormat& audioStorageFormat,
													UInt64 byteCount);
		static	I<CCodec::DecodeInfo>		composeDecodeInfo(const SAudioStorageFormat& audioStorageFormat,
													const I<CSeekableDataSource>& seekableDataSource,
													UInt64 startByteOffset, UInt64 byteCount, bool _8BitIsUnsigned);

	// Properties
	public:
		static	OSType						mFloatID;
		static	OSType						mIntegerID;

	private:
				CPCMAudioCodecInternals*	mInternals;
};
