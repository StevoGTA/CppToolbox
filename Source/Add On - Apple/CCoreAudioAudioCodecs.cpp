//----------------------------------------------------------------------------------------------------------------------
//	CCoreAudioAudioCodecs.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAACAudioCodec.h"

#include "CByteReader.h"
#include "CLogServices.h"
#include "SError-Apple.h"

#include <AudioToolbox/AudioToolbox.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAACDecodeAudioCodec

class CAACDecodeAudioCodec : public CDecodeAudioCodec {
	public:
												// Lifecycle methods
												CAACDecodeAudioCodec(OSType codecID,
														const I<CMediaPacketSource>& mediaPacketSource,
														const CData& configurationData) :
													mCodecID(codecID),
															mDecodeInfo(mediaPacketSource, configurationData),
															mAudioConverterRef(nil), mDecodeFramesToIgnore(0),
															mInputPacketData((CData::ByteCount) 10 * 1024),
															mInputPacketDescriptionsData((CData::ByteCount) 1 * 1024)
													{}
												~CAACDecodeAudioCodec()
													{
														// Cleanup
														if (mAudioConverterRef != nil)
															::AudioConverterDispose(mAudioConverterRef);
													}

												// CAudioCodec methods - Decoding
				TArray<SAudioProcessingSetup>	getAudioProcessingSetups(const SAudioStorageFormat& audioStorageFormat);
				OI<SError>						setup(const SAudioProcessingFormat& audioProcessingFormat);
				CAudioFrames::Requirements		getRequirements() const
													{ return CAudioFrames::Requirements(1024, 1024 * 2); }
				void							seek(UniversalTimeInterval timeInterval);
				OI<SError>						decodeInto(CAudioFrames& audioFrames);

												// Class methods
		static	OSStatus						fillBufferData(AudioConverterRef inAudioConverter,
														UInt32* ioNumberDataPackets, AudioBufferList* ioBufferList,
														AudioStreamPacketDescription** outDataPacketDescription,
														void* inUserData);

	private:
		OSType						mCodecID;
		CAACAudioCodec::DecodeInfo	mDecodeInfo;

		OI<SAudioProcessingFormat>	mAudioProcessingFormat;

		AudioConverterRef			mAudioConverterRef;
		UInt32						mDecodeFramesToIgnore;
		CData						mInputPacketData;
		CData						mInputPacketDescriptionsData;
		OI<SError>					mFillBufferDataError;
};

