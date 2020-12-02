//----------------------------------------------------------------------------------------------------------------------
//	CDVIIntelIMAADPCMAudioCodec.cpp			©2020 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDVIIntelIMAADPCMAudioCodec.h"

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

#pragma pack(push,1)
struct SStateInfo {
	SInt16	mIndex;
	SInt32	mPreviousValue;
};
#pragma pack(pop)

#pragma pack(push,1)
struct SDVIIntelPacketHeader {
	SInt16	mInitialSample;
	UInt8	mInitialIndex;
	UInt8	mZero;
};
#pragma pack(pop)

const	UInt32	kDVIIntelFramesPerPacket = 505;
const	UInt32	kDVIIntelBytesPerPacketPerChannel =
						sizeof(SDVIIntelPacketHeader) + (kDVIIntelFramesPerPacket - 1) / 2;

const	UInt64	kPacketIndexNotSet = ~0;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDVIIntelIMAADPCMAudioCodecInternals

class CDVIIntelIMAADPCMAudioCodecInternals {
	public:
												CDVIIntelIMAADPCMAudioCodecInternals() :
													mDecodeCurrentPacketIndex(kPacketIndexNotSet)
													{}

				SInt16							sampleForDeltaCode(UInt8 deltaCode, SStateInfo& stateInfo)
													{
														// Setup
														SInt32	step = sStepSizeTable[stateInfo.mIndex];
														SInt32	diff = step >> 3;
														if (deltaCode & 0x01) diff += (step >> 2);
														if (deltaCode & 0x02) diff += (step >> 1);
														if (deltaCode & 0x04) diff += step;
														if (deltaCode & 0x08) diff = -diff;

														// Update
														stateInfo.mIndex =
																std::max<SInt32>(
																		std::min<SInt32>(
																				stateInfo.mIndex +
																						sIndexAdjustTable[deltaCode],
																				88),
																		0);
														stateInfo.mPreviousValue =
																std::max<SInt32>(
																		std::min<SInt32>(
																				stateInfo.mPreviousValue + diff,
																				0x7FFF),
																		-0x7FFF);

														return stateInfo.mPreviousValue;
													}

												// Class methods
		static	I<CAudioCodec>					instantiate(OSType id)
													{
														return I<CAudioCodec>(new CDVIIntelIMAADPCMAudioCodec());
													}
		static	TArray<SAudioProcessingSetup>	getAudioProcessingSetups(OSType id,
														const SAudioStorageFormat& audioStorageFormat)
													{
														return TNArray<SAudioProcessingSetup>(
																SAudioProcessingSetup(*audioStorageFormat.getBits(),
																		audioStorageFormat.getSampleRate(),
																		audioStorageFormat.getChannelMap()));
													}

		OI<CByteParceller>			mByteParceller;
		OI<SAudioProcessingFormat>	mAudioProcessingFormat;
		UInt64						mDecodeCurrentPacketIndex;
};


//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDVIIntelIMAADPCMAudioCodec

// MARK: Properties

OSType	CDVIIntelIMAADPCMAudioCodec::mID = 0x6D730011;

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CDVIIntelIMAADPCMAudioCodec::CDVIIntelIMAADPCMAudioCodec()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CDVIIntelIMAADPCMAudioCodecInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CDVIIntelIMAADPCMAudioCodec::~CDVIIntelIMAADPCMAudioCodec()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CAudioCodec methods - Decoding

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudioProcessingSetup> CDVIIntelIMAADPCMAudioCodec::getDecodeAudioProcessingSetups(
		const SAudioStorageFormat& storedAudioSampleFormat) const
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudioProcessingSetup>(
			SAudioProcessingSetup(*storedAudioSampleFormat.getBits(), storedAudioSampleFormat.getSampleRate(),
					storedAudioSampleFormat.getChannelMap()));
}

//----------------------------------------------------------------------------------------------------------------------
void CDVIIntelIMAADPCMAudioCodec::setupForDecode(const SAudioProcessingFormat& audioProcessingFormat,
		CByteParceller& byteParceller, const I<CAudioCodec::CDecodeInfo>& decodeInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SDataDecodeInfo*	dataDecodeInfo = (SDataDecodeInfo*) &*decodeInfo;

	// Store
	mInternals->mByteParceller =
			OI<CByteParceller>(
					new CByteParceller(byteParceller, dataDecodeInfo->getStartOffset(), dataDecodeInfo->getSize()));
	mInternals->mAudioProcessingFormat = OI<SAudioProcessingFormat>(audioProcessingFormat);
}

