//----------------------------------------------------------------------------------------------------------------------
//	CAACAudioCodec.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioCodec.h"
#include "SMediaPacket.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAACAudioCodec

class CAACAudioCodec {
	// Info
	public:
		struct Info {
			// Methods
			public:
											// Lifecycle methods
											Info(OSType codecID, Float32 sampleRate, EAudioChannelMap audioChannelMap,
													const CData& magicCookie, UInt16 startCodes) :
												mCodecID(codecID), mSampleRate(sampleRate),
														mAudioChannelMap(audioChannelMap), mMagicCookie(magicCookie),
														mStartCodes(startCodes)
												{}
											Info(const Info& other) :
												mCodecID(other.mCodecID), mSampleRate(other.mSampleRate),
														mAudioChannelMap(other.mAudioChannelMap),
														mMagicCookie(other.mMagicCookie), mStartCodes(other.mStartCodes)
												{}

											// Instance methods
						OSType				getCodecID() const
												{ return mCodecID; }
						Float32				getSampleRate() const
												{ return mSampleRate; }
						EAudioChannelMap	getAudioChannelMap() const
												{ return mAudioChannelMap; }
				const	CData&				getMagicCookie() const
												{ return mMagicCookie; }
						UInt16				getStartCodes() const
												{ return mStartCodes; }

			// Properties
			private:
				OSType				mCodecID;
				Float32				mSampleRate;
				EAudioChannelMap	mAudioChannelMap;
				CData				mMagicCookie;
				UInt16				mStartCodes;
		};

	// Methods
	public:
											// Class methods
		static	OV<Info>					composeInfo(const CData& configurationData, UInt16 channels);
		static	SAudioStorageFormat			composeAudioStorageFormat(const Info& info);
		static	OV<I<CDecodeAudioCodec> >	create(const Info& info,
													const I<CRandomAccessDataSource>& randomAccessDataSource,
													const TArray<SMediaPacketAndLocation>& packetAndLocations);

	// Properties
	public:
		static	const	OSType	mAACLCID;
		static	const	CString	mAACLCName;

		static	const	OSType	mAACLDID;
		static	const	CString	mAACLDName;
};
