//----------------------------------------------------------------------------------------------------------------------
//	CDVIIntelIMAADPCMAudioCodec.cpp			©2020 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDVIIntelIMAADPCMAudioCodec.h"

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

#pragma pack(push, 1)

struct SStateInfo {
	SInt16	mIndex;
	SInt32	mPreviousValue;
};

struct SDVIIntelPacketHeader {
	SInt16	mInitialSample;
	UInt8	mInitialIndex;
	UInt8	mZero;
};

#pragma pack(pop)

const	UInt32	kDVIIntelFramesPerPacket = 505;
const	UInt32	kDVIIntelBytesPerPacketPerChannel =
						sizeof(SDVIIntelPacketHeader) + (kDVIIntelFramesPerPacket - 1) / 2;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDVIIntelIMAADPCMDecodeAudioCodec

class CDVIIntelIMAADPCMDecodeAudioCodec : public CDecodeAudioCodec {
	public:
										// Lifecycle methods
										CDVIIntelIMAADPCMDecodeAudioCodec(
												const I<CMediaPacketSource>& mediaPacketSource) :
											mMediaPacketSourceDecodeInfo(mediaPacketSource),
													mDecodeFramesToIgnore(0)
											{}

										// CAudioCodec methods - Decoding
		TArray<SAudioProcessingSetup>	getAudioProcessingSetups(const SAudioStorageFormat& audioStorageFormat);
		OI<SError>						setup(const SAudioProcessingFormat& audioProcessingFormat);
		CAudioFrames::Requirements		getRequirements() const
											{ return CAudioFrames::Requirements(kDVIIntelFramesPerPacket,
													kDVIIntelFramesPerPacket * 2); }
		void							seek(UniversalTimeInterval timeInterval);
		OI<SError>						decodeInto(CAudioFrames& audioFrames);

										// Private methods
		SInt16	sampleForDeltaCode(UInt8 deltaCode, SStateInfo& stateInfo)
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

						stateInfo.mPreviousValue += diff;
						if (stateInfo.mPreviousValue > 0x7FFF)
							stateInfo.mPreviousValue = 0x7FFF;
						else if (stateInfo.mPreviousValue < -0x7FFF)
							stateInfo.mPreviousValue = -0x7FFF;

						return (SInt16) stateInfo.mPreviousValue;
					}

	private:
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
	AssertFailIf(audioFrames.getAvailableFrameCount() < (kDVIIntelFramesPerPacket * 2));

	// Setup
	UInt16	channels = mAudioProcessingFormat->getChannels();

	// Decode packets
	CAudioFrames::Info	writeInfo = audioFrames.getWriteInfo();
	UInt32				remainingFrames = writeInfo.getFrameCount();
	SInt16*				bufferPtr = (SInt16*) writeInfo.getSegments()[0];
	UInt32				decodedFrameCount = 0;
	while (remainingFrames >= kDVIIntelFramesPerPacket) {
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
		const	UInt8*	packetBufferPtr = (const UInt8*) dataInfo.getValue().getData().getBytePtr();

		// Setup state infos by decoding packet headers
		TBuffer<SStateInfo>	stateInfos(channels);
		for (UInt16 channel = 0; channel < channels; channel++, packetBufferPtr += sizeof(SDVIIntelPacketHeader)) {
			// Get info
			SDVIIntelPacketHeader*	packetHeader = (SDVIIntelPacketHeader*) packetBufferPtr;

			// Setup state infos
			stateInfos[channel].mPreviousValue = EndianS16_LtoN(packetHeader->mInitialSample);
			stateInfos[channel].mIndex = packetHeader->mInitialIndex;

			// Copy first sample
			bufferPtr[channel] = (SInt16) stateInfos[channel].mPreviousValue;
		}

		// Iterate sample blocks
		for (UInt32 sampleBlockIndex = 0; sampleBlockIndex < ((kDVIIntelFramesPerPacket - 1) / 8); sampleBlockIndex++) {
			// Loop channels
			for (UInt8 channel = 0; channel < channels; channel++) {
				// Setup
				SStateInfo&	stateInfo = stateInfos[channel];
				SInt16*		decodedSamplePtr =
									bufferPtr +							// Start
									channels +							// Initial samples
									sampleBlockIndex * channels * 8 +	// Blocks of samples already completed
									channel;							// Channel index

				// Decode 4 bytes / 8 samples
				UInt8	deltaCodes = *(packetBufferPtr++);
				*decodedSamplePtr = sampleForDeltaCode(deltaCodes & 0x0f, stateInfo);
				decodedSamplePtr += channels;
				*decodedSamplePtr = sampleForDeltaCode(deltaCodes >> 4, stateInfo);
				decodedSamplePtr += channels;

				deltaCodes = *(packetBufferPtr++);
				*decodedSamplePtr = sampleForDeltaCode(deltaCodes & 0x0f, stateInfo);
				decodedSamplePtr += channels;
				*decodedSamplePtr = sampleForDeltaCode(deltaCodes >> 4, stateInfo);
				decodedSamplePtr += channels;

				deltaCodes = *(packetBufferPtr++);
				*decodedSamplePtr = sampleForDeltaCode(deltaCodes & 0x0f, stateInfo);
				decodedSamplePtr += channels;
				*decodedSamplePtr = sampleForDeltaCode(deltaCodes >> 4, stateInfo);
				decodedSamplePtr += channels;

				deltaCodes = *(packetBufferPtr++);
				*decodedSamplePtr = sampleForDeltaCode(deltaCodes & 0x0f, stateInfo);
				decodedSamplePtr += channels;
				*decodedSamplePtr = sampleForDeltaCode(deltaCodes >> 4, stateInfo);
			}
		}

		// Packet decoded
		bufferPtr += kDVIIntelFramesPerPacket * (CData::ByteCount) channels;
		decodedFrameCount += kDVIIntelFramesPerPacket;
		remainingFrames -= kDVIIntelFramesPerPacket;
		audioFrames.completeWrite(kDVIIntelFramesPerPacket);
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
UInt64 CDVIIntelIMAADPCMAudioCodec::composeFrameCount(const SAudioStorageFormat& audioStorageFormat, UInt64 byteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt64	bytesPerPacket = kDVIIntelBytesPerPacketPerChannel * (UInt64) audioStorageFormat.getChannels();

	return byteCount / bytesPerPacket * kDVIIntelFramesPerPacket;
}

//----------------------------------------------------------------------------------------------------------------------
I<CDecodeAudioCodec> CDVIIntelIMAADPCMAudioCodec::create(const SAudioStorageFormat& audioStorageFormat,
		const I<CSeekableDataSource>& seekableDataSource, UInt64 startByteOffset, UInt64 byteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt32	bytesPerPacket = kDVIIntelBytesPerPacketPerChannel * audioStorageFormat.getChannels();

	return I<CDecodeAudioCodec>(
			new CDVIIntelIMAADPCMDecodeAudioCodec(
					new CSeekableUniformMediaPacketSource(seekableDataSource, startByteOffset, byteCount,
							bytesPerPacket, kDVIIntelFramesPerPacket)));
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Declare audio codecs

REGISTER_CODEC(dviIntelIMA,
		CAudioCodec::Info(CDVIIntelIMAADPCMAudioCodec::mID, CString(OSSTR("DVI/Intel IMA ADPCM 4:1"))));