//----------------------------------------------------------------------------------------------------------------------
SAudioReadStatus CDVIIntelIMAADPCMAudioCodec::decode(const SMediaPosition& mediaPosition, CAudioData& audioData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt16	channels = mInternals->mAudioProcessingFormat->getChannels();
	UInt32	bytesPerPacket = kDVIIntelBytesPerPacketPerChannel * channels;

	// Update read position if needed
	UInt64	nextPacketIndex = mInternals->mDecodeCurrentPacketIndex + 1;
	if (mediaPosition.getMode() == SMediaPosition::kFromStart) {
		// Get new sample position
		UInt64	frameIndex = mediaPosition.getFrameIndex(mInternals->mAudioProcessingFormat->getSampleRate());
		UInt64	packetIndex = (frameIndex + kDVIIntelFramesPerPacket - 1) / kDVIIntelFramesPerPacket;

		if (packetIndex != mInternals->mDecodeCurrentPacketIndex) {
			// Need to seek
			SInt64		byteIndex = packetIndex * bytesPerPacket;
			OI<SError>	error = mInternals->mByteParceller->setPos(kDataSourcePositionFromBeginning, byteIndex);
			if (error.hasInstance())
				return SAudioReadStatus(*error);

			mInternals->mDecodeCurrentPacketIndex = kPacketIndexNotSet;
			nextPacketIndex = packetIndex;
		}
	}

	// Decode packets
	SInt16*	bufferPtr = (SInt16*) (*audioData.getBuffers())[0];
	UInt32	availableFrameCount = audioData.getAvailableFrameCount();
	UInt32	decodedFrameCount = 0;
	while (availableFrameCount >= kDVIIntelFramesPerPacket) {
		// Read next packet
		TBuffer<UInt8>	packetBuffer(bytesPerPacket);
		OI<SError>		error = mInternals->mByteParceller->readData(*packetBuffer, bytesPerPacket);
		if (error.hasInstance()) {
			// Check situation
			if (decodedFrameCount > 0)
				// EOF, but have decoded frames
				break;
			else
				// EOF, no decoded frames
				return SAudioReadStatus(*error);
		}

		// Decode packet
		const	UInt8*	packetBufferPtr = *packetBuffer;

		// Setup state infos by decoding packet headers
		SStateInfo	stateInfos[channels];
		for (UInt16 channel = 0; channel < channels; channel++, packetBufferPtr += sizeof(SDVIIntelPacketHeader)) {
			// Get info
			SDVIIntelPacketHeader*	packetHeader = (SDVIIntelPacketHeader*) packetBufferPtr;

			// Setup state infos
			stateInfos[channel].mPreviousValue = EndianS16_LtoN(packetHeader->mInitialSample);
			stateInfos[channel].mIndex = packetHeader->mInitialIndex;

			// Copy first sample
			bufferPtr[channel] = stateInfos[channel].mPreviousValue;
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
				*decodedSamplePtr = mInternals->sampleForDeltaCode(deltaCodes & 0x0f, stateInfo);
				decodedSamplePtr += channels;
				*decodedSamplePtr = mInternals->sampleForDeltaCode(deltaCodes >> 4, stateInfo);
				decodedSamplePtr += channels;

				deltaCodes = *(packetBufferPtr++);
				*decodedSamplePtr = mInternals->sampleForDeltaCode(deltaCodes & 0x0f, stateInfo);
				decodedSamplePtr += channels;
				*decodedSamplePtr = mInternals->sampleForDeltaCode(deltaCodes >> 4, stateInfo);
				decodedSamplePtr += channels;

				deltaCodes = *(packetBufferPtr++);
				*decodedSamplePtr = mInternals->sampleForDeltaCode(deltaCodes & 0x0f, stateInfo);
				decodedSamplePtr += channels;
				*decodedSamplePtr = mInternals->sampleForDeltaCode(deltaCodes >> 4, stateInfo);
				decodedSamplePtr += channels;

				deltaCodes = *(packetBufferPtr++);
				*decodedSamplePtr = mInternals->sampleForDeltaCode(deltaCodes & 0x0f, stateInfo);
				decodedSamplePtr += channels;
				*decodedSamplePtr = mInternals->sampleForDeltaCode(deltaCodes >> 4, stateInfo);
			}
		}

		// Packet decoded
		bufferPtr += kDVIIntelFramesPerPacket * channels;
		decodedFrameCount += kDVIIntelFramesPerPacket;
		availableFrameCount -= kDVIIntelFramesPerPacket;
	}

	// Update
	audioData.completeWrite(decodedFrameCount);

	return SAudioReadStatus(
			(Float32) mInternals->mByteParceller->getPos() / (Float32) mInternals->mByteParceller->getSize());
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Declare audio codecs

REGISTER_AUDIO_CODEC(DVIIntelIMA,
		CAudioCodec::SInfo(CDVIIntelIMAADPCMAudioCodec::mID, CString("DVI/Intel IMA ADPCM 4:1"),
				CDVIIntelIMAADPCMAudioCodecInternals::getAudioProcessingSetups,
				CDVIIntelIMAADPCMAudioCodecInternals::instantiate));
