//----------------------------------------------------------------------------------------------------------------------
//	CAudioInterleaver.cpp			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioInterleaver.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioInterleaverInternals

class CAudioInterleaverInternals {
	public:
		typedef	void	(*PerformProc)(const TNumberArray<void*> readAudioFramesSegments,
								const TNumberArray<void*> writeAudioFramesSegments, UInt32 frameCount);

						CAudioInterleaverInternals() : mPerformProc(nil) {}

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

		OV<SAudioProcessingFormat>	mInputAudioProcessingFormat;
		OI<CAudioFrames>			mInputAudioFrames;
		PerformProc					mPerformProc;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioInterleaver

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioInterleaver::CAudioInterleaver() : CBasicAudioProcessor()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CAudioInterleaverInternals();
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
		const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mInputAudioProcessingFormat = OV<SAudioProcessingFormat>(audioProcessingFormat);

	// Setup
	switch (mInternals->mInputAudioProcessingFormat->getBits()) {
		case 64:	mInternals->mPerformProc = CAudioInterleaverInternals::perform8BytesPerFrame;	break;
		case 32:	mInternals->mPerformProc = CAudioInterleaverInternals::perform4BytesPerFrame;	break;
		case 24:	mInternals->mPerformProc = CAudioInterleaverInternals::perform3BytesPerFrame;	break;
		case 16:	mInternals->mPerformProc = CAudioInterleaverInternals::perform2BytesPerFrame;	break;
		default:	mInternals->mPerformProc = CAudioInterleaverInternals::perform1BytePerFrame;	break;
	}

	// Do super
	return CAudioProcessor::connectInput(audioProcessor, audioProcessingFormat);
}

//----------------------------------------------------------------------------------------------------------------------
TNArray<CString> CAudioInterleaver::getSetupDescription(const CString& indent)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get upstream setup descriptions
	TNArray<CString>	setupDescriptions = CBasicAudioProcessor::getSetupDescription(indent);

	// Add our setup description
	setupDescriptions += indent + CString(OSSTR("Interleaver"));

	return setupDescriptions;
}

//----------------------------------------------------------------------------------------------------------------------
SAudioSourceStatus CAudioInterleaver::performInto(CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	if (!mInternals->mInputAudioFrames.hasInstance() ||
			(mInternals->mInputAudioFrames->getAllocatedFrameCount() != audioFrames.getAllocatedFrameCount()))
		// Create Audio Data
		mInternals->mInputAudioFrames =
				OI<CAudioFrames>(
						new CAudioFrames(mInternals->mInputAudioProcessingFormat->getChannels(),
								mInternals->mInputAudioProcessingFormat->getBits() / 8,
								audioFrames.getAllocatedFrameCount()));
	else
		// Reset existing Audio Data
		mInternals->mInputAudioFrames->reset();

	// Read
	SAudioSourceStatus	audioSourceStatus = CAudioProcessor::performInto(*mInternals->mInputAudioFrames);
	if (!audioSourceStatus.isSuccess())
		// Error
		return audioSourceStatus;

	// Perform
	CAudioFrames::Info	readAudioFramesInfo = mInternals->mInputAudioFrames->getReadInfo();
	CAudioFrames::Info	writeAudioFramesInfo = audioFrames.getWriteInfo();
	mInternals->mPerformProc(readAudioFramesInfo.getSegments(), writeAudioFramesInfo.getSegments(),
			mInternals->mInputAudioFrames->getCurrentFrameCount());
	audioFrames.completeWrite(mInternals->mInputAudioFrames->getCurrentFrameCount());

	return audioSourceStatus;
}

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudioProcessingSetup> CAudioInterleaver::getInputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudioProcessingSetup>(
			SAudioProcessingSetup(SAudioProcessingSetup::BitsInfo(mOutputAudioProcessingFormat->getBits()),
					SAudioProcessingSetup::SampleRateInfo(mOutputAudioProcessingFormat->getSampleRate()),
					SAudioProcessingSetup::ChannelMapInfo(mOutputAudioProcessingFormat->getAudioChannelMap()),
					SAudioProcessingSetup::SampleTypeOption(
							mOutputAudioProcessingFormat->getIsFloat() ?
									SAudioProcessingSetup::kSampleTypeFloat :
									SAudioProcessingSetup::kSampleTypeSignedInteger),
					SAudioProcessingSetup::EndianOption(
							mOutputAudioProcessingFormat->getIsBigEndian() ?
									SAudioProcessingSetup::kEndianBig : SAudioProcessingSetup::kEndianLittle),
					SAudioProcessingSetup::InterleavedOption(SAudioProcessingSetup::kNonInterleaved)));
}

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudioProcessingSetup> CAudioInterleaver::getOutputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudioProcessingSetup>(
			SAudioProcessingSetup(SAudioProcessingSetup::BitsInfo::mUnspecified,
					SAudioProcessingSetup::SampleRateInfo::mUnspecified,
					SAudioProcessingSetup::ChannelMapInfo::mUnspecified,
					SAudioProcessingSetup::SampleTypeOption::kSampleTypeUnspecified,
					SAudioProcessingSetup::EndianOption::kEndianUnspecified,
					SAudioProcessingSetup::InterleavedOption::kInterleaved));
}

