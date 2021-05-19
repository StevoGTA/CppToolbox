//----------------------------------------------------------------------------------------------------------------------
//	CAACAudioCodec.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioCodec.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAACAudioCodec

class CAACAudioCodecInternals;
class CAACAudioCodec : public CDecodeOnlyAudioCodec {
	// Decode info
	public:
		class DecodeInfo : public CCodec::PacketsDecodeInfo {
			// Methods
			public:
								// Lifecycle methods
								DecodeInfo(const TArray<PacketAndLocation>& packetAndLocations,
										const CData& magicCookie, UInt16 startCodes) :
									PacketsDecodeInfo(packetAndLocations), mMagicCookie(magicCookie),
											mStartCodes(startCodes)
									{}

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
				void					setupForDecode(const SAudioProcessingFormat& audioProcessingFormat,
												const I<CDataSource>& dataSource,
												const I<CCodec::DecodeInfo>& decodeInfo);
				SAudioReadStatus		decode(const SMediaPosition& mediaPosition, CAudioFrames& audioFrames);

										// Class methods
		static	OI<SAudioStorageFormat>	composeStorageFormat(UInt16 startCodes, UInt16 channels);

	// Properties
	public:
		static	OSType						mAACLCID;
		static	OSType						mAACLDID;

	private:
				CAACAudioCodecInternals*	mInternals;
};
