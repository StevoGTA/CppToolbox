//----------------------------------------------------------------------------------------------------------------------
//	CIMAADPCMAudioCodec.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioCodec.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CIMAADPCMCoder

class CIMAADPCMCoder {
	// StateInfo
	public:
		class StateInfo {
			// Methods
			public:
					// Lifecycle methods
					StateInfo() : mSample(0), mIndex(0) {}
					StateInfo(SInt16 sample, SInt16 index) : mSample(sample), mIndex(index) {}

					// Instance methods
			SInt16	getSample() const
						{ return mSample; }
			void	setSample(SInt16 sample)
						{ mSample = sample; }
			SInt16	getIndex() const
						{ return mIndex; }

			void	decodeDeltaCodes(const UInt8* deltaCodesPtr, SInt16* samplePtr, UInt8 channelCount,
							UInt16 sampleCount);
			void	encodeSamples(const SInt16* samplePtr, UInt8 channelCount, UInt16 sampleCount,
							UInt8* deltaCodesPtr);

			// Properties
			private:
				SInt16	mSample;
				SInt16	mIndex;
		};
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CIMAADPCMDecoder

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
		void	decodeGrouped(const UInt8* packetPtr, UInt32 channelHeaderByteCount, UInt32 samplesPerGroup,
						UInt32 groupCount);
		void	decodeUngrouped(const UInt8* packetPtr, UInt32 channelHeaderByteCount, UInt32 samplesPerChannel);

	// Properties
	private:
		Internals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDVIIntelIMAADPCMAudioCodec

class CDVIIntelIMAADPCMAudioCodec {
	// ChannelHeader
	public:
#pragma pack(push, 1)
		struct ChannelHeader {
			// Methods
			public:
						// Lifecycle methods
						ChannelHeader(const CIMAADPCMCoder::StateInfo& stateInfo) :
							mInitialSample(EndianS16_NtoL(stateInfo.getSample())),
									mInitialIndex((UInt8) stateInfo.getIndex()), mZero(0)
							{}

						// Instance methods
				SInt16	getInitialSample() const
							{ return EndianS16_LtoN(mInitialSample); }
				SInt16	getInitialIndex() const
							{ return mInitialIndex; }

			// Properties (in storage endian)
			private:
				SInt16	mInitialSample;
				UInt8	mInitialIndex;
				UInt8	mZero;
		};
#pragma pack(pop)

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
