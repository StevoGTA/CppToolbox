//----------------------------------------------------------------------------------------------------------------------
//	CAudioChannelMapper.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioChannelMapper.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioChannelMapper::Map

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CAudioChannelMapper::Map CAudioChannelMapper::Map::getMap(const SAudio::ChannelMap& fromAudioChannelMap,
		const SAudio::ChannelMap& toAudioChannelMap)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt8	inputChannelCount = fromAudioChannelMap.getChannelCount();
	UInt8	outputChannelCount = toAudioChannelMap.getChannelCount();
	AssertFailIf((outputChannelCount == 1) && (inputChannelCount > 1));

	// Check situation
	TNArray<OV<UInt8> >	inputChannelIndexes;
	if ((inputChannelCount == 1) && (outputChannelCount == 2))
		// Mono -> Stereo, duplicate the channel
		inputChannelIndexes += TNArray<OV<UInt8> >(OV<UInt8>(0), 2);
	else
		// Fewer -> More copies the channels and leaves the extra ones silent; More -> Fewer keeps the first channels
		for (UInt8 i = 0; i < outputChannelCount; i++)
			// Add channel
			inputChannelIndexes += (i < inputChannelCount) ? OV<UInt8>(i) : OV<UInt8>();

	return Map(inputChannelCount, inputChannelIndexes);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioChannelMapper::Map CAudioChannelMapper::Map::getMapForAddingChannels(UInt8 channelCount, UInt8 addedChannelCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Copy existing channels, added channels are silent
	TNArray<OV<UInt8> >	inputChannelIndexes;
	for (UInt8 i = 0; i < channelCount + addedChannelCount; i++)
		// Add channel
		inputChannelIndexes += (i < channelCount) ? OV<UInt8>(i) : OV<UInt8>();

	return Map(channelCount, inputChannelIndexes);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioChannelMapper::Map CAudioChannelMapper::Map::getMapForRemovingChannels(UInt8 channelCount,
		const CBits& channelsToRemove)
//----------------------------------------------------------------------------------------------------------------------
{
	// Copy kept channels in order
	TNArray<OV<UInt8> >	inputChannelIndexes;
	for (UInt8 i = 0; i < channelCount; i++) {
		// Check if kept
		if (!channelsToRemove.get(i))
			// Add channel
			inputChannelIndexes += OV<UInt8>(i);
	}

	return Map(channelCount, inputChannelIndexes);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioChannelMapper::Internals

class CAudioChannelMapper::Internals {
	public:
		typedef	void	(*CopySamplesProc)(const UInt8* sourcePtr, UInt32 sourceStride, UInt8* destinationPtr,
								UInt32 destinationStride, UInt32 frameCount);

												Internals(const Map& map) : mMap(map), mCopySamplesProc(nil) {}

		template <typename T>	static	void	copySamples(const UInt8* sourcePtr, UInt32 sourceStride,
														UInt8* destinationPtr, UInt32 destinationStride,
														UInt32 frameCount)
													{
														// Copy each sample
														for (UInt32 i = 0; i < frameCount;
																i++, sourcePtr += sourceStride,
																destinationPtr += destinationStride)
															// Copy sample
															*((T*) destinationPtr) = *((const T*) sourcePtr);
													}

		Map								mMap;
		OV<SAudio::ProcessingFormat>	mInputAudioProcessingFormat;
		OI<CAudioFrames>				mInputAudioFrames;
		CopySamplesProc					mCopySamplesProc;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioChannelMapper

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioChannelMapper::CAudioChannelMapper(const Map& map) : CBasicAudioProcessor()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(map);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioChannelMapper::~CAudioChannelMapper()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CAudioProcessor methods

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CAudioChannelMapper::connectInput(const I<CAudioProcessor>& audioProcessor,
		const SAudio::ProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mInputAudioProcessingFormat.setValue(audioProcessingFormat);

	// Setup
	switch (audioProcessingFormat.getBytesPerSample()) {
		case 1:
			// 1 byte per sample
			mInternals->mCopySamplesProc = Internals::copySamples<UInt8>;
			break;

		case 2:
			// 2 bytes per sample
			mInternals->mCopySamplesProc = Internals::copySamples<UInt16>;
			break;

		case 3:
			// 3 bytes per sample
			mInternals->mCopySamplesProc = Internals::copySamples<SAudio::Sample24Bits>;
			break;

		case 4:
			// 4 bytes per sample
			mInternals->mCopySamplesProc = Internals::copySamples<UInt32>;
			break;

		case 8:
			// 8 bytes per sample
			mInternals->mCopySamplesProc = Internals::copySamples<UInt64>;
			break;

		default:
			// Unsupported bits
			AssertFailUnimplemented();
	}

	return CAudioProcessor::connectInput(audioProcessor, audioProcessingFormat);
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CString> CAudioChannelMapper::getSetupDescription(const CString& indent)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get upstream setup descriptions
	TNArray<CString>	setupDescriptions(CBasicAudioProcessor::getSetupDescription(indent));

	// Add our setup description
	setupDescriptions +=
			indent + CString(OSSTR("Channel Mapper from ")) +
					CString::mDoubleQuote +
							mInternals->mInputAudioProcessingFormat->getChannelMap().getDisplayString() +
							CString::mDoubleQuote +
					CString(OSSTR(" to ")) +
					CString::mDoubleQuote +
							mOutputAudioProcessingFormat->getChannelMap().getDisplayString() +
							CString::mDoubleQuote;

	return setupDescriptions;
}

//----------------------------------------------------------------------------------------------------------------------
TNumberArray<Float32> CAudioChannelMapper::setChannelGains(const TNumberArray<Float32>& channelGains)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNumberArray<Float32>	channelGains_ = CAudioProcessor::setChannelGains(channelGains);

	// Each output channel has the gain of the input channel it takes
	TNumberArray<Float32>	outputChannelGains;
	for (UInt8 i = 0; i < mInternals->mMap.getOutputChannelCount(); i++) {
		// Setup
		const	OV<UInt8>&	inputChannelIndex = mInternals->mMap.getInputChannelIndex(i);

		// Add gain
		outputChannelGains +=
				(inputChannelIndex.hasValue() && (*inputChannelIndex < channelGains_.getCount())) ?
						channelGains_[*inputChannelIndex] : 1.0f;
	}

	return outputChannelGains;
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CAudioProcessor::SourceInfo> CAudioChannelMapper::performInto(CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	if (!mInternals->mInputAudioFrames.hasInstance() ||
			(mInternals->mInputAudioFrames->getAllocatedFrameCount() != audioFrames.getAllocatedFrameCount())) {
		// Create Audio Data
		if (mInternals->mInputAudioProcessingFormat->getIsInterleaved())
			// Interleaved
			mInternals->mInputAudioFrames =
					OI<CAudioFrames>(
							new CAudioFrames(mInternals->mInputAudioProcessingFormat->getBytesPerFrame(),
									audioFrames.getAllocatedFrameCount()));
		else
			// Non-interleaved
			mInternals->mInputAudioFrames =
					OI<CAudioFrames>(
							new CAudioFrames(mInternals->mInputAudioProcessingFormat->getChannelMap().getChannelCount(),
									mInternals->mInputAudioProcessingFormat->getBits() / 8,
									audioFrames.getAllocatedFrameCount()));
	} else
		// Reset existing Audio Data
		mInternals->mInputAudioFrames->reset();

	// Read
	TVResult<SourceInfo>	sourceInfo = CAudioProcessor::performInto(*mInternals->mInputAudioFrames);
	ReturnValueIfResultError(sourceInfo, sourceInfo);

	// Setup
	const	Map&				map = mInternals->mMap;
			CAudioFrames::Info	readInfo = mInternals->mInputAudioFrames->getReadInfo();
			CAudioFrames::Info	writeInfo = audioFrames.getWriteInfo();
			UInt32				frameCount = readInfo.getFrameCount();
			UInt32				bytesPerSample = mInternals->mInputAudioProcessingFormat->getBytesPerSample();

	// Check interleaved
	if (mInternals->mInputAudioProcessingFormat->getIsInterleaved()) {
		// Interleaved
		UInt32	inputBytesPerFrame = bytesPerSample * map.getInputChannelCount();
		UInt32	outputBytesPerFrame = bytesPerSample * map.getOutputChannelCount();
		for (UInt8 i = 0; i < map.getOutputChannelCount(); i++) {
			// Setup
			const	OV<UInt8>&	inputChannelIndex = map.getInputChannelIndex(i);
					UInt8*		destinationPtr = (UInt8*) writeInfo.getSegment(0) + i * bytesPerSample;

			// Check if have input channel
			if (inputChannelIndex.hasValue())
				// Copy samples
				mInternals->mCopySamplesProc(
						(const UInt8*) readInfo.getSegment(0) + *inputChannelIndex * bytesPerSample, inputBytesPerFrame,
						destinationPtr, outputBytesPerFrame, frameCount);
			else
				// Silence
				for (UInt32 frameIndex = 0; frameIndex < frameCount;
						frameIndex++, destinationPtr += outputBytesPerFrame)
					// Clear sample
					::memset(destinationPtr, 0, bytesPerSample);
		}
	} else {
		// Non-interleaved
		for (UInt8 i = 0; i < map.getOutputChannelCount(); i++) {
			// Check if have input channel
			const	OV<UInt8>&	inputChannelIndex = map.getInputChannelIndex(i);
			if (inputChannelIndex.hasValue())
				// Copy segment
				::memcpy(writeInfo.getSegment(i), readInfo.getSegment(*inputChannelIndex), frameCount * bytesPerSample);
			else
				// Silence
				::memset(writeInfo.getSegment(i), 0, frameCount * bytesPerSample);
		}
	}

	// Complete
	audioFrames.completeWrite(frameCount);

	return sourceInfo;
}

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudio::ProcessingSetup> CAudioChannelMapper::getInputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudio::ProcessingSetup>(
			SAudio::ProcessingSetup(SAudio::ProcessingSetup::BitsInfo(mOutputAudioProcessingFormat->getBits()),
					SAudio::ProcessingSetup::SampleRateInfo(mOutputAudioProcessingFormat->getSampleRate()),
					SAudio::ProcessingSetup::ChannelMapInfo::mUnspecified,
					SAudio::ProcessingSetup::SampleTypeOption(
							mOutputAudioProcessingFormat->getIsFloat() ?
									SAudio::ProcessingSetup::kSampleTypeFloat :
									SAudio::ProcessingSetup::kSampleTypeSignedInteger),
					SAudio::ProcessingSetup::EndianOption(
							mOutputAudioProcessingFormat->getIsBigEndian() ?
									SAudio::ProcessingSetup::kEndianBig : SAudio::ProcessingSetup::kEndianLittle),
					SAudio::ProcessingSetup::InterleavedOption(
							mOutputAudioProcessingFormat->getIsInterleaved() ?
									SAudio::ProcessingSetup::kInterleaved : SAudio::ProcessingSetup::kNonInterleaved)));
}

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudio::ProcessingSetup> CAudioChannelMapper::getOutputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	return TSArray<SAudio::ProcessingSetup>(SAudio::ProcessingSetup::mUnspecified);
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
bool CAudioChannelMapper::canPerform(const SAudio::ChannelMap& fromAudioChannelMap,
		const SAudio::ChannelMap& toAudioChannelMap)
//----------------------------------------------------------------------------------------------------------------------
{
	// Mapping is by position, so at least one channel map must have unspecified order
	return (fromAudioChannelMap.getChannelCount() > 0) && (toAudioChannelMap.getChannelCount() > 0) &&
			(fromAudioChannelMap.hasUnspecifiedOrder() || toAudioChannelMap.hasUnspecifiedOrder()) &&
			!((toAudioChannelMap.getChannelCount() == 1) && (fromAudioChannelMap.getChannelCount() > 1));
}
