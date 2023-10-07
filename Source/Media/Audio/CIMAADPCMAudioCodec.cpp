//----------------------------------------------------------------------------------------------------------------------
//	CIMAADPCMAudioCodec.cpp			©2020 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CIMAADPCMAudioCodec.h"

#include "CCodecRegistry.h"
#include "CMediaPacketSource.h"
#include "TBuffer.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

// IMA ADPCM info
static	const	SInt16	sStepSizeTable[] =
							{
								7,     8,     9,     10,    11,    12,     13,    14,    16,
								17,    19,    21,    23,    25,    28,     31,    34,    37,
								41,    45,    50,    55,    60,    66,     73,    80,    88,
								97,    107,   118,   130,   143,   157,    173,   190,   209,
								230,   253,   279,   307,   337,   371,    408,   449,   494,
								544,   598,   658,   724,   796,   876,    963,   1060,  1166,
								1282,  1411,  1552,  1707,  1878,  2066,   2272,  2499,  2749,
								3024,  3327,  3660,  4026,  4428,  4871,   5358,  5894,  6484,
								7132,  7845,  8630,  9493,  10442, 11487,  12635, 13899, 15289,
								16818, 18500, 20350, 22385, 24623, 27086,  29794, 32767,
							};

static	const	SInt16	sIndexAdjustTable[] =
							{
								-1,	-1,	-1,	-1,	// +0 - +3, decrease the step size
								 2,	 4,	 6,	 8,	// +4 - +7, increase the step size
								-1,	-1,	-1,	-1,	// -0 - -3, decrease the step size
								 2,	 4,	 6,	 8,	// -4 - -7, increase the step size
							};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CIMAADPCMCoder::StateInfo

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CIMAADPCMCoder::StateInfo::decodeDeltaCodes(const UInt8* deltaCodesPtr, SInt16* samplePtr, UInt8 channelCount,
		UInt16 sampleCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate samples
	for (UInt16 sampleIndex = 0; sampleIndex < sampleCount; sampleIndex += 2) {
		// Setup
		UInt8	deltaCodes = *(deltaCodesPtr++);
		UInt8	deltaCode;
		SInt32	step, diff, sample;

		// Do first delta code
		deltaCode = deltaCodes & 0x0f;
		step = sStepSizeTable[mIndex];
		diff = step >> 3;
		if (deltaCode & 0x01) diff += (step >> 2);
		if (deltaCode & 0x02) diff += (step >> 1);
		if (deltaCode & 0x04) diff += step;
		if (deltaCode & 0x08) diff = -diff;

		// Update
		mIndex += sIndexAdjustTable[deltaCode];
		if (mIndex > 88)
			mIndex = 88;
		else if (mIndex < 0)
			mIndex = 0;

		sample = mSample + diff;
		if (sample > 0x7FFF)
			mSample = 0x7FFF;
		else if (sample < -0x7FFF)
			mSample = -0x7FFF;
		else
			mSample = (SInt16) sample;
		*samplePtr = mSample;
		samplePtr += channelCount;

		// Do second delta code
		deltaCode = deltaCodes >> 4;
		step = sStepSizeTable[mIndex];
		diff = step >> 3;
		if (deltaCode & 0x01) diff += (step >> 2);
		if (deltaCode & 0x02) diff += (step >> 1);
		if (deltaCode & 0x04) diff += step;
		if (deltaCode & 0x08) diff = -diff;

		// Update
		mIndex += sIndexAdjustTable[deltaCode];
		if (mIndex > 88)
			mIndex = 88;
		else if (mIndex < 0)
			mIndex = 0;

		sample = mSample + diff;
		if (sample > 0x7FFF)
			mSample = 0x7FFF;
		else if (sample < -0x7FFF)
			mSample = -0x7FFF;
		else
			mSample = (SInt16) sample;
		samplePtr += channelCount;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CIMAADPCMCoder::StateInfo::encodeSamples(const SInt16* samplePtr, UInt8 channelCount, UInt16 sampleCount,
		UInt8* deltaCodesPtr)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate samples
	for (UInt16 sampleIndex = 0; sampleIndex < sampleCount; sampleIndex += 2) {
		// Setup
		SInt32	diff, sample;
		SInt16	step;

		// Do first sample
		diff = (SInt32) *samplePtr - mSample;
		samplePtr += channelCount;

		step = sStepSizeTable[mIndex];

		UInt8	deltaCodeL = 0;

		// Set sign bit
		if (diff < 0) {
			deltaCodeL = 8;
			diff = -diff;
		}

		// Set top bit
		if (diff >= step) {
			deltaCodeL |= 4;
			diff -= step;
		}
		step >>= 1;

		// Set middle bit
		if (diff >= step) {
			deltaCodeL |= 2;
			diff -= step;
		}
		step >>= 1;

		// Set bottom bit
		if (diff >= step)
			deltaCodeL |= 1;

		// Update state
		step = sStepSizeTable[mIndex];
		diff = step >> 3;
		if (deltaCodeL & 1)	diff += step >> 2;
		if (deltaCodeL & 2)	diff += step >> 1;
		if (deltaCodeL & 4)	diff += step;
		if (deltaCodeL & 8)	diff = -diff;

		mIndex += sIndexAdjustTable[deltaCodeL];
		if (mIndex < 0)
			mIndex = 0;
		if (mIndex > 88)
			mIndex = 88;

		sample = mSample + diff;
		if (sample > 0x7FFF)
			mSample = 0x7FFF;
		else if (sample < -0x7FFF)
			mSample = -0x7FFF;
		else
			mSample = (SInt16) sample;

		// Do second sample
		diff = (SInt32) *samplePtr - mSample;
		samplePtr += channelCount;

		step = sStepSizeTable[mIndex];

		UInt8	deltaCodeH = 0;

		// Set sign bit
		if (diff < 0) {
			deltaCodeH = 8;
			diff = -diff;
		}

		// Set top bit
		if (diff >= step) {
			deltaCodeH |= 4;
			diff -= step;
		}
		step >>= 1;

		// Set middle bit
		if (diff >= step) {
			deltaCodeH |= 2;
			diff -= step;
		}
		step >>= 1;

		// Set bottom bit
		if (diff >= step)
			deltaCodeH |= 1;

		// Update state
		step = sStepSizeTable[mIndex];
		diff = step >> 3;
		if (deltaCodeH & 1)	diff += step >> 2;
		if (deltaCodeH & 2)	diff += step >> 1;
		if (deltaCodeH & 4)	diff += step;
		if (deltaCodeH & 8)	diff = -diff;

		mIndex += sIndexAdjustTable[deltaCodeH];
		if (mIndex < 0)
			mIndex = 0;
		if (mIndex > 88)
			mIndex = 88;

		sample = mSample + diff;
		if (sample > 0x7FFF)
			mSample = 0x7FFF;
		else if (sample < -0x7FFF)
			mSample = -0x7FFF;
		else
			mSample = (SInt16) sample;

		// Store delta codes
		*(deltaCodesPtr++) = (deltaCodeH << 4) | deltaCodeL;
	}
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CIMAADPCMDecoder::Internals

class CIMAADPCMDecoder::Internals {
	public:
		Internals(SInt16* framePtr, UInt8 channels) : mFramePtr(framePtr), mStateInfos(channels) {}

		SInt16*								mFramePtr;
		TBuffer<CIMAADPCMCoder::StateInfo>	mStateInfos;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CIMAADPCMDecoder

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CIMAADPCMDecoder::CIMAADPCMDecoder(SInt16* framePtr, UInt8 channels)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(framePtr, channels);
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
	mInternals->mStateInfos[channel] = CIMAADPCMCoder::StateInfo(sample, index);
}

//----------------------------------------------------------------------------------------------------------------------
void CIMAADPCMDecoder::emitSamplesFromState()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt8	channels = (UInt8) mInternals->mStateInfos.getCount();

	// Iterate channels
	for (UInt8 channel = 0; channel < channels; channel++)
		// Emit sample
		*mInternals->mFramePtr++ = mInternals->mStateInfos[channel].getSample();
}

//----------------------------------------------------------------------------------------------------------------------
void CIMAADPCMDecoder::decodeGrouped(const UInt8* packetPtr, UInt32 channelHeaderByteCount, UInt32 samplesPerGroup,
		UInt32 groupCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt8	channelCount = (UInt8) mInternals->mStateInfos.getCount();

	// Iterate groups
	for (UInt32 groupIndex = 0; groupIndex < groupCount; groupIndex++) {
		// Loop channels
		for (UInt8 channel = 0; channel < channelCount; channel++) {
			// Setup
			const	UInt8*	deltaCodesPtr =
									packetPtr +
									channelHeaderByteCount * channelCount +
									groupIndex * samplesPerGroup / 2 * channelCount +
									samplesPerGroup / 2 * channel;

			// Decode delta codes
			mInternals->mStateInfos[channel].decodeDeltaCodes(deltaCodesPtr, mInternals->mFramePtr + channel,
					channelCount, samplesPerGroup);
		}

		// Update
		mInternals->mFramePtr += samplesPerGroup * channelCount;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CIMAADPCMDecoder::decodeUngrouped(const UInt8* packetPtr, UInt32 channelHeaderByteCount,
		UInt32 samplesPerChannel)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt8	channelCount = (UInt8) mInternals->mStateInfos.getCount();

	// Iterate channels
	for (UInt8 channel = 0; channel < channelCount; channel++) {
		// Setup
		const	UInt8*	deltaCodesPtr =
								packetPtr +
								(channelHeaderByteCount + samplesPerChannel / 2) * channel +
								channelHeaderByteCount;

		// Decode delta codes
		mInternals->mStateInfos[channel].decodeDeltaCodes(deltaCodesPtr, mInternals->mFramePtr + channel, channelCount,
				samplesPerChannel);
	}

	// Update
	mInternals->mFramePtr += samplesPerChannel * channelCount;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDVIIntelIMAADPCMDecodeAudioCodec

class CDVIIntelIMAADPCMDecodeAudioCodec : public CDecodeAudioCodec {
	public:
										// Lifecycle methods
										CDVIIntelIMAADPCMDecodeAudioCodec(UInt32 framesPerPacket,
												const I<CMediaPacketSource>& mediaPacketSource) :
											mFramesPerPacket(framesPerPacket), mMediaPacketSource(mediaPacketSource),
													mDecodeFramesToIgnore(0)
											{}

										// CAudioCodec methods - Decoding
		TArray<SAudio::ProcessingSetup>	getAudioProcessingSetups(const SAudio::Format& audioFormat);
		OV<SError>						setup(const SAudio::ProcessingFormat& audioProcessingFormat);
		CAudioFrames::Requirements		getRequirements() const
											{ return CAudioFrames::Requirements(mFramesPerPacket,
													mFramesPerPacket * 2); }
		void							seek(UniversalTimeInterval timeInterval);
		OV<SError>						decodeInto(CAudioFrames& audioFrames);

	private:
		UInt32							mFramesPerPacket;
		I<CMediaPacketSource>			mMediaPacketSource;

		OV<SAudio::ProcessingFormat>	mAudioProcessingFormat;
		UInt32							mDecodeFramesToIgnore;
};

// MARK: CAudioCodec methods - Decoding

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudio::ProcessingSetup> CDVIIntelIMAADPCMDecodeAudioCodec::getAudioProcessingSetups(
		const SAudio::Format& audioFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudio::ProcessingSetup>(
				SAudio::ProcessingSetup(16, audioFormat.getSampleRate(), audioFormat.getChannelMap()));
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CDVIIntelIMAADPCMDecodeAudioCodec::setup(const SAudio::ProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mAudioProcessingFormat.setValue(audioProcessingFormat);

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
void CDVIIntelIMAADPCMDecodeAudioCodec::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Seek
	mDecodeFramesToIgnore =
			mMediaPacketSource->seekToDuration((UInt32) (timeInterval * mAudioProcessingFormat->getSampleRate()));
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CDVIIntelIMAADPCMDecodeAudioCodec::decodeInto(CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf(audioFrames.getAllocatedFrameCount() < (mFramesPerPacket * 2));

	// Setup
	UInt8	channels = mAudioProcessingFormat->getChannelMap().getChannelCount();

	// Decode packets
	CAudioFrames::Info	writeInfo = audioFrames.getWriteInfo();
	UInt32				remainingFrames = writeInfo.getFrameCount();
	SInt16*				bufferPtr = (SInt16*) writeInfo.getSegments()[0];
	UInt32				decodedFrameCount = 0;
	while (remainingFrames >= mFramesPerPacket) {
		// Get next packet
		TVResult<CMediaPacketSource::DataInfo>	dataInfo = mMediaPacketSource->readNext();
		if (dataInfo.hasError()) {
			// Check situation
			if (decodedFrameCount > 0)
				// EOF, but have decoded frames
				break;
			else
				// EOF, no decoded frames
				return OV<SError>(dataInfo.getError());
		}

		// Decode packet
		const	UInt8*	packetPtr = (const UInt8*) dataInfo->getData().getBytePtr();

		// Setup IMA/ADPCM decoder
		CIMAADPCMDecoder	imaADCPMDecoder(bufferPtr, channels);
		for (UInt8 channel = 0; channel < channels; channel++) {
			// Get info
			const	CDVIIntelIMAADPCMAudioCodec::ChannelHeader&	channelHeader =
																		((CDVIIntelIMAADPCMAudioCodec::ChannelHeader*)
																						packetPtr)
																				[channel];

			// Init this channel
			imaADCPMDecoder.initChannel(channel, channelHeader.getInitialSample(), channelHeader.getInitialIndex());
		}

		// Emit samples from header
		imaADCPMDecoder.emitSamplesFromState();

		// Decode packet
		imaADCPMDecoder.decodeGrouped(packetPtr, sizeof(CDVIIntelIMAADPCMAudioCodec::ChannelHeader), 8,
				(mFramesPerPacket - 1) / 8);
		audioFrames.completeWrite(mFramesPerPacket);

		// Update
		bufferPtr += mFramesPerPacket * channels;
		decodedFrameCount += mFramesPerPacket;
		remainingFrames -= mFramesPerPacket;
		mDecodeFramesToIgnore = 0;
	}

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDVIIntelIMAADPCMAudioCodec

// MARK: Properties

const	OSType	CDVIIntelIMAADPCMAudioCodec::mID = 0x6D730011;

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
SAudio::Format CDVIIntelIMAADPCMAudioCodec::composeAudioFormat(Float32 sampleRate,
		const SAudio::ChannelMap& audioChannelMap)
//----------------------------------------------------------------------------------------------------------------------
{
	return SAudio::Format(mID, sampleRate, audioChannelMap);
}

//----------------------------------------------------------------------------------------------------------------------
SMedia::SegmentInfo CDVIIntelIMAADPCMAudioCodec::composeMediaSegmentInfo( const SAudio::Format& audioFormat,
		UInt64 byteCount, UInt16 blockAlign)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt64	framesPerPacket =
					((UInt64) blockAlign / (UInt64) audioFormat.getChannelMap().getChannelCount() -
							sizeof(CDVIIntelIMAADPCMAudioCodec::ChannelHeader)) * 2 + 1;

	return SAudio::composeMediaSegmentInfo(audioFormat, byteCount / (UInt64) blockAlign * framesPerPacket, byteCount);
}

//----------------------------------------------------------------------------------------------------------------------
I<CDecodeAudioCodec> CDVIIntelIMAADPCMAudioCodec::create(const SAudio::Format& audioFormat,
		const I<CRandomAccessDataSource>& randomAccessDataSource, UInt64 startByteOffset, UInt64 byteCount,
		UInt16 blockAlign)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt32	framesPerPacket =
					((UInt32) blockAlign / (UInt32) audioFormat.getChannelMap().getChannelCount() -
							sizeof(CDVIIntelIMAADPCMAudioCodec::ChannelHeader)) * 2 + 1;

	return I<CDecodeAudioCodec>(
			new CDVIIntelIMAADPCMDecodeAudioCodec(framesPerPacket,
					new CSeekableUniformMediaPacketSource(randomAccessDataSource, startByteOffset, byteCount,
							blockAlign, framesPerPacket)));
}
