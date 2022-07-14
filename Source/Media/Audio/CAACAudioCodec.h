//----------------------------------------------------------------------------------------------------------------------
//	CAACAudioCodec.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioCodec.h"
#include "SMediaPacket.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAACAudioCodec

class CAACAudioCodec {
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
										// Class methods
		static	OI<SAudioStorageFormat>	composeAudioStorageFormat(const CData& configurationData, UInt16 channels);
		static	I<CDecodeAudioCodec>	create(const SAudioStorageFormat& audioStorageFormat,
												const I<CRandomAccessDataSource>& randomAccessDataSource,
												const TArray<SMediaPacketAndLocation>& packetAndLocations,
												const CData& configurationData);

	// Properties
	public:
		static	const	OSType	mAACLCID;
		static	const	CString	mAACLCName;

		static	const	OSType	mAACLDID;
		static	const	CString	mAACLDName;
};
