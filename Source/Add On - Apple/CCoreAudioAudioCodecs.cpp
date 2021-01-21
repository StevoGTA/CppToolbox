//----------------------------------------------------------------------------------------------------------------------
//	CCoreAudioAudioCodecs.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAACAudioCodec.h"

#include "CLogServices-Apple.h"
#include "SError-Apple.h"

#include <AudioToolbox/AudioToolbox.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAACAudioCodecInternals

class CAACAudioCodecInternals {
	public:
							CAACAudioCodecInternals(OSType codecID) :
									mCodecID(codecID),
									mAudioConverterRef(nil), mInputPacketData((CData::Size) 10 * 1024),
									mInputPacketDescriptionsData((CData::Size) 1 * 1024),
									mNextPacketIndex(0)
								{}
							~CAACAudioCodecInternals()
								{
									if (mAudioConverterRef != nil)
										::AudioConverterDispose(mAudioConverterRef);
								}

		static	OSStatus	fillBufferData(AudioConverterRef inAudioConverter, UInt32* ioNumberDataPackets,
									AudioBufferList* ioBufferList,
									AudioStreamPacketDescription** outDataPacketDescription, void* inUserData)
								{
									// Setup
									CAACAudioCodecInternals&	internals = *((CAACAudioCodecInternals*) inUserData);

									// Add packets
									UInt32							available = internals.mInputPacketData.getSize();
									UInt8*							packetDataPtr =
																			(UInt8*) internals.mInputPacketData
																					.getMutableBytePtr();
									TNArray<CAudioCodec::Packet>	packets;
									while (internals.mNextPacketIndex < internals.mPacketLocations->getCount()) {
										// Setup
										CAudioCodec::PacketLocation&	packetLocation =
																				internals.mPacketLocations->getAt(
																						internals.mNextPacketIndex);

										// Check if have space
										if (packetLocation.mPacket.mSize <= available) {
											// Add packet
											OI<SError>	error =
																internals.mByteParceller->setPos(
																		CDataSource::kPositionFromBeginning,
																		packetLocation.mPos);
											ReturnValueIfError(error, -1);

											error =
													internals.mByteParceller->readData(packetDataPtr,
															packetLocation.mPacket.mSize);
											ReturnValueIfError(error, -1);

											packets += packetLocation.mPacket;

											// Update
											internals.mNextPacketIndex++;

											available -= packetLocation.mPacket.mSize;
											packetDataPtr += packetLocation.mPacket.mSize;
										} else
											// No more space
											break;
									}

									// Prepare return info
									*ioNumberDataPackets = packets.getCount();

									ioBufferList->mNumberBuffers = 1;
									ioBufferList->mBuffers[0].mData = internals.mInputPacketData.getMutableBytePtr();
									ioBufferList->mBuffers[0].mDataByteSize =
											internals.mInputPacketData.getSize() - available;

									if ((*ioNumberDataPackets * sizeof(AudioStreamPacketDescription)) >
											internals.mInputPacketDescriptionsData.getSize())
										// Increase packet descriptions data size
										internals.mInputPacketDescriptionsData.setSize(
												*ioNumberDataPackets * sizeof(AudioStreamPacketDescription));

									AudioStreamPacketDescription*	packetDescription =
																			(AudioStreamPacketDescription*)
																					internals
																							.mInputPacketDescriptionsData
																							.getMutableBytePtr();
									*outDataPacketDescription = packetDescription;

									SInt64	offset = 0;
									for (TIteratorD<CAudioCodec::Packet> iterator = packets.getIterator();
											iterator.hasValue(); iterator.advance(), packetDescription++) {
										// Update
										packetDescription->mStartOffset = offset;
										packetDescription->mVariableFramesInPacket = iterator->mSampleCount;
										packetDescription->mDataByteSize = iterator->mSize;

										offset += iterator->mSize;
									}

									return noErr;
								}

		OSType										mCodecID;
		OI<CByteParceller>							mByteParceller;
		OI<SAudioProcessingFormat>					mAudioProcessingFormat;
		OI<TArray<CAudioCodec::PacketLocation> >	mPacketLocations;

		AudioConverterRef							mAudioConverterRef;
		CData										mInputPacketData;
		CData										mInputPacketDescriptionsData;
		OI<SError>									mFillBufferDataError;

