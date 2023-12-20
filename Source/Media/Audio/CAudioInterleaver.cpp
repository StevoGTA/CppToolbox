//----------------------------------------------------------------------------------------------------------------------
//	CAudioInterleaver.cpp			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioInterleaver.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioInterleaver::Internals

class CAudioInterleaver::Internals {
	public:
		typedef	void	(*PerformProc)(const TNumberArray<void*> readAudioFramesSegments,
								const TNumberArray<void*> writeAudioFramesSegments, UInt32 frameCount);

						Internals() : mPerformProc(nil) {}

		static	void	perform8BytesPerFrame(const TNumberArray<void*> readAudioFramesSegments,
								const TNumberArray<void*> writeAudioFramesSegments, UInt32 frameCount)
							{
								// Setup
								CArray::ItemCount	segmentCount = readAudioFramesSegments.getCount();

								// Iterate read audio frames segments
								for (CArray::ItemIndex i = 0; i < segmentCount; i++) {
									// Setup
									const	UInt64*	sourcePtr = (const UInt64*) readAudioFramesSegments[i];
											UInt64*	destinationPtr = ((UInt64*) writeAudioFramesSegments[0]) + i;

									// Iterate frames
									for (UInt32 j = 0; j < frameCount; j++, sourcePtr++, destinationPtr += segmentCount)
										// Copy sample
										*destinationPtr = *sourcePtr;
								}
							}
		static	void	perform4BytesPerFrame(const TNumberArray<void*> readAudioFramesSegments,
								const TNumberArray<void*> writeAudioFramesSegments, UInt32 frameCount)
							{
								// Setup
								CArray::ItemCount	segmentCount = readAudioFramesSegments.getCount();

								// Iterate read audio frames segments
								for (CArray::ItemIndex i = 0; i < segmentCount; i++) {
									// Setup
									const	UInt32*	sourcePtr = (const UInt32*) readAudioFramesSegments[i];
											UInt32*	destinationPtr = ((UInt32*) writeAudioFramesSegments[0]) + i;

									// Iterate frames
									for (UInt32 j = 0; j < frameCount; j++, sourcePtr++, destinationPtr += segmentCount)
										// Copy sample
										*destinationPtr = *sourcePtr;
								}
							}
		static	void	perform3BytesPerFrame(const TNumberArray<void*> readAudioFramesSegments,
								const TNumberArray<void*> writeAudioFramesSegments, UInt32 frameCount)
							{
								// Setup
								CArray::ItemCount	segmentCount = readAudioFramesSegments.getCount();

								// Iterate read audio frames segments
								for (CArray::ItemIndex i = 0; i < segmentCount; i++) {
									// Setup
									const	UInt8*	sourcePtr = (const UInt8*) readAudioFramesSegments[i];
											UInt8*	destinationPtr = ((UInt8*) writeAudioFramesSegments[0]) + i * 3;

									// Iterate frames
									for (UInt32 j = 0; j < frameCount; j++, destinationPtr += (segmentCount - 1) * 3) {
										// Copy sample
										*(destinationPtr++) = *(sourcePtr++);
										*(destinationPtr++) = *(sourcePtr++);
										*(destinationPtr++) = *(sourcePtr++);
									}
								}
							}
		static	void	perform2BytesPerFrame(const TNumberArray<void*> readAudioFramesSegments,
								const TNumberArray<void*> writeAudioFramesSegments, UInt32 frameCount)
							{
								// Setup
								CArray::ItemCount	segmentCount = readAudioFramesSegments.getCount();

								// Iterate read audio frames segments
								for (CArray::ItemIndex i = 0; i < segmentCount; i++) {
									// Setup
									const	UInt16*	sourcePtr = (const UInt16*) readAudioFramesSegments[i];
											UInt16*	destinationPtr = ((UInt16*) writeAudioFramesSegments[0]) + i;

									// Iterate frames
									for (UInt32 j = 0; j < frameCount; j++, sourcePtr++, destinationPtr += segmentCount)
										// Copy sample
										*destinationPtr = *sourcePtr;
								}
							}
		static	void	perform1BytePerFrame(const TNumberArray<void*> readAudioFramesSegments,
								const TNumberArray<void*> writeAudioFramesSegments, UInt32 frameCount)
							{
								// Setup
								CArray::ItemCount	segmentCount = readAudioFramesSegments.getCount();

								// Iterate read audio frames segments
								for (CArray::ItemIndex i = 0; i < segmentCount; i++) {
									// Setup
									const	UInt8*	sourcePtr = (const UInt8*) readAudioFramesSegments[i];
											UInt8*	destinationPtr = ((UInt8*) writeAudioFramesSegments[0]) + i;

									// Iterate frames
									for (UInt32 j = 0; j < frameCount; j++, sourcePtr++, destinationPtr += segmentCount)
										// Copy sample
										*destinationPtr = *sourcePtr;
								}
							}

