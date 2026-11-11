//----------------------------------------------------------------------------------------------------------------------
//	CAudioChannelMixer.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioChannelMixer.h"

#include "ConcurrencyPrimitives.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioChannelMixer::Matrix

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CAudioChannelMixer::Matrix CAudioChannelMixer::Matrix::getMatrixForMono(UInt8 inputChannelCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Sum every channel
	TBuffer<Float32>	gains(inputChannelCount);
	for (UInt8 i = 0; i < inputChannelCount; i++)
		// Add channel
		gains[i] = 1.0;

	return Matrix(inputChannelCount, 1, gains);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioChannelMixer::Internals

class CAudioChannelMixer::Internals {
	public:
		Internals(const Matrix& matrix) : mMatrix(matrix) {}

		Matrix							mMatrix;
		OV<SAudio::ProcessingFormat>	mInputAudioProcessingFormat;
		OI<CAudioFrames>				mInputAudioFrames;

		CReadPreferringLock				mChannelGainsLock;
		TNumberArray<Float32>			mChannelGains;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioChannelMixer

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioChannelMixer::CAudioChannelMixer(const Matrix& matrix) : CBasicAudioProcessor()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(matrix);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioChannelMixer::~CAudioChannelMixer()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CAudioProcessor methods

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CAudioChannelMixer::connectInput(const I<CAudioProcessor>& audioProcessor,
		const SAudio::ProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mInputAudioProcessingFormat.setValue(audioProcessingFormat);

	return CAudioProcessor::connectInput(audioProcessor, audioProcessingFormat);
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CString> CAudioChannelMixer::getSetupDescription(const CString& indent)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get upstream setup descriptions
	TNArray<CString>	setupDescriptions(CBasicAudioProcessor::getSetupDescription(indent));

	// Add our setup description
	setupDescriptions +=
			indent + CString(OSSTR("Channel Mixer from ")) +
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
TNumberArray<Float32> CAudioChannelMixer::setChannelGains(const TNumberArray<Float32>& channelGains)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNumberArray<Float32>	channelGains_ = CAudioProcessor::setChannelGains(channelGains);

	// Store
	mInternals->mChannelGainsLock.lockForWriting();
	mInternals->mChannelGains = channelGains_;
	mInternals->mChannelGainsLock.unlockForWriting();

	return TNumberArray<Float32>();
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CAudioProcessor::SourceInfo> CAudioChannelMixer::performInto(CAudioFrames& audioFrames)
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
									sizeof(Float32), audioFrames.getAllocatedFrameCount()));
	} else
		// Reset existing Audio Data
		mInternals->mInputAudioFrames->reset();

	// Read
	TVResult<SourceInfo>	sourceInfo = CAudioProcessor::performInto(*mInternals->mInputAudioFrames);
	ReturnValueIfResultError(sourceInfo, sourceInfo);

	// Setup
	const	Matrix&				matrix = mInternals->mMatrix;
			CAudioFrames::Info	readInfo = mInternals->mInputAudioFrames->getReadInfo();
			CAudioFrames::Info	writeInfo = audioFrames.getWriteInfo();
			UInt32				frameCount = readInfo.getFrameCount();
			bool				isInputInterleaved = mInternals->mInputAudioProcessingFormat->getIsInterleaved();
			bool				isOutputInterleaved = mOutputAudioProcessingFormat->getIsInterleaved();
			UInt32				inputStride = isInputInterleaved ? matrix.getInputChannelCount() : 1;
			UInt32				outputStride = isOutputInterleaved ? matrix.getOutputChannelCount() : 1;

	// Setup channel gains
	TBuffer<Float32>	channelGains(matrix.getInputChannelCount());
	mInternals->mChannelGainsLock.lockForReading();
	for (UInt8 i = 0; i < matrix.getInputChannelCount(); i++)
		// Store gain
		channelGains[i] = (i < mInternals->mChannelGains.getCount()) ? mInternals->mChannelGains[i] : 1.0f;
	mInternals->mChannelGainsLock.unlockForReading();

	// Start from silence
	audioFrames.clear();

	// Iterate output channels
	for (UInt8 outputChannelIndex = 0; outputChannelIndex < matrix.getOutputChannelCount(); outputChannelIndex++) {
		// Setup
		Float32*	destinationPtr =
							isOutputInterleaved ?
									(Float32*) writeInfo.getSegment(0) + outputChannelIndex :
									(Float32*) writeInfo.getSegment(outputChannelIndex);

		// Add each input channel that has a gain
		for (UInt8 inputChannelIndex = 0; inputChannelIndex < matrix.getInputChannelCount(); inputChannelIndex++) {
			// Check gain
			Float32	gain = matrix.getGain(outputChannelIndex, inputChannelIndex) * channelGains[inputChannelIndex];
			if (gain == 0.0)
				// Skip
				continue;

			// Add
			const	Float32*	sourcePtr =
										isInputInterleaved ?
												(const Float32*) readInfo.getSegment(0) + inputChannelIndex :
												(const Float32*) readInfo.getSegment(inputChannelIndex);
			for (UInt32 frameIndex = 0; frameIndex < frameCount; frameIndex++)
				// Add sample
				destinationPtr[frameIndex * outputStride] += gain * sourcePtr[frameIndex * inputStride];
		}
	}

	// Complete
	audioFrames.completeWrite(frameCount);

	return sourceInfo;
}

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudio::ProcessingSetup> CAudioChannelMixer::getInputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudio::ProcessingSetup>(
			SAudio::ProcessingSetup(SAudio::ProcessingSetup::BitsInfo(32),
					SAudio::ProcessingSetup::SampleRateInfo(mOutputAudioProcessingFormat->getSampleRate()),
					SAudio::ProcessingSetup::ChannelMapInfo::mUnspecified, SAudio::ProcessingSetup::kSampleTypeFloat,
					SAudio::ProcessingSetup::kEndianNative, SAudio::ProcessingSetup::kInterleavedUnspecified));
}

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudio::ProcessingSetup> CAudioChannelMixer::getOutputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudio::ProcessingSetup>(
			SAudio::ProcessingSetup(SAudio::ProcessingSetup::BitsInfo(32),
					SAudio::ProcessingSetup::SampleRateInfo::mUnspecified,
					SAudio::ProcessingSetup::ChannelMapInfo::mUnspecified, SAudio::ProcessingSetup::kSampleTypeFloat,
					SAudio::ProcessingSetup::kEndianNative, SAudio::ProcessingSetup::kInterleavedUnspecified));
}
