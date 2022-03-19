//----------------------------------------------------------------------------------------------------------------------
//	CAACAudioCodec.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioCodec.h"
#include "SMediaPacket.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAACAudioCodec

class CAACAudioCodecInternals;
class CAACAudioCodec : public CDecodeOnlyAudioCodec {
	// Decode info
	public:
		class DecodeInfo : public CCodec::MediaPacketSourceDecodeInfo {
			// Methods
			public:
								// Lifecycle methods
								DecodeInfo(const I<CMediaPacketSource>& mediaPacketSource,
										const CData& configurationData);

								// Instance methods
				const	CData&	getMagicCookie() const
									{ return mMagicCookie; }
						UInt16	getStartCodes() const
									{ return mStartCodes; }

			// Properties
			private:
				CData	mMagicCookie;
				UInt16	mStartCodes;
		};

	// Methods
	public:
											// Lifecycle methods
											CAACAudioCodec(OSType codecID);
											~CAACAudioCodec();

											// CAudioCodec methods - Decoding
				OI<SError>					setupForDecode(const SAudioProcessingFormat& audioProcessingFormat,
													const I<CCodec::DecodeInfo>& decodeInfo);
				CAudioFrames::Requirements	getRequirements() const;
				void						seek(UniversalTimeInterval timeInterval);
				OI<SError>					decodeInto(CAudioFrames& audioFrames);

											// Class methods
		static	OI<SAudioStorageFormat>		composeAudioStorageFormat(const CData& configurationData, UInt16 channels);
		static	I<CCodec::DecodeInfo>		composeDecodeInfo(const I<CSeekableDataSource>& seekableDataSource,
													const TArray<SMediaPacketAndLocation>& packetAndLocations,
													const CData& configurationData);

	// Properties
	public:
		static	OSType						mAACLCID;
		static	CString						mAACLCName;

		static	OSType						mAACLDID;
		static	CString						mAACLDName;

	private:
				CAACAudioCodecInternals*	mInternals;
};