		OV<SAudio::ProcessingFormat>	mInputAudioProcessingFormat;
		OI<CAudioFrames>				mInputAudioFrames;
		PerformProc						mPerformProc;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioInterleaver

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioInterleaver::CAudioInterleaver() : CBasicAudioProcessor()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals();
}

//----------------------------------------------------------------------------------------------------------------------
CAudioInterleaver::~CAudioInterleaver()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CAudioProcessor methods

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CAudioInterleaver::connectInput(const I<CAudioProcessor>& audioProcessor,
		const SAudio::ProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mInputAudioProcessingFormat.setValue(audioProcessingFormat);

	// Setup
	switch (mInternals->mInputAudioProcessingFormat->getBits()) {
		case 64:	mInternals->mPerformProc = Internals::perform8BytesPerFrame;	break;
		case 32:	mInternals->mPerformProc = Internals::perform4BytesPerFrame;	break;
		case 24:	mInternals->mPerformProc = Internals::perform3BytesPerFrame;	break;
		case 16:	mInternals->mPerformProc = Internals::perform2BytesPerFrame;	break;
		default:	mInternals->mPerformProc = Internals::perform1BytePerFrame;	break;
	}

	// Do super
	return CAudioProcessor::connectInput(audioProcessor, audioProcessingFormat);
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CString> CAudioInterleaver::getSetupDescription(const CString& indent)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get upstream setup descriptions
	TNArray<CString>	setupDescriptions(CBasicAudioProcessor::getSetupDescription(indent));

	// Add our setup description
	setupDescriptions += indent + CString(OSSTR("Interleaver"));

	return setupDescriptions;
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CAudioProcessor::SourceInfo> CAudioInterleaver::performInto(CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	if (!mInternals->mInputAudioFrames.hasInstance() ||
			(mInternals->mInputAudioFrames->getAllocatedFrameCount() != audioFrames.getAllocatedFrameCount()))
		// Create Audio Data
		mInternals->mInputAudioFrames =
				OI<CAudioFrames>(
						new CAudioFrames(mInternals->mInputAudioProcessingFormat->getChannelMap().getChannelCount(),
								mInternals->mInputAudioProcessingFormat->getBits() / 8,
								audioFrames.getAllocatedFrameCount()));
	else
		// Reset existing Audio Data
		mInternals->mInputAudioFrames->reset();

	// Read
	TVResult<SourceInfo>	sourceInfo = CAudioProcessor::performInto(*mInternals->mInputAudioFrames);
	ReturnValueIfResultError(sourceInfo, sourceInfo);

	// Perform
	CAudioFrames::Info	readAudioFramesInfo = mInternals->mInputAudioFrames->getReadInfo();
	CAudioFrames::Info	writeAudioFramesInfo = audioFrames.getWriteInfo();
	mInternals->mPerformProc(readAudioFramesInfo.getSegments(), writeAudioFramesInfo.getSegments(),
			mInternals->mInputAudioFrames->getCurrentFrameCount());
	audioFrames.completeWrite(mInternals->mInputAudioFrames->getCurrentFrameCount());

	return sourceInfo;
}

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudio::ProcessingSetup> CAudioInterleaver::getInputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudio::ProcessingSetup>(
			SAudio::ProcessingSetup(SAudio::ProcessingSetup::BitsInfo(mOutputAudioProcessingFormat->getBits()),
					SAudio::ProcessingSetup::SampleRateInfo(mOutputAudioProcessingFormat->getSampleRate()),
					SAudio::ProcessingSetup::ChannelMapInfo(mOutputAudioProcessingFormat->getChannelMap()),
					SAudio::ProcessingSetup::SampleTypeOption(
							mOutputAudioProcessingFormat->getIsFloat() ?
									SAudio::ProcessingSetup::kSampleTypeFloat :
									SAudio::ProcessingSetup::kSampleTypeSignedInteger),
					SAudio::ProcessingSetup::EndianOption(
							mOutputAudioProcessingFormat->getIsBigEndian() ?
									SAudio::ProcessingSetup::kEndianBig : SAudio::ProcessingSetup::kEndianLittle),
					SAudio::ProcessingSetup::InterleavedOption(SAudio::ProcessingSetup::kNonInterleaved)));
}

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudio::ProcessingSetup> CAudioInterleaver::getOutputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudio::ProcessingSetup>(
			SAudio::ProcessingSetup(SAudio::ProcessingSetup::BitsInfo::mUnspecified,
					SAudio::ProcessingSetup::SampleRateInfo::mUnspecified,
					SAudio::ProcessingSetup::ChannelMapInfo::mUnspecified,
					SAudio::ProcessingSetup::SampleTypeOption::kSampleTypeUnspecified,
					SAudio::ProcessingSetup::EndianOption::kEndianUnspecified,
					SAudio::ProcessingSetup::InterleavedOption::kInterleaved));
}

