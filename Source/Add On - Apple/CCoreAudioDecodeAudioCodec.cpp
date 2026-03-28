//----------------------------------------------------------------------------------------------------------------------
//	CCoreAudioDecodeAudioCodec.cpp			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CCoreAudioDecodeAudioCodec.h"

#include "SError-Apple.h"

using MediaPacketsAndBuffer = CMediaPacketSource::MediaPacketsAndBuffer;

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreAudioDecodeAudioCodec::Internals

class CCoreAudioDecodeAudioCodec::Internals {
	public:
							Internals(OSType codecID, const I<CMediaPacketSource>& mediaPacketSource) :
								mCodecID(codecID), mMediaPacketSource(mediaPacketSource),
										mAudioConverterRef(nil), mDecodeFramesToIgnore(0)
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
									TVResult<MediaPacketsAndBuffer>	mediaPacketsAndBuffer =
																			internals
																					.mMediaPacketSource->
																							readNext(10 * 1024);
									if (mediaPacketsAndBuffer.hasError()) {
										// Check error
										if (mediaPacketsAndBuffer.getError() == SError::mEndOfData) {
											// End of data
											ioBufferList->mBuffers[0].mData = nil;
											ioBufferList->mBuffers[0].mDataByteSize = 0;

											*ioNumberDataPackets = 0;

											return noErr;
										} else {
											// Other error
											internals.mFillBufferDataError = mediaPacketsAndBuffer.getError();

											return -1;
										}
									}

									// Store data
									internals.mMediaPacketsBuffer.setValue(mediaPacketsAndBuffer->getBuffer());

									// Process packets
									const	TArray<SMedia::Packet>&	mediaPackets =
																			mediaPacketsAndBuffer->getMediaPackets();
									internals.mAudioStreamPacketDescriptions.setValue(
											TBuffer<AudioStreamPacketDescription>(mediaPackets.getCount()));

									SInt64	offset = 0;
									for (TArray<SMedia::Packet>::Iterator iterator = mediaPackets.getIterator();
											iterator; iterator++) {
										// Update
										AudioStreamPacketDescription&	audioStreamPacketDescription =
																				(*internals.mAudioStreamPacketDescriptions)
																						[iterator.getIndex()];
										audioStreamPacketDescription.mStartOffset = offset;
										audioStreamPacketDescription.mVariableFramesInPacket = iterator->getDuration();
										audioStreamPacketDescription.mDataByteSize = iterator->getByteCount();

										offset += iterator->getByteCount();
									}

									// Prepare return info
									*ioNumberDataPackets = mediaPackets.getCount();

									ioBufferList->mNumberBuffers = 1;
									ioBufferList->mBuffers[0].mData = **internals.mMediaPacketsBuffer;
									ioBufferList->mBuffers[0].mDataByteSize =
											(UInt32) internals.mMediaPacketsBuffer->getByteCount();

									if (outDataPacketDescription != nil)
										// Pass back
										*outDataPacketDescription = **internals.mAudioStreamPacketDescriptions;

									return noErr;
								}

		OSType										mCodecID;
		I<CMediaPacketSource>						mMediaPacketSource;

		OV<SAudio::ProcessingFormat>				mAudioProcessingFormat;

		AudioConverterRef							mAudioConverterRef;
		UInt32										mDecodeFramesToIgnore;
		OV<TBuffer<UInt8> >							mMediaPacketsBuffer;
		OV<TBuffer<AudioStreamPacketDescription> >	mAudioStreamPacketDescriptions;
		OV<SError>									mFillBufferDataError;
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
