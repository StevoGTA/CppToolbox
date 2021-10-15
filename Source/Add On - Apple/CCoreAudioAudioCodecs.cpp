//----------------------------------------------------------------------------------------------------------------------
//	CCoreAudioAudioCodecs.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAACAudioCodec.h"

#include "CByteReader.h"
#include "CLogServices-Apple.h"
#include "SError-Apple.h"

#include <AudioToolbox/AudioToolbox.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAACAudioCodecInternals

class CAACAudioCodecInternals {
	public:
							CAACAudioCodecInternals(OSType codecID) :
								mCodecID(codecID),
									mAudioConverterRef(nil), mDecodeFramesToIgnore(0),
									mInputPacketData((CData::Size) 10 * 1024),
									mInputPacketDescriptionsData((CData::Size) 1 * 1024)
								{}
							~CAACAudioCodecInternals()
								{
									// Cleanup
									if (mAudioConverterRef != nil)
										::AudioConverterDispose(mAudioConverterRef);
								}

		static	OSStatus	fillBufferData(AudioConverterRef inAudioConverter, UInt32* ioNumberDataPackets,
									AudioBufferList* ioBufferList,
									AudioStreamPacketDescription** outDataPacketDescription, void* inUserData)
								{
									// Setup
									CAACAudioCodecInternals&	internals = *((CAACAudioCodecInternals*) inUserData);

									// Read packets
									TIResult<TArray<SMediaPacket> >	mediaPacketsResult =
																			(*internals.mDecodeInfo)->
																					getMediaPacketSource()->
																					getMediaPackets(
																							internals.mInputPacketData);
									ReturnValueIfResultError(mediaPacketsResult, -1);

									// Prepare return info
									const	TArray<SMediaPacket>&	mediaPackets = mediaPacketsResult.getValue();

									*ioNumberDataPackets = mediaPackets.getCount();

									ioBufferList->mNumberBuffers = 1;
									ioBufferList->mBuffers[0].mData = internals.mInputPacketData.getMutableBytePtr();
									ioBufferList->mBuffers[0].mDataByteSize =
											(UInt32) internals.mInputPacketData.getSize();

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
									for (TIteratorD<SMediaPacket> iterator = mediaPackets.getIterator();
											iterator.hasValue(); iterator.advance(), packetDescription++) {
										// Update
										packetDescription->mStartOffset = offset;
										packetDescription->mVariableFramesInPacket = iterator->mDuration;
										packetDescription->mDataByteSize = iterator->mByteCount;

										offset += iterator->mByteCount;
									}

									return noErr;
								}

		OSType						mCodecID;
		OI<SAudioProcessingFormat>	mAudioProcessingFormat;
		OI<I<CCodec::DecodeInfo> >	mDecodeInfo;

		AudioConverterRef			mAudioConverterRef;
		UInt32						mDecodeFramesToIgnore;
		CData						mInputPacketData;
		CData						mInputPacketDescriptionsData;
		OI<SError>					mFillBufferDataError;
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
void CAACAudioCodec::setupForDecode(const SAudioProcessingFormat& audioProcessingFormat,
		const I<CCodec::DecodeInfo>& decodeInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	DecodeInfo&	aacDecodeInfo = *((DecodeInfo*) &*decodeInfo);

	// Store
	mInternals->mAudioProcessingFormat = OI<SAudioProcessingFormat>(audioProcessingFormat);
	mInternals->mDecodeInfo = OI<I<CCodec::DecodeInfo> >(decodeInfo);

	// Create Audio Converter
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

	OSStatus	status = ::AudioConverterNew(&sourceFormat, &destinationFormat, &mInternals->mAudioConverterRef);
	LogOSStatusIfFailed(status, OSSTR("AudioConverterNew"));

	// Set magic cookie
	status =
			::AudioConverterSetProperty(mInternals->mAudioConverterRef,
					kAudioConverterDecompressionMagicCookie, (UInt32) aacDecodeInfo.getMagicCookie().getSize(),
					aacDecodeInfo.getMagicCookie().getBytePtr());
	LogOSStatusIfFailed(status, OSSTR("AudioConverterSetProperty for magic cookie"));
}

//----------------------------------------------------------------------------------------------------------------------
void CAACAudioCodec::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset audio converter
	::AudioConverterReset(mInternals->mAudioConverterRef);

	// Seek
	mInternals->mDecodeFramesToIgnore =
			(*mInternals->mDecodeInfo)->getMediaPacketSource()->seekToDuration(
					(UInt32) (timeInterval * mInternals->mAudioProcessingFormat->getSampleRate()));
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CAACAudioCodec::decode(CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf(audioFrames.getAvailableFrameCount() < (1024 * 2));

	// Setup
	AudioBufferList	audioBufferList;
	audioBufferList.mNumberBuffers = 1;
	audioBufferList.mBuffers[0].mNumberChannels = mInternals->mAudioProcessingFormat->getChannels();

	// Check if have frames to ignore
	OSStatus	status;
	if (mInternals->mDecodeFramesToIgnore > 0) {
		// Decode these frames, but throw away
		audioFrames.getAsWrite(audioBufferList);
		status =
				::AudioConverterFillComplexBuffer(mInternals->mAudioConverterRef,
						CAACAudioCodecInternals::fillBufferData, mInternals, &mInternals->mDecodeFramesToIgnore,
						&audioBufferList, nil);
		if (status != noErr) return mInternals->mFillBufferDataError;
		if (mInternals->mDecodeFramesToIgnore == 0) return OI<SError>(SError::mEndOfData);

		// Done ignoring
		mInternals->mDecodeFramesToIgnore = 0;
	}

	// Fill buffer
	UInt32	frameCount = audioFrames.getAsWrite(audioBufferList);
	status =
			::AudioConverterFillComplexBuffer(mInternals->mAudioConverterRef, CAACAudioCodecInternals::fillBufferData,
					mInternals, &frameCount, &audioBufferList, nil);
	if (status != noErr) return mInternals->mFillBufferDataError;
	if (frameCount == 0) return OI<SError>(SError::mEndOfData);

	// Update
	audioFrames.completeWrite(audioBufferList);

	return OI<SError>();
}