// MARK: CAudioCodec methods - Decoding

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudioProcessingSetup> CAACDecodeAudioCodec::getAudioProcessingSetups(
		const SAudioStorageFormat& audioStorageFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudioProcessingSetup>(
			SAudioProcessingSetup(32, audioStorageFormat.getSampleRate(), audioStorageFormat.getChannelMap(),
					SAudioProcessingSetup::kSampleTypeFloat));
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CAACDecodeAudioCodec::setup(const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mAudioProcessingFormat = OI<SAudioProcessingFormat>(audioProcessingFormat);

	// Create Audio Converter
	AudioStreamBasicDescription	sourceFormat = {0};
	sourceFormat.mFormatID = (mCodecID == CAACAudioCodec::mAACLCID) ? kAudioFormatMPEG4AAC : kAudioFormatMPEG4AAC_LD;
	sourceFormat.mFormatFlags = 0;
	sourceFormat.mSampleRate = audioProcessingFormat.getSampleRate();
	sourceFormat.mChannelsPerFrame = audioProcessingFormat.getChannels();

	AudioStreamBasicDescription	destinationFormat;
	FillOutASBDForLPCM(destinationFormat, audioProcessingFormat.getSampleRate(), audioProcessingFormat.getChannels(),
			audioProcessingFormat.getBits(), audioProcessingFormat.getBits(), audioProcessingFormat.getIsFloat(),
			audioProcessingFormat.getIsBigEndian(), !audioProcessingFormat.getIsInterleaved());

	OSStatus	status = ::AudioConverterNew(&sourceFormat, &destinationFormat, &mAudioConverterRef);
	ReturnErrorIfFailed(status, OSSTR("AudioConverterNew"));

	// Set magic cookie
	status =
			::AudioConverterSetProperty(mAudioConverterRef, kAudioConverterDecompressionMagicCookie,
					(UInt32) mDecodeInfo.getMagicCookie().getByteCount(), mDecodeInfo.getMagicCookie().getBytePtr());
	ReturnErrorIfFailed(status, OSSTR("AudioConverterSetProperty for magic cookie"));

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
void CAACDecodeAudioCodec::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset audio converter
	::AudioConverterReset(mAudioConverterRef);

	// Seek
	mDecodeFramesToIgnore =
			mDecodeInfo.getMediaPacketSource()->seekToDuration(
					(UInt32) (timeInterval * mAudioProcessingFormat->getSampleRate()));
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CAACDecodeAudioCodec::decodeInto(CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf(audioFrames.getAvailableFrameCount() < (1024 * 2));

	// Setup
	AudioBufferList	audioBufferList;
	audioBufferList.mNumberBuffers = 1;
	audioBufferList.mBuffers[0].mNumberChannels = mAudioProcessingFormat->getChannels();

	// Check if have frames to ignore
	OSStatus	status;
	if (mDecodeFramesToIgnore > 0) {
		// Decode these frames, but throw away
		audioFrames.getAsWrite(audioBufferList);
		status =
				::AudioConverterFillComplexBuffer(mAudioConverterRef, fillBufferData, this, &mDecodeFramesToIgnore,
						&audioBufferList, nil);
		if (status != noErr) return mFillBufferDataError;
		if (mDecodeFramesToIgnore == 0) return OI<SError>(SError::mEndOfData);

		// Done ignoring
		mDecodeFramesToIgnore = 0;
	}

	// Fill buffer
	UInt32	frameCount = audioFrames.getAsWrite(audioBufferList);
	status =
			::AudioConverterFillComplexBuffer(mAudioConverterRef, fillBufferData, this, &frameCount, &audioBufferList,
					nil);
	if (status != noErr) return mFillBufferDataError;
	if (frameCount == 0) return OI<SError>(SError::mEndOfData);

	// Update
	audioFrames.completeWrite(audioBufferList);

	return OI<SError>();
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
OSStatus CAACDecodeAudioCodec::fillBufferData(AudioConverterRef inAudioConverter, UInt32* ioNumberDataPackets,
		AudioBufferList* ioBufferList, AudioStreamPacketDescription** outDataPacketDescription, void* inUserData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CAACDecodeAudioCodec&	audioCodec = *((CAACDecodeAudioCodec*) inUserData);

	// Read packets
	TIResult<TArray<SMediaPacket> >	mediaPacketsResult =
											audioCodec.mDecodeInfo.
													getMediaPacketSource()->readNextInto(audioCodec.mInputPacketData);
	if (mediaPacketsResult.hasError()) {
		// Check error
		if (mediaPacketsResult.getError() == SError::mEndOfData) {
			// End of data
			*ioNumberDataPackets = 0;

			return noErr;
		} else {
			// Other error
			audioCodec.mFillBufferDataError = mediaPacketsResult.getError();

			return -1;
		}
	}

	// Prepare return info
	const	TArray<SMediaPacket>&	mediaPackets = mediaPacketsResult.getValue();

	*ioNumberDataPackets = mediaPackets.getCount();

	ioBufferList->mNumberBuffers = 1;
	ioBufferList->mBuffers[0].mData = audioCodec.mInputPacketData.getMutableBytePtr();
	ioBufferList->mBuffers[0].mDataByteSize = (UInt32) audioCodec.mInputPacketData.getByteCount();

	if ((*ioNumberDataPackets * sizeof(AudioStreamPacketDescription)) >
			audioCodec.mInputPacketDescriptionsData.getByteCount())
		// Increase packet descriptions data size
		audioCodec.mInputPacketDescriptionsData.setByteCount(
				*ioNumberDataPackets * sizeof(AudioStreamPacketDescription));

	AudioStreamPacketDescription*	packetDescription =
											(AudioStreamPacketDescription*)
													audioCodec.mInputPacketDescriptionsData.getMutableBytePtr();
	*outDataPacketDescription = packetDescription;

	SInt64	offset = 0;
	for (TIteratorD<SMediaPacket> iterator = mediaPackets.getIterator(); iterator.hasValue(); iterator.advance(),
			packetDescription++) {
		// Update
		packetDescription->mStartOffset = offset;
		packetDescription->mVariableFramesInPacket = iterator->mDuration;
		packetDescription->mDataByteSize = iterator->mByteCount;

		offset += iterator->mByteCount;
	}

	return noErr;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAACAudioCodec

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
I<CDecodeAudioCodec> CAACAudioCodec::create(const SAudioStorageFormat& audioStorageFormat,
		const I<CSeekableDataSource>& seekableDataSource, const TArray<SMediaPacketAndLocation>& packetAndLocations,
		const CData& configurationData)
//----------------------------------------------------------------------------------------------------------------------
{
	return I<CDecodeAudioCodec>(
			new CAACDecodeAudioCodec(audioStorageFormat.getCodecID(),
					I<CMediaPacketSource>(
							new CSeekableVaryingMediaPacketSource(seekableDataSource, packetAndLocations)),
					configurationData));
}
