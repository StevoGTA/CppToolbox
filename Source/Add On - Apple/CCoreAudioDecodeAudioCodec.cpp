//----------------------------------------------------------------------------------------------------------------------
//	CCoreAudioDecodeAudioCodec.cpp			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CCoreAudioDecodeAudioCodec.h"

#include "SError-Apple.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreAudioDecodeAudioCodec::Internals

class CCoreAudioDecodeAudioCodec::Internals {
	public:
							Internals(OSType codecID, const I<CMediaPacketSource>& mediaPacketSource) :
								mCodecID(codecID), mMediaPacketSource(mediaPacketSource),
										mAudioConverterRef(nil), mDecodeFramesToIgnore(0),
										mInputPacketData((CData::ByteCount) 10 * 1024),
										mInputPacketDescriptionsData((CData::ByteCount) 1 * 1024)
								{}
							~Internals()
								{
									// Cleanup
									if (mAudioConverterRef != nil)
										::AudioConverterDispose(mAudioConverterRef);
								}

							// Class methods
		static	OSStatus	fillBufferData(AudioConverterRef inAudioConverter, UInt32* ioNumberDataPackets,
									AudioBufferList* ioBufferList,
									AudioStreamPacketDescription** outDataPacketDescription, void* inUserData)
								{
									// Setup
									Internals&	internals = *((Internals*) inUserData);

									// Read packets
									TVResult<TArray<SMedia::Packet> >	mediaPacketsResult =
																				internals.mMediaPacketSource->
																						readNextInto(
																								internals
																										.mInputPacketData);
									if (mediaPacketsResult.hasError()) {
										// Check error
										if (mediaPacketsResult.getError() == SError::mEndOfData) {
											// End of data
											*ioNumberDataPackets = 0;

											return noErr;
										} else {
											// Other error
											internals.mFillBufferDataError = mediaPacketsResult.getError();

											return -1;
										}
									}

									// Prepare return info
									const	TArray<SMedia::Packet>&	mediaPackets = *mediaPacketsResult;

									*ioNumberDataPackets = mediaPackets.getCount();

									if ((*ioNumberDataPackets * sizeof(AudioStreamPacketDescription)) >
											internals.mInputPacketDescriptionsData.getByteCount())
										// Increase packet descriptions data size
										internals.mInputPacketDescriptionsData.setByteCount(
												*ioNumberDataPackets * sizeof(AudioStreamPacketDescription));

									AudioStreamPacketDescription*	packetDescription =
																			(AudioStreamPacketDescription*)
																					internals
																							.mInputPacketDescriptionsData
																							.getMutableBytePtr();
									if (outDataPacketDescription != nil)
										//
										*outDataPacketDescription = packetDescription;

									SInt64	offset = 0;
									for (TArray<SMedia::Packet>::Iterator iterator = mediaPackets.getIterator();
											iterator; iterator++, packetDescription++) {
										// Update
										packetDescription->mStartOffset = offset;
										packetDescription->mVariableFramesInPacket = iterator->getDuration();
										packetDescription->mDataByteSize = iterator->getByteCount();

										offset += iterator->getByteCount();
									}

									ioBufferList->mNumberBuffers = 1;
									ioBufferList->mBuffers[0].mData = internals.mInputPacketData.getMutableBytePtr();
									ioBufferList->mBuffers[0].mDataByteSize = (UInt32) offset;

									return noErr;
								}

		OSType							mCodecID;
		I<CMediaPacketSource>			mMediaPacketSource;

		OV<SAudio::ProcessingFormat>	mAudioProcessingFormat;

		AudioConverterRef				mAudioConverterRef;
		UInt32							mDecodeFramesToIgnore;
		CData							mInputPacketData;
		CData							mInputPacketDescriptionsData;
		OV<SError>						mFillBufferDataError;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CCoreAudioDecodeAudioCodec

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CCoreAudioDecodeAudioCodec::CCoreAudioDecodeAudioCodec(OSType codecID, const I<CMediaPacketSource>& mediaPacketSource)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(codecID, mediaPacketSource);
}

//----------------------------------------------------------------------------------------------------------------------
CCoreAudioDecodeAudioCodec::~CCoreAudioDecodeAudioCodec()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CDecodeAudioCodec methods

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CCoreAudioDecodeAudioCodec::setup(const SAudio::ProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mAudioProcessingFormat.setValue(audioProcessingFormat);

	// Create Audio Converter
	AudioStreamBasicDescription	sourceABSD = getSourceASBD(mInternals->mCodecID, audioProcessingFormat);

	AudioStreamBasicDescription	destinationASBD;
	FillOutASBDForLPCM(destinationASBD, audioProcessingFormat.getSampleRate(),
			audioProcessingFormat.getChannelMap().getChannelCount(), audioProcessingFormat.getBits(),
			audioProcessingFormat.getBits(), audioProcessingFormat.getIsFloat(), audioProcessingFormat.getIsBigEndian(),
			!audioProcessingFormat.getIsInterleaved());

	OSStatus	status = ::AudioConverterNew(&sourceABSD, &destinationASBD, &mInternals->mAudioConverterRef);
	ReturnErrorIfFailed(status, CString(OSSTR("AudioConverterNew")));

	// Set magic cookie
	OV<SError>	error = setMagicCookie(mInternals->mAudioConverterRef);
	ReturnErrorIfError(error);

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
void CCoreAudioDecodeAudioCodec::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset audio converter
	::AudioConverterReset(mInternals->mAudioConverterRef);

	// Seek
	mInternals->mDecodeFramesToIgnore =
			mInternals->mMediaPacketSource->seekToDuration(
					(UInt32) (timeInterval * mInternals->mAudioProcessingFormat->getSampleRate()));
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CCoreAudioDecodeAudioCodec::decodeInto(CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf(audioFrames.getAllocatedFrameCount() < (1024 * 2));

	// Setup
	AudioBufferList	audioBufferList;
	audioBufferList.mNumberBuffers = 1;
	audioBufferList.mBuffers[0].mNumberChannels = mInternals->mAudioProcessingFormat->getChannelMap().getChannelCount();

	// Check if have frames to ignore
	OSStatus	status;
	if (mInternals->mDecodeFramesToIgnore > 0) {
		// Decode these frames, but throw away
		audioFrames.getAsWrite(audioBufferList);
		status =
				::AudioConverterFillComplexBuffer(mInternals->mAudioConverterRef, Internals::fillBufferData,
						mInternals, &mInternals->mDecodeFramesToIgnore, &audioBufferList, nil);
		if (status != noErr) return mInternals->mFillBufferDataError;
		if (mInternals->mDecodeFramesToIgnore == 0) return OV<SError>(SError::mEndOfData);

		// Done ignoring
		mInternals->mDecodeFramesToIgnore = 0;
	}

	// Fill buffer
	UInt32	frameCount = audioFrames.getAsWrite(audioBufferList);
	status =
			::AudioConverterFillComplexBuffer(mInternals->mAudioConverterRef, Internals::fillBufferData, mInternals,
					&frameCount, &audioBufferList, nil);
	if (status != noErr) return mInternals->mFillBufferDataError;
	if (frameCount == 0) return OV<SError>(SError::mEndOfData);

	// Update
	audioFrames.completeWrite(audioBufferList);

	return OV<SError>();
}
