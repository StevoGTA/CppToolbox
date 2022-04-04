//----------------------------------------------------------------------------------------------------------------------
//	CIMAADPCMAudioCodec.cpp			©2020 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CIMAADPCMAudioCodec.h"

#include "CCodecRegistry.h"
#include "SMediaPacket.h"
#include "TBuffer.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

// IMA ADPCM info
static	const	SInt16	sStepSizeTable[] = {
	7,     8,     9,     10,    11,    12,     13,    14,    16,
	17,    19,    21,    23,    25,    28,     31,    34,    37,
	41,    45,    50,    55,    60,    66,     73,    80,    88,
	97,    107,   118,   130,   143,   157,    173,   190,   209,
	230,   253,   279,   307,   337,   371,    408,   449,   494,
	544,   598,   658,   724,   796,   876,    963,   1060,  1166,
	1282,  1411,  1552,  1707,  1878,  2066,   2272,  2499,  2749,
	3024,  3327,  3660,  4026,  4428,  4871,   5358,  5894,  6484,
	7132,  7845,  8630,  9493,  10442, 11487,  12635, 13899, 15289,
	16818, 18500, 20350, 22385, 24623, 27086,  29794, 32767};

static	const	SInt16	sIndexAdjustTable[] = {
	-1,	-1,	-1,	-1,	// +0 - +3, decrease the step size
	 2,	 4,	 6,	 8,	// +4 - +7, increase the step size
	-1,	-1,	-1,	-1,	// -0 - -3, decrease the step size
	 2,	 4,	 6,	 8	// -4 - -7, increase the step size
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CIMAADPCMDecoderInternals

class CIMAADPCMDecoderInternals {
	public:
		struct StateInfo {
			// Lifecycle methods
			StateInfo() : mSample(0), mIndex(0) {}
			StateInfo(SInt16 sample, SInt16 index) : mSample(sample), mIndex(index) {}

			SInt32	mSample;
			SInt16	mIndex;
		};

				CIMAADPCMDecoderInternals(SInt16* framePtr, UInt8 channels) :
					mFramePtr(framePtr), mStateInfos(channels)
					{}

		void	decodeForDeltaCode(SInt16* samplePtr, UInt8 deltaCode, StateInfo& stateInfo)
					{
						// Setup
						SInt32	step = sStepSizeTable[stateInfo.mIndex];
						SInt32	diff = step >> 3;
						if (deltaCode & 0x01) diff += (step >> 2);
						if (deltaCode & 0x02) diff += (step >> 1);
						if (deltaCode & 0x04) diff += step;
						if (deltaCode & 0x08) diff = -diff;

						// Update
						stateInfo.mIndex += sIndexAdjustTable[deltaCode];
						if (stateInfo.mIndex > 88)
							stateInfo.mIndex = 88;
						else if (stateInfo.mIndex < 0)
							stateInfo.mIndex = 0;

						stateInfo.mSample += diff;
						if (stateInfo.mSample > 0x7FFF)
							stateInfo.mSample = 0x7FFF;
						else if (stateInfo.mSample < -0x7FFF)
							stateInfo.mSample = -0x7FFF;

						// Store sample
						*samplePtr = stateInfo.mSample;
					}

		SInt16*				mFramePtr;
		TBuffer<StateInfo>	mStateInfos;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CIMAADPCMDecoder

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CIMAADPCMDecoder::CIMAADPCMDecoder(SInt16* framePtr, UInt8 channels)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CIMAADPCMDecoderInternals(framePtr, channels);
}

//----------------------------------------------------------------------------------------------------------------------
CIMAADPCMDecoder::~CIMAADPCMDecoder()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CIMAADPCMDecoder::initChannel(UInt8 channel, SInt16 sample, SInt16 index)
//----------------------------------------------------------------------------------------------------------------------
{
	// Init state info
	mInternals->mStateInfos[channel] = CIMAADPCMDecoderInternals::StateInfo(sample, index);
}

//----------------------------------------------------------------------------------------------------------------------
void CIMAADPCMDecoder::emitSamplesFromState()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt8	channels = mInternals->mStateInfos.getCount();

	// Iterate channels
	for (UInt8 channel = 0; channel < channels; channel++)
		// Emit sample
		*mInternals->mFramePtr++ = mInternals->mStateInfos[channel].mSample;
}

