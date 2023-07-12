//----------------------------------------------------------------------------------------------------------------------
//	CAACAudioCodec.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioCodec.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAACAudioCodec

class CAACAudioCodec {
	// Info
	public:
		struct Info {
			// Methods
			public:
											// Lifecycle methods
											Info(OSType codecID, Float32 sampleRate,
													const SAudio::ChannelMap& audioChannelMap,
													const CData& magicCookie, UInt16 startCodes) :
												mCodecID(codecID), mSampleRate(sampleRate),
														mAudioTrackChannelMap(audioChannelMap),
														mMagicCookie(magicCookie), mStartCodes(startCodes)
												{}
											Info(const Info& other) :
												mCodecID(other.mCodecID), mSampleRate(other.mSampleRate),
														mAudioTrackChannelMap(other.mAudioTrackChannelMap),
														mMagicCookie(other.mMagicCookie),
														mStartCodes(other.mStartCodes)
												{}

											// Instance methods
						OSType				getCodecID() const
												{ return mCodecID; }
						Float32				getSampleRate() const
												{ return mSampleRate; }
				const	SAudio::ChannelMap&	getAudioChannelMap() const
												{ return mAudioTrackChannelMap; }
				const	CData&				getMagicCookie() const
												{ return mMagicCookie; }
						UInt16				getStartCodes() const
												{ return mStartCodes; }

			// Properties
			private:
				OSType				mCodecID;
				Float32				mSampleRate;
				SAudio::ChannelMap	mAudioTrackChannelMap;
				CData				mMagicCookie;
				UInt16				mStartCodes;
		};

	// Methods
	public:
										// Class methods
		static	OV<Info>				composeInfo(const CData& configurationData, UInt16 channels);
		static	SAudio::Format			composeAudioFormat(const Info& info);
		static	I<CDecodeAudioCodec>	create(const Info& info,
												const I<CRandomAccessDataSource>& randomAccessDataSource,
												const TArray<SMedia::PacketAndLocation>& packetAndLocations);

	// Properties
	public:
		static	const	OSType	mAACLCID;
		static	const	OSType	mAACLDID;
};
