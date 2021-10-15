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
		class DecodeInfo : public CCodec::DecodeInfo {
			// Methods
			public:
								// Lifecycle methods
								DecodeInfo(const I<CMediaPacketSource>& mediaPacketSource,
										const CData& magicCookie, UInt16 startCodes) :
									CCodec::DecodeInfo(mediaPacketSource), mMagicCookie(magicCookie),
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
				void						setupForDecode(const SAudioProcessingFormat& audioProcessingFormat,
													const I<CCodec::DecodeInfo>& decodeInfo);
				CAudioFrames::Requirements	getRequirements() const;
				void						seek(UniversalTimeInterval timeInterval);
				OI<SError>					decode(CAudioFrames& audioFrames);

											// Class methods
		static	OI<SAudioStorageFormat>		composeStorageFormat(UInt16 startCodes, UInt16 channels);

	// Properties
	public:
		static	OSType						mAACLCID;
		static	OSType						mAACLDID;

	private:
				CAACAudioCodecInternals*	mInternals;
};
