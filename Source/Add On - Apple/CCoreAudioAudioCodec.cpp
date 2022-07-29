//----------------------------------------------------------------------------------------------------------------------
//	CCoreAudioAudioCodec.cpp			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CCoreAudioAudioCodec.h"

#include "SError-Apple.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreAudioDecodeAudioCodecInternals

class CCoreAudioDecodeAudioCodecInternals {
	public:
							CCoreAudioDecodeAudioCodecInternals(OSType codecID,
									const I<CMediaPacketSource>& mediaPacketSource) :
								mCodecID(codecID), mMediaPacketSource(mediaPacketSource),
										mAudioConverterRef(nil), mDecodeFramesToIgnore(0),
										mInputPacketData((CData::ByteCount) 10 * 1024),
										mInputPacketDescriptionsData((CData::ByteCount) 1 * 1024)
								{}
							~CCoreAudioDecodeAudioCodecInternals()
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
									CCoreAudioDecodeAudioCodecInternals&	internals =
																					*((CCoreAudioDecodeAudioCodecInternals*)
																							inUserData);

									// Read packets
									TIResult<TArray<SMediaPacket> >	mediaPacketsResult =
																			internals.mMediaPacketSource->readNextInto(
																					internals.mInputPacketData);
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
									const	TArray<SMediaPacket>&	mediaPackets = *mediaPacketsResult;

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

									ioBufferList->mNumberBuffers = 1;
									ioBufferList->mBuffers[0].mData = internals.mInputPacketData.getMutableBytePtr();
									ioBufferList->mBuffers[0].mDataByteSize = (UInt32) offset;

									return noErr;
								}

		OSType						mCodecID;
		I<CMediaPacketSource>		mMediaPacketSource;

		OI<SAudioProcessingFormat>	mAudioProcessingFormat;

		AudioConverterRef			mAudioConverterRef;
		UInt32						mDecodeFramesToIgnore;
		CData						mInputPacketData;
		CData						mInputPacketDescriptionsData;
		OI<SError>					mFillBufferDataError;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CCoreAudioDecodeAudioCodec

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CCoreAudioDecodeAudioCodec::CCoreAudioDecodeAudioCodec(OSType codecID, const I<CMediaPacketSource>& mediaPacketSource)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CCoreAudioDecodeAudioCodecInternals(codecID, mediaPacketSource);
}

//----------------------------------------------------------------------------------------------------------------------
CCoreAudioDecodeAudioCodec::~CCoreAudioDecodeAudioCodec()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CDecodeAudioCodec methods

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CCoreAudioDecodeAudioCodec::setup(const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mAudioProcessingFormat = OI<SAudioProcessingFormat>(audioProcessingFormat);

	// Create Audio Converter
	AudioStreamBasicDescription	sourceABSD = getSourceASBD(mInternals->mCodecID, audioProcessingFormat);

	AudioStreamBasicDescription	destinationASBD;
	FillOutASBDForLPCM(destinationASBD, audioProcessingFormat.getSampleRate(), audioProcessingFormat.getChannels(),
			audioProcessingFormat.getBits(), audioProcessingFormat.getBits(), audioProcessingFormat.getIsFloat(),
			audioProcessingFormat.getIsBigEndian(), !audioProcessingFormat.getIsInterleaved());

	OSStatus	status = ::AudioConverterNew(&sourceABSD, &destinationASBD, &mInternals->mAudioConverterRef);
	ReturnErrorIfFailed(status, OSSTR("AudioConverterNew"));

	// Set magic cookie
	OI<SError>	error = setMagicCookie(mInternals->mAudioConverterRef);
	ReturnErrorIfError(error);

	return OI<SError>();
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
OI<SError> CCoreAudioDecodeAudioCodec::decodeInto(CAudioFrames& audioFrames)
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
						CCoreAudioDecodeAudioCodecInternals::fillBufferData, mInternals,
						&mInternals->mDecodeFramesToIgnore, &audioBufferList, nil);
		if (status != noErr) return mInternals->mFillBufferDataError;
		if (mInternals->mDecodeFramesToIgnore == 0) return OI<SError>(SError::mEndOfData);

		// Done ignoring
		mInternals->mDecodeFramesToIgnore = 0;
	}

	// Fill buffer
	UInt32	frameCount = audioFrames.getAsWrite(audioBufferList);
	status =
			::AudioConverterFillComplexBuffer(mInternals->mAudioConverterRef,
					CCoreAudioDecodeAudioCodecInternals::fillBufferData, mInternals, &frameCount, &audioBufferList,
					nil);
	if (status != noErr) return mInternals->mFillBufferDataError;
	if (frameCount == 0) return OI<SError>(SError::mEndOfData);

	// Update
	audioFrames.completeWrite(audioBufferList);

	return OI<SError>();
}
