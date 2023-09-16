//----------------------------------------------------------------------------------------------------------------------
//	CAudioDeinterleaver.cpp			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioDeinterleaver.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioDeinterleaver::Internals

class CAudioDeinterleaver::Internals {
	public:
		typedef	void	(*PerformProc)(const TNumberArray<void*> readAudioFramesSegments,
								const TNumberArray<void*> writeAudioFramesSegments, UInt32 frameCount);

						Internals() : mPerformProc(nil) {}

		static	void	perform8BytesPerFrame(const TNumberArray<void*> readAudioFramesSegments,
								const TNumberArray<void*> writeAudioFramesSegments, UInt32 frameCount)
							{
								// Setup
								CArray::ItemCount	segmentCount = writeAudioFramesSegments.getCount();

								// Iterate read audio frames segments
								for (CArray::ItemIndex i = 0; i < segmentCount; i++) {
									// Setup
									const	UInt64*	sourcePtr = ((const UInt64*) readAudioFramesSegments[0]) + i;
											UInt64*	destinationPtr = (UInt64*) writeAudioFramesSegments[i];

									// Iterate frames
									for (UInt32 j = 0; j < frameCount; j++, sourcePtr += segmentCount, destinationPtr++)
										// Copy sample
										*destinationPtr = *sourcePtr;
								}
							}
		static	void	perform4BytesPerFrame(const TNumberArray<void*> readAudioFramesSegments,
								const TNumberArray<void*> writeAudioFramesSegments, UInt32 frameCount)
							{
								// Setup
								CArray::ItemCount	segmentCount = writeAudioFramesSegments.getCount();

								// Iterate read audio frames segments
								for (CArray::ItemIndex i = 0; i < segmentCount; i++) {
									// Setup
									const	UInt32*	sourcePtr = ((const UInt32*) readAudioFramesSegments[0]) + i;
											UInt32*	destinationPtr = (UInt32*) writeAudioFramesSegments[i];

									// Iterate frames
									for (UInt32 j = 0; j < frameCount; j++, sourcePtr += segmentCount, destinationPtr++)
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
									const	UInt8*	sourcePtr = ((const UInt8*) readAudioFramesSegments[0]) + i * 3;
											UInt8*	destinationPtr = (UInt8*) writeAudioFramesSegments[i];

									// Iterate frames
									for (UInt32 j = 0; j < frameCount; j++, sourcePtr += (segmentCount - 1) * 3) {
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
								CArray::ItemCount	segmentCount = writeAudioFramesSegments.getCount();

								// Iterate read audio frames segments
								for (CArray::ItemIndex i = 0; i < segmentCount; i++) {
									// Setup
									const	UInt16*	sourcePtr = ((const UInt16*) readAudioFramesSegments[0]) + i;
											UInt16*	destinationPtr = (UInt16*) writeAudioFramesSegments[i];

									// Iterate frames
									for (UInt32 j = 0; j < frameCount; j++, sourcePtr += segmentCount, destinationPtr++)
										// Copy sample
										*destinationPtr = *sourcePtr;
								}
							}
		static	void	perform1BytePerFrame(const TNumberArray<void*> readAudioFramesSegments,
								const TNumberArray<void*> writeAudioFramesSegments, UInt32 frameCount)
							{
								// Setup
								CArray::ItemCount	segmentCount = writeAudioFramesSegments.getCount();

								// Iterate read audio frames segments
								for (CArray::ItemIndex i = 0; i < segmentCount; i++) {
									// Setup
									const	UInt8*	sourcePtr = ((const UInt8*) readAudioFramesSegments[0]) + i;
											UInt8*	destinationPtr = (UInt8*) writeAudioFramesSegments[i];

									// Iterate frames
									for (UInt32 j = 0; j < frameCount; j++, sourcePtr += segmentCount, destinationPtr++)
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
// MARK: - CAudioDeinterleaver

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioDeinterleaver::CAudioDeinterleaver() : CBasicAudioProcessor()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals();
}

//----------------------------------------------------------------------------------------------------------------------
CAudioDeinterleaver::~CAudioDeinterleaver()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CAudioProcessor methods

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CAudioDeinterleaver::connectInput(const I<CAudioProcessor>& audioProcessor,
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
TArray<CString> CAudioDeinterleaver::getSetupDescription(const CString& indent)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get upstream setup descriptions
	TNArray<CString>	setupDescriptions(CBasicAudioProcessor::getSetupDescription(indent));

	// Add our setup description
	setupDescriptions += indent + CString(OSSTR("Deinterleaver"));

	return setupDescriptions;
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<SMedia::SourceInfo> CAudioDeinterleaver::performInto(CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	if (!mInternals->mInputAudioFrames.hasInstance() ||
			(mInternals->mInputAudioFrames->getAllocatedFrameCount() != audioFrames.getAllocatedFrameCount()))
		// Create Audio Data
		mInternals->mInputAudioFrames =
				OI<CAudioFrames>(
						new CAudioFrames(1, mInternals->mInputAudioProcessingFormat->getBytesPerFrame(),
								audioFrames.getAllocatedFrameCount()));
	else
		// Reset existing Audio Data
		mInternals->mInputAudioFrames->reset();

	// Read
	TVResult<SMedia::SourceInfo>	mediaSourceInfo = CAudioProcessor::performInto(*mInternals->mInputAudioFrames);
	if (mediaSourceInfo.hasError())
		// Error
		return mediaSourceInfo;

	// Perform
	CAudioFrames::Info	readAudioFramesInfo = mInternals->mInputAudioFrames->getReadInfo();
	CAudioFrames::Info	writeAudioFramesInfo = audioFrames.getWriteInfo();
	mInternals->mPerformProc(readAudioFramesInfo.getSegments(), writeAudioFramesInfo.getSegments(),
			mInternals->mInputAudioFrames->getCurrentFrameCount());
	audioFrames.completeWrite(mInternals->mInputAudioFrames->getCurrentFrameCount());

	return mediaSourceInfo;
}

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudio::ProcessingSetup> CAudioDeinterleaver::getInputSetups() const
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
					SAudio::ProcessingSetup::InterleavedOption(SAudio::ProcessingSetup::kInterleaved)));
}

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudio::ProcessingSetup> CAudioDeinterleaver::getOutputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudio::ProcessingSetup>(
			SAudio::ProcessingSetup(SAudio::ProcessingSetup::BitsInfo::mUnspecified,
					SAudio::ProcessingSetup::SampleRateInfo::mUnspecified,
					SAudio::ProcessingSetup::ChannelMapInfo::mUnspecified,
					SAudio::ProcessingSetup::SampleTypeOption::kSampleTypeUnspecified,
					SAudio::ProcessingSetup::EndianOption::kEndianUnspecified,
					SAudio::ProcessingSetup::InterleavedOption::kNonInterleaved));
}