//----------------------------------------------------------------------------------------------------------------------
void CIMAADPCMDecoder::decodeInterleaved(const UInt8* packetPtr, UInt32 channelHeaderByteCount, UInt32 samplesPerGroup,
		UInt32 groupCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt8	channels = mInternals->mStateInfos.getCount();

	// Iterate groups
	for (UInt32 groupIndex = 0; groupIndex < groupCount; groupIndex++) {
		// Loop channels
		for (UInt8 channel = 0; channel < channels; channel++) {
			// Setup
					CIMAADPCMDecoderInternals::StateInfo&	stateInfo = mInternals->mStateInfos[channel];
			const	UInt8*									encodedSamplePtr =
																	packetPtr +
																	channelHeaderByteCount * channels +
																	groupIndex * samplesPerGroup / 2 * channels +
																	samplesPerGroup / 2 * channel;
					SInt16*									decodedSamplePtr = mInternals->mFramePtr + channel;

			// Iterate samples in group
			for (UInt8 sampleIndex = 0; sampleIndex < samplesPerGroup; sampleIndex += 2) {
				// Decode 4 bytes / 8 samples
				UInt8	deltaCodes = *(encodedSamplePtr++);
				mInternals->decodeForDeltaCode(decodedSamplePtr, deltaCodes & 0x0f, stateInfo);
				decodedSamplePtr += channels;
				mInternals->decodeForDeltaCode(decodedSamplePtr, deltaCodes >> 4, stateInfo);
				decodedSamplePtr += channels;
			}
		}

		// Update
		mInternals->mFramePtr += samplesPerGroup * channels;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CIMAADPCMDecoder::decodeNoninterleaved(const UInt8* packetPtr, UInt32 channelHeaderByteCount,
		UInt32 samplesPerChannel)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt8	channels = mInternals->mStateInfos.getCount();

	// Iterate channels
	for (UInt8 channel = 0; channel < channels; channel++) {
		// Setup
				CIMAADPCMDecoderInternals::StateInfo&	stateInfo = mInternals->mStateInfos[channel];
		const	UInt8*									encodedSamplePtr =
																packetPtr +
																(channelHeaderByteCount + samplesPerChannel / 2) *
																		channel +
																channelHeaderByteCount;
				SInt16*									decodedSamplePtr = mInternals->mFramePtr + channel;

		// Iterate samples in group
		for (UInt8 sampleIndex = 0; sampleIndex < samplesPerChannel; sampleIndex += 2) {
			// Decode 4 bytes / 8 samples
			UInt8	deltaCodes = *(encodedSamplePtr++);
			mInternals->decodeForDeltaCode(decodedSamplePtr, deltaCodes & 0x0f, stateInfo);
			decodedSamplePtr += channels;
			mInternals->decodeForDeltaCode(decodedSamplePtr, deltaCodes >> 4, stateInfo);
			decodedSamplePtr += channels;
		}
	}

	// Update
	mInternals->mFramePtr += samplesPerChannel * channels;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDVIIntelIMAADPCMDecodeAudioCodec

#pragma pack(push, 1)

struct SDVIIntelChannelHeader {
			// Instance methods
	SInt16	getInitialSample() const
				{ return EndianS16_LtoN(mInitialSample); }
	SInt16	getInitialIndex() const
				{ return mInitialIndex; }

	private:
		SInt16	mInitialSample;
		UInt8	mInitialIndex;
		UInt8	mZero;
};

#pragma pack(pop)

class CDVIIntelIMAADPCMDecodeAudioCodec : public CDecodeAudioCodec {
	public:
										// Lifecycle methods
										CDVIIntelIMAADPCMDecodeAudioCodec(UInt32 framesPerPacket,
												const I<CMediaPacketSource>& mediaPacketSource) :
											mFramesPerPacket(framesPerPacket),
													mMediaPacketSourceDecodeInfo(mediaPacketSource),
													mDecodeFramesToIgnore(0)
											{}

										// CAudioCodec methods - Decoding
		TArray<SAudioProcessingSetup>	getAudioProcessingSetups(const SAudioStorageFormat& audioStorageFormat);
		OI<SError>						setup(const SAudioProcessingFormat& audioProcessingFormat);
		CAudioFrames::Requirements		getRequirements() const
											{ return CAudioFrames::Requirements(mFramesPerPacket,
													mFramesPerPacket * 2); }
		void							seek(UniversalTimeInterval timeInterval);
		OI<SError>						decodeInto(CAudioFrames& audioFrames);

	private:
		UInt32								mFramesPerPacket;
		CCodec::MediaPacketSourceDecodeInfo	mMediaPacketSourceDecodeInfo;

		OI<SAudioProcessingFormat>			mAudioProcessingFormat;
		UInt32								mDecodeFramesToIgnore;
};

// MARK: CAudioCodec methods - Decoding

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudioProcessingSetup> CDVIIntelIMAADPCMDecodeAudioCodec::getAudioProcessingSetups(
		const SAudioStorageFormat& audioStorageFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudioProcessingSetup>(
				SAudioProcessingSetup(16, audioStorageFormat.getSampleRate(), audioStorageFormat.getChannelMap()));
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CDVIIntelIMAADPCMDecodeAudioCodec::setup(const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mAudioProcessingFormat = OI<SAudioProcessingFormat>(audioProcessingFormat);

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
void CDVIIntelIMAADPCMDecodeAudioCodec::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Seek
	mDecodeFramesToIgnore =
			mMediaPacketSourceDecodeInfo.getMediaPacketSource()->seekToDuration(
					(UInt32) (timeInterval * mAudioProcessingFormat->getSampleRate()));
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CDVIIntelIMAADPCMDecodeAudioCodec::decodeInto(CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf(audioFrames.getAvailableFrameCount() < (mFramesPerPacket * 2));

	// Setup
	UInt8	channels = mAudioProcessingFormat->getChannels();

	// Decode packets
	CAudioFrames::Info	writeInfo = audioFrames.getWriteInfo();
	UInt32				remainingFrames = writeInfo.getFrameCount();
	SInt16*				bufferPtr = (SInt16*) writeInfo.getSegments()[0];
	UInt32				decodedFrameCount = 0;
	while (remainingFrames >= mFramesPerPacket) {
		// Get next packet
		TIResult<CMediaPacketSource::DataInfo>	dataInfo =
														mMediaPacketSourceDecodeInfo.getMediaPacketSource()->readNext();
		if (dataInfo.hasError()) {
			// Check situation
			if (decodedFrameCount > 0)
				// EOF, but have decoded frames
				break;
			else
				// EOF, no decoded frames
				return OI<SError>(dataInfo.getError());
		}

		// Decode packet
		const	UInt8*	packetPtr = (const UInt8*) dataInfo.getValue().getData().getBytePtr();

		// Setup IMA/ADPCM decoder
		CIMAADPCMDecoder	imaADCPMDecoder(bufferPtr, channels);
		for (UInt16 channel = 0; channel < channels; channel++) {
			// Get info
			const	SDVIIntelChannelHeader&	channelHeader = ((SDVIIntelChannelHeader*) packetPtr)[channel];

			// Init this channel
			imaADCPMDecoder.initChannel(channel, channelHeader.getInitialSample(), channelHeader.getInitialIndex());
		}

		// Emit samples from header
		imaADCPMDecoder.emitSamplesFromState();

		// Decode packet
		imaADCPMDecoder.decodeInterleaved(packetPtr, sizeof(SDVIIntelChannelHeader), 8, (mFramesPerPacket - 1) / 8);
		audioFrames.completeWrite(mFramesPerPacket);

		// Update
		bufferPtr += mFramesPerPacket * channels;
		decodedFrameCount += mFramesPerPacket;
		remainingFrames -= mFramesPerPacket;
		mDecodeFramesToIgnore = 0;
	}

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDVIIntelIMAADPCMAudioCodec

// MARK: Properties

OSType	CDVIIntelIMAADPCMAudioCodec::mID = 0x6D730011;

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
OI<SAudioStorageFormat> CDVIIntelIMAADPCMAudioCodec::composeAudioStorageFormat(Float32 sampleRate,
		EAudioChannelMap channelMap)
//----------------------------------------------------------------------------------------------------------------------
{
	return OI<SAudioStorageFormat>(new SAudioStorageFormat(mID, sampleRate, channelMap));
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CDVIIntelIMAADPCMAudioCodec::composeFrameCount(const SAudioStorageFormat& audioStorageFormat, UInt64 byteCount,
		UInt16 blockAlign)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt64	framesPerPacket =
					((UInt64) blockAlign / (UInt64) audioStorageFormat.getChannels() - sizeof(SDVIIntelChannelHeader)) *
							2 + 1;

	return byteCount / (UInt64) blockAlign * framesPerPacket;
}

//----------------------------------------------------------------------------------------------------------------------
I<CDecodeAudioCodec> CDVIIntelIMAADPCMAudioCodec::create(const SAudioStorageFormat& audioStorageFormat,
		const I<CSeekableDataSource>& seekableDataSource, UInt64 startByteOffset, UInt64 byteCount, UInt16 blockAlign)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt32	framesPerPacket =
					((UInt32) blockAlign / (UInt32) audioStorageFormat.getChannels() - sizeof(SDVIIntelChannelHeader)) *
							2 + 1;

	return I<CDecodeAudioCodec>(
			new CDVIIntelIMAADPCMDecodeAudioCodec(framesPerPacket,
					new CSeekableUniformMediaPacketSource(seekableDataSource, startByteOffset, byteCount,
							blockAlign, framesPerPacket)));
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Declare audio codecs

REGISTER_CODEC(dviIntelIMA,
		CAudioCodec::Info(CDVIIntelIMAADPCMAudioCodec::mID, CString(OSSTR("DVI/Intel IMA ADPCM 4:1"))));