		UInt32										mNextPacketIndex;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAACAudioCodec

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAACAudioCodec::CAACAudioCodec(OSType codecID)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CAACAudioCodecInternals(codecID);
}

//----------------------------------------------------------------------------------------------------------------------
CAACAudioCodec::~CAACAudioCodec()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CAudioCodec methods - Decoding

//----------------------------------------------------------------------------------------------------------------------
void CAACAudioCodec::setupForDecode(const SAudioProcessingFormat& audioProcessingFormat, CByteParceller& byteParceller,
		const I<CAudioCodec::DecodeInfo>& decodeInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	PacketsWithMagicCookieDecodeInfo&	packetsWithMagicCookieDecodeInfo =
														*((PacketsWithMagicCookieDecodeInfo*) &*decodeInfo);

	// Store
	mInternals->mByteParceller = OI<CByteParceller>(byteParceller);
	mInternals->mAudioProcessingFormat = OI<SAudioProcessingFormat>(audioProcessingFormat);
	mInternals->mPacketLocations = OI<TArray<CAudioCodec::PacketLocation> >(
			packetsWithMagicCookieDecodeInfo.getPacketLocations());

	// Setup
	AudioStreamBasicDescription	sourceFormat = {0};
	sourceFormat.mFormatID = (mInternals->mCodecID == mAACLCID) ? kAudioFormatMPEG4AAC : kAudioFormatMPEG4AAC_LD;
	sourceFormat.mFormatFlags = 0;
	sourceFormat.mSampleRate = audioProcessingFormat.getSampleRate();
	sourceFormat.mChannelsPerFrame = audioProcessingFormat.getChannels();

	AudioStreamBasicDescription	destinationFormat;
	FillOutASBDForLPCM(destinationFormat, audioProcessingFormat.getSampleRate(),
			audioProcessingFormat.getChannels(), audioProcessingFormat.getBits(),
			audioProcessingFormat.getBits(), audioProcessingFormat.getIsFloat(),
			audioProcessingFormat.getIsBigEndian(), !audioProcessingFormat.getIsInterleaved());

	// Create Audio Converter
	OSStatus	status = ::AudioConverterNew(&sourceFormat, &destinationFormat, &mInternals->mAudioConverterRef);
	LOG_OSSTATUS_IF_FAILED(status, OSSTR("AudioConverterNew"));

	// Set magic cookie
	status =
			::AudioConverterSetProperty(mInternals->mAudioConverterRef,
					kAudioConverterDecompressionMagicCookie,
					packetsWithMagicCookieDecodeInfo.getMagicCookie().getSize(),
					packetsWithMagicCookieDecodeInfo.getMagicCookie().getBytePtr());
	LOG_OSSTATUS_IF_FAILED(status, OSSTR("AudioConverterSetProperty for magic cookie"));
}

//----------------------------------------------------------------------------------------------------------------------
SAudioReadStatus CAACAudioCodec::decode(const SMediaPosition& mediaPosition, CAudioData& audioData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update read position if needed
	if ((mediaPosition.getMode() != SMediaPosition::kFromCurrent) && (mInternals->mNextPacketIndex != 0)) {
		// Reset audio converter
		::AudioConverterReset(mInternals->mAudioConverterRef);

		// Update next packet index
		mInternals->mNextPacketIndex =
				getPacketIndex(mediaPosition, *mInternals->mAudioProcessingFormat, *mInternals->mPacketLocations);
	}

	// Setup
	Float32		sourceProcessed =
						(Float32) mInternals->mNextPacketIndex / (Float32) mInternals->mPacketLocations->getCount();

	AudioBufferList	audioBufferList;
	audioBufferList.mNumberBuffers = 1;
	audioData.getAsWrite(audioBufferList);

	// Fill buffer
	UInt32		frameCount = audioData.getAvailableFrameCount();
	OSStatus	status =
						::AudioConverterFillComplexBuffer(mInternals->mAudioConverterRef,
								CAACAudioCodecInternals::fillBufferData, mInternals, &frameCount, &audioBufferList,
								nil);
	if (status != noErr) return SAudioReadStatus(*mInternals->mFillBufferDataError);
	if (frameCount == 0) return SAudioReadStatus(SError::mEndOfData);

	// Update
	audioData.completeWrite(frameCount);

	return SAudioReadStatus(sourceProcessed);
}
