//----------------------------------------------------------------------------------------------------------------------
//	CAudioPlayer-Apple.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioPlayer.h"

#include "CBits.h"
#include "SError-Apple.h"

#import <AudioToolbox/AudioToolbox.h>

/*
	References:
		https://github.com/thestk/rtaudio
*/

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioEngine

class CAudioEngine {
	public:
											CAudioEngine() :
												mOutputAudioUnit(nil), mMixerAudioUnit(nil), mASBD(),
														mMaxOutputFrames(0), mMaxAudioPlayers(8)
												{}

				AudioStreamBasicDescription	getInputFormat();

				UInt32						getMaxOutputFrames();

				void						setMaxAudioPlayers(UInt32 maxAudioPlayers);
				OV<UInt32>					addAudioPlayer(AURenderCallback inputProc, void* userData);
				void						setAudioPlayerGain(UInt32 index, Float32 gain);
				void						play();
				void						removeAudioPlayer(UInt32 index);

		static	CAudioEngine				mShared;

				AudioComponentInstance		mOutputAudioUnit;
				AudioComponentInstance		mMixerAudioUnit;
				AudioStreamBasicDescription	mASBD;

				UInt32						mMaxOutputFrames;
				UInt32						mMaxAudioPlayers;
				CBits						mConnectedAudioPlayers;
};

// MARK: Properties

CAudioEngine	CAudioEngine::mShared;

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
AudioStreamBasicDescription CAudioEngine::getInputFormat()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	OSStatus	status;

	// Check if have already set things up
	if (mOutputAudioUnit == nil) {
		// Setup
		AudioComponentDescription	audioComponentDescription = {0};

		// Setup Output Audio Unit
		audioComponentDescription.componentType = kAudioUnitType_Output;
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
		audioComponentDescription.componentSubType = kAudioUnitSubType_RemoteIO;
#else
		audioComponentDescription.componentSubType = kAudioUnitSubType_DefaultOutput;
#endif
		AudioComponent	audioComponent = ::AudioComponentFindNext(nil, &audioComponentDescription);

		status = ::AudioComponentInstanceNew(audioComponent, &mOutputAudioUnit);
		LogOSStatusIfFailed(status, OSSTR("AudioComponentInstanceNew(OutputAudioUnit)"));

		status = ::AudioUnitInitialize(mOutputAudioUnit);
		LogOSStatusIfFailed(status, OSSTR("AudioUnitInitialize(OutputAudioUnit)"));

		AudioStreamBasicDescription	asbd;
		UInt32						size = sizeof(AudioStreamBasicDescription);
		status =
				::AudioUnitGetProperty(mOutputAudioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0,
						&asbd, &size);
		LogOSStatusIfFailed(status, OSSTR("AudioUnitGetProperty(OutputAudioUnit, StreamFormat)"));
		Float32	outputAudioUnitSampleRate = asbd.mSampleRate;

		// Setup Mixer Audio Unit
		audioComponentDescription.componentType = kAudioUnitType_Mixer;
		audioComponentDescription.componentSubType =
				kAudioUnitSubType_MultiChannelMixer;
		audioComponent = ::AudioComponentFindNext(nil, &audioComponentDescription);

		status = ::AudioComponentInstanceNew(audioComponent, &mMixerAudioUnit);
		LogOSStatusIfFailed(status, OSSTR("AudioComponentInstanceNew(MixerAudioUnit)"));

		status = ::AudioUnitInitialize(mMixerAudioUnit);
		LogOSStatusIfFailed(status, OSSTR("AudioUnitInitialize(MixerAudioUnit)"));

		size = sizeof(UInt32);
		status =
				::AudioUnitGetProperty(mMixerAudioUnit, kAudioUnitProperty_MaximumFramesPerSlice,
						kAudioUnitScope_Global, 0, &mMaxOutputFrames, &size);
		LogOSStatusIfFailed(status, OSSTR("AudioUnitGetProperty(MixerAudioUnit, MaximumFramesPerSlice)"));

		size = sizeof(UInt32);
		status =
				::AudioUnitSetProperty(mMixerAudioUnit, kAudioUnitProperty_ElementCount, kAudioUnitScope_Input, 0,
						&mMaxAudioPlayers, size);
		LogOSStatusIfFailed(status, OSSTR("AudioUnitSetProperty(MixerAudioUnit, ElementCount)"));

		status =
				::AudioUnitSetParameter(mMixerAudioUnit, kMultiChannelMixerParam_Volume, kAudioUnitScope_Output, 0, 1.0,
						0);
		LogOSStatusIfFailed(status, OSSTR("AudioUnitSetParameter(MixerAudioUnit, Volume)"));

		// Connect Mixer Audio Unit to Output Audio Unit
		AudioUnitConnection	audioUnitConnection;
		audioUnitConnection.sourceAudioUnit = mMixerAudioUnit;
		audioUnitConnection.sourceOutputNumber = 0;
		audioUnitConnection.destInputNumber = 0;
		size = sizeof(AudioUnitConnection);
		status =
				::AudioUnitSetProperty(mOutputAudioUnit, kAudioUnitProperty_MakeConnection, kAudioUnitScope_Input, 0,
						&audioUnitConnection, size);
		LogOSStatusIfFailed(status, OSSTR("AudioUnitSetProperty(Connecting MixerAudioUnit to OutputAudioUnit)"));

		size = sizeof(AudioStreamBasicDescription);
		status =
				::AudioUnitGetProperty(mMixerAudioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0,
						&mASBD, &size);
		LogOSStatusIfFailed(status, OSSTR("AudioUnitGetProperty(MixerAudioUnit, StreamFormat)"));

		mASBD.mSampleRate = outputAudioUnitSampleRate;
		status =
				::AudioUnitSetProperty(mMixerAudioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0,
						&mASBD, size);
		LogOSStatusIfFailed(status, OSSTR("AudioUnitSetProperty(MixerAudioUnit, StreamFormat)"));
	}

	return mASBD;
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CAudioEngine::getMaxOutputFrames()
//----------------------------------------------------------------------------------------------------------------------
{
	return mMaxOutputFrames;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioEngine::setMaxAudioPlayers(UInt32 maxAudioPlayers)
//----------------------------------------------------------------------------------------------------------------------
{
	mMaxAudioPlayers = maxAudioPlayers;
}

//----------------------------------------------------------------------------------------------------------------------
OV<UInt32> CAudioEngine::addAudioPlayer(AURenderCallback inputProc, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check for available slot
	for (UInt32 i = 0; i < mMaxAudioPlayers; i++) {
		// Check if thie Audio Engine Track has an Audio Player
		if (!mConnectedAudioPlayers.get(i)) {
			// Found an available Audio Engine Track
			mConnectedAudioPlayers.set(i);

			// Set render proc
			AURenderCallbackStruct renderCallbackStruct = {inputProc, userData};
			OSStatus	status =
								::AudioUnitSetProperty(mMixerAudioUnit, kAudioUnitProperty_SetRenderCallback,
										kAudioUnitScope_Input, i, &renderCallbackStruct,
										sizeof(AURenderCallbackStruct));
			LogOSStatusIfFailed(status, OSSTR("AudioUnitSetProperty(MixerAudioUnit, RenderCallback)"));

			return OV<UInt32>(i);
		}
	}

	// No available tracks
	CLogServices::logError(CString(OSSTR("CAudioEngine has no available tracks...")));

	return OV<UInt32>();
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioEngine::setAudioPlayerGain(UInt32 index, Float32 gain)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set gain
	OSStatus	status =
						::AudioUnitSetParameter(mMixerAudioUnit, kMultiChannelMixerParam_Volume, kAudioUnitScope_Input,
								index, gain, 0);
	LogOSStatusIfFailed(status, OSSTR("AudioUnitSetParameter(MixerAudioUnit, Volume)"));
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioEngine::play()
//----------------------------------------------------------------------------------------------------------------------
{
	// Start
	OSStatus	status = ::AudioOutputUnitStart(mOutputAudioUnit);
	LogOSStatusIfFailed(status, OSSTR("AudioOutputUnitStart()"));
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioEngine::removeAudioPlayer(UInt32 index)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set render proc to silence
	AURenderCallbackStruct renderCallbackStruct = {nil, nil};
	OSStatus	status =
						::AudioUnitSetProperty(mMixerAudioUnit, kAudioUnitProperty_SetRenderCallback,
								kAudioUnitScope_Input, index, &renderCallbackStruct, sizeof(AURenderCallbackStruct));
	LogOSStatusIfFailed(status, OSSTR("AudioUnitSetProperty(MixerAudioUnit, RenderCallback)"));

	// Clear Audio Player
	mConnectedAudioPlayers.clear(index);

	// Check if have any connected Audio Players
	for (UInt32 i = 0; i < mMaxAudioPlayers; i++) {
		// Check if have an active Audio Player here
		if (mConnectedAudioPlayers.get(i))
			// Yes
			return;
	}

	// No connected Audio Players
	status = ::AudioOutputUnitStop(mOutputAudioUnit);
	LogOSStatusIfFailed(status, OSSTR("AudioOutputUnitStop()"));
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioPlayer::Internals

class CAudioPlayer::Internals {
	public:
							Internals(CAudioPlayer& audioPlayer, const CString& identifier,
									const CAudioPlayer::Info& info) :
								mAudioPlayer(audioPlayer), mIdentifier(identifier), mInfo(info),
										mIsPlaying(false), mIsSeeking(false),
										mLastSeekTimeInterval(0.0), mCurrentPlaybackTimeInterval(0.0), mGain(1.0),
										mRenderProcShouldSendFrames(false), mRenderProcIsSendingFrames(false),
										mRenderProcShouldNotifyEndOfData(false), mRenderProcPreviousReadByteCount(0),
										mRenderProcPreviousFrameCount(0), mRenderProcFrameIndex(0),
										mRenderProcFrameCount(~0)
								{}

		static	void		readerThreadError(const SError& error, Internals* internals)
								{ internals->mInfo.error(internals->mAudioPlayer, error); }

		static	OSStatus	renderProc(void* inRefCon, AudioUnitRenderActionFlags* inActionFlags,
									const AudioTimeStamp* inTimeStamp, UInt32 inBusNumber, UInt32 inNumFrames,
									AudioBufferList* ioData)
								{
									// Setup
									Internals&	internals = *((Internals*) inRefCon);

									// Check if rendered any frames from the buffer
									if (internals.mRenderProcPreviousReadByteCount > 0) {
										// Commit previous read
										internals.mQueue->commitRead(internals.mRenderProcPreviousReadByteCount);
										internals.mRenderProcPreviousReadByteCount = 0;
									}

									// Check if rendered any frames
									if (internals.mRenderProcPreviousFrameCount > 0) {
										// Update info
										internals.mRenderProcFrameIndex += internals.mRenderProcPreviousFrameCount;
										internals.mCurrentPlaybackTimeInterval +=
												(UniversalTimeInterval) internals.mRenderProcPreviousFrameCount /
														*internals.mSampleRate;

										internals.mRenderProcFrameCount -= internals.mRenderProcPreviousFrameCount;
										internals.mRenderProcPreviousFrameCount = 0;

										// Check if seeking
										if (internals.mRenderProcShouldSendFrames && !internals.mIsSeeking)
											// Notify playback position updated
											internals.mInfo.positionUpdated(internals.mAudioPlayer,
													internals.mCurrentPlaybackTimeInterval);

										// Notify queue read complete
										internals.mAudioPlayerBufferThread->noteQueueReadComplete();
									}

									// Check if should send frames
									if (internals.mRenderProcShouldSendFrames) {
										// Sending frames
										CSRSWBIPSegmentedQueue::ReadBufferInfo	readBufferInfo =
																						internals.mQueue->requestRead();
										UInt32									frameCount =
																						std::min<UInt32>(inNumFrames,
																								internals
																									.mRenderProcFrameCount);
										UInt32									requiredByteCount =
																						frameCount *
																								*internals.mBytesPerFrame;
										if (readBufferInfo.hasBuffer() &&
												(readBufferInfo.mByteCount >= requiredByteCount) &&
												(frameCount == inNumFrames)) {
											// Can point to buffer
											for (UInt32 i = 0; i < ioData->mNumberBuffers; i++)
												// Prepare this buffer
												ioData->mBuffers[i].mData = readBufferInfo.bufferAtIndex(i);

											// Store
											internals.mRenderProcIsSendingFrames = true;
											internals.mRenderProcPreviousReadByteCount = requiredByteCount;
											internals.mRenderProcPreviousFrameCount = inNumFrames;
										} else {
											// Must copy buffers
											UInt32	byteOffset = 0;
											while ((requiredByteCount > 0) && readBufferInfo.hasBuffer()) {
												// Must copy
												UInt32	bytesToCopy =
																std::min<UInt32>(requiredByteCount,
																		readBufferInfo.mByteCount);
												for (UInt32 i = 0; i < ioData->mNumberBuffers; i++)
													// Copy
													::memcpy((UInt8*) ioData->mBuffers[i].mData + byteOffset,
															readBufferInfo.bufferAtIndex(i), bytesToCopy);
												internals.mQueue->commitRead(bytesToCopy);

												// Update info
												byteOffset += bytesToCopy;
												requiredByteCount -= bytesToCopy;
												internals.mRenderProcPreviousFrameCount +=
														bytesToCopy / *internals.mBytesPerFrame;

												// Get next read buffer info
												readBufferInfo = internals.mQueue->requestRead();
											}

											// Check situation
											if (byteOffset < ioData->mBuffers[0].mDataByteSize) {
												// Check if all silence
												if (internals.mRenderProcPreviousFrameCount == 0)
													// Buffer is entirely silence
													*inActionFlags = kAudioUnitRenderAction_OutputIsSilence;

												// Fill the rest with silence
												for (UInt32 i = 0; i < ioData->mNumberBuffers; i++)
													// Clear
													::memset((UInt8*) ioData->mBuffers[i].mData + byteOffset, 0,
															ioData->mBuffers[i].mDataByteSize - byteOffset);

												// Check situation
												if ((internals.mRenderProcFrameIndex == 0) &&
														(internals.mRenderProcPreviousFrameCount == 0))
													// Was recently started, but the reader thread doesn't yet have
													//	frames queued.
													internals.mRenderProcIsSendingFrames = true;
												else if (requiredByteCount == 0)
													// Fulfilled the requirements, but did not fill a buffer.  Must be
													//	seeking
													internals.mRenderProcIsSendingFrames = true;
												else if (!internals.mAudioPlayerBufferThread->getDidReachEnd()) {
													// Have a discontinuity in the queued frames
													CLogServices::logError(
															CString(OSSTR("CAudioPlayer ")) + internals.mIdentifier +
																	CString(OSSTR(" requires ")) +
																	CString(inNumFrames) +
																	CString(OSSTR(" frames, but is ")) +
																	CString(inNumFrames -
																			internals.mRenderProcPreviousFrameCount) +
																	CString(OSSTR(" frames short, will glitch...")));

													internals.mRenderProcIsSendingFrames = true;
												} else if (internals.mRenderProcPreviousFrameCount > 0)
													// At the end, but have queued framnes
													internals.mRenderProcIsSendingFrames = true;
												else {
													// At the end, did not queue any frames
													if (internals.mRenderProcIsSendingFrames &&
															internals.mRenderProcShouldNotifyEndOfData)
														// Just reached the end of data
														internals.mInfo.endOfData(internals.mAudioPlayer);

													internals.mRenderProcIsSendingFrames = false;
												}
											} else
												// Update
												internals.mRenderProcIsSendingFrames = true;
										}

										// Check if have frames
										if (internals.mRenderProcPreviousFrameCount > 0) {
											// Iterate channels
											for (UInt32 channelIndex = 0; channelIndex < ioData->mNumberBuffers;
													channelIndex++) {
												// Setup
												Float32		gain =
																	(channelIndex < internals.mChannelGains.getCount()) ?
																			internals.mChannelGains[channelIndex] :
																			internals.mGain;
												Float32*	samplePtr = (Float32*) ioData->mBuffers[channelIndex].mData;

												// Apply
												for (UInt32 sampleIndex = 0;
														sampleIndex < internals.mRenderProcPreviousFrameCount;
														sampleIndex++, *(samplePtr++) *= gain) ;
											}
										}
									} else {
										// Not sending frames
										*inActionFlags = kAudioUnitRenderAction_OutputIsSilence;
										for (UInt32 i = 0; i < ioData->mNumberBuffers; i++)
											// Clear
											::memset(ioData->mBuffers[i].mData, 0, ioData->mBuffers[i].mDataByteSize);

										internals.mRenderProcIsSendingFrames = false;
									}

									return noErr;
								}

		CAudioPlayer&					mAudioPlayer;
		CString							mIdentifier;
		CAudioPlayer::Info				mInfo;

		OI<CAudioPlayerBufferThread>	mAudioPlayerBufferThread;
		OI<CSRSWBIPSegmentedQueue>		mQueue;
		OV<UInt32>						mAudioEngineIndex;
		OV<SAudio::ProcessingFormat>	mAudioProcessingFormat;
		OV<Float32>						mSampleRate;
		OV<UInt32>						mBytesPerFrame;

		bool							mIsPlaying;
		bool							mIsSeeking;
		OV<SMedia::Segment>				mMediaSegment;
		UniversalTimeInterval			mLastSeekTimeInterval;
		UniversalTimeInterval			mCurrentPlaybackTimeInterval;
		Float32							mGain;
		TNumberArray<Float32>			mChannelGains;

		bool							mRenderProcShouldSendFrames;
		bool							mRenderProcIsSendingFrames;
		bool							mRenderProcShouldNotifyEndOfData;
		UInt32							mRenderProcPreviousReadByteCount;
		UInt32							mRenderProcPreviousFrameCount;
		UInt32							mRenderProcFrameIndex;
		UInt32							mRenderProcFrameCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioPlayer

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioPlayer::CAudioPlayer(const CString& identifier, const Info& info) : CAudioDestination()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(*this, identifier, info);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioPlayer::~CAudioPlayer()
//----------------------------------------------------------------------------------------------------------------------
{
	// Stop sending frames
	mInternals->mRenderProcShouldSendFrames = false;
	while (mInternals->mRenderProcIsSendingFrames)
		// Sleep
		CThread::sleepFor(0.001);

	// Remove from engine
	if (mInternals->mAudioEngineIndex.hasValue())
		// Remove
		CAudioEngine::mShared.removeAudioPlayer(*mInternals->mAudioEngineIndex);

	// Stop threads
	if (mInternals->mAudioPlayerBufferThread.hasInstance())
		// Shutdown
		mInternals->mAudioPlayerBufferThread->shutdown();

	// Cleanup
	Delete(mInternals);
}

// MARK: CAudioProcessor methods

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CAudioPlayer::connectInput(const I<CAudioProcessor>& audioProcessor,
		const SAudio::ProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mAudioProcessingFormat.setValue(audioProcessingFormat);
	
	// Setup
	UInt32	segmentCount;
	mInternals->mSampleRate = OV<Float32>(audioProcessingFormat.getSampleRate());
	if (audioProcessingFormat.getIsInterleaved()) {
		// Interleaved
		mInternals->mBytesPerFrame =
				OV<UInt32>(audioProcessingFormat.getBits() / 8 *
						audioProcessingFormat.getChannelMap().getChannelCount());
		segmentCount = 1;
	} else {
		// Non-interleaved
		mInternals->mBytesPerFrame = OV<UInt32>(audioProcessingFormat.getBits() / 8);
		segmentCount = audioProcessingFormat.getChannelMap().getChannelCount();
	}

	UInt32	frameCount = CAudioPlayer::getPlaybackBufferDuration() * audioProcessingFormat.getSampleRate();
	mInternals->mQueue =
			OI<CSRSWBIPSegmentedQueue>(
					new CSRSWBIPSegmentedQueue(segmentCount, frameCount * *mInternals->mBytesPerFrame));
	mInternals->mAudioPlayerBufferThread =
			OI<CAudioPlayerBufferThread>(
					new CAudioPlayerBufferThread(*this, *mInternals->mQueue, *mInternals->mBytesPerFrame,
							CAudioEngine::mShared.getMaxOutputFrames(),
							(CAudioPlayerBufferThread::ErrorProc) Internals::readerThreadError, mInternals));

	// Do super
	return CAudioProcessor::connectInput(audioProcessor, audioProcessingFormat);
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CString> CAudioPlayer::getSetupDescription(const CString& indent)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get upstream setup descriptions
	TNArray<CString>	setupDescriptions(CAudioDestination::getSetupDescription(indent));

	// Add our setup description
	setupDescriptions += indent + CString(OSSTR("Audio Player"));
	setupDescriptions += indent + CString(OSSTR("    ")) + mInternals->mAudioProcessingFormat->getDescription();

	return setupDescriptions;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::setMediaSegment(const OV<SMedia::Segment>& mediaSegment)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mMediaSegment = mediaSegment;

	// Do super
	CAudioDestination::setMediaSegment(mediaSegment);
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if have audio engine index
	if (!mInternals->mAudioEngineIndex.hasValue()) {
		// Add player
		mInternals->mAudioEngineIndex = CAudioEngine::mShared.addAudioPlayer(Internals::renderProc, mInternals);
		CAudioEngine::mShared.setAudioPlayerGain(*mInternals->mAudioEngineIndex, 1.0);
	}
	if (!mInternals->mAudioEngineIndex.hasValue())
		// No available slots
		return;

	// Stop sending frames
	mInternals->mRenderProcShouldSendFrames = false;
	while (mInternals->mRenderProcIsSendingFrames)
		// Sleep
		CThread::sleepFor(0.001);

	// Pause Reader Thread
	mInternals->mAudioPlayerBufferThread->pause();

	// Reset buffer
	mInternals->mQueue->reset();

	// Reset and seek
	CAudioDestination::reset();
	CAudioDestination::seek(timeInterval);

	// Update stuffs
	mInternals->mIsSeeking = !mInternals->mIsPlaying;
	mInternals->mLastSeekTimeInterval = timeInterval;
	mInternals->mCurrentPlaybackTimeInterval = timeInterval;

	mInternals->mRenderProcFrameIndex = 0;
	mInternals->mRenderProcFrameCount =
			!mInternals->mIsPlaying ? (UInt32) (*mInternals->mSampleRate * CAudioPlayer::kPreviewDuration) : ~0;

	// Resume
	mInternals->mAudioPlayerBufferThread->resume();

	// Send frames
	mInternals->mRenderProcShouldSendFrames = true;

	// Start the engine if not already started
	CAudioEngine::mShared.play();

	// Notify playback position updated
	mInternals->mInfo.positionUpdated(*this, timeInterval);
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Stop sending frames
	mInternals->mRenderProcShouldSendFrames = false;
	while (mInternals->mRenderProcIsSendingFrames)
		// Sleep
		CThread::sleepFor(0.001);

	// Remove from engine
	if (mInternals->mAudioEngineIndex.hasValue()) {
		// Remove
		CAudioEngine::mShared.removeAudioPlayer(*mInternals->mAudioEngineIndex);
		mInternals->mAudioEngineIndex = OV<UInt32>();
	}

	// No longer playing
	mInternals->mIsPlaying = false;

	// Pause Reader Thread
	mInternals->mAudioPlayerBufferThread->pause();

	// Reset buffer
	mInternals->mQueue->reset();

	// Reset the pipeline
	CAudioDestination::reset();

	// Reset
	mInternals->mCurrentPlaybackTimeInterval =
			mInternals->mMediaSegment.hasValue() ? mInternals->mMediaSegment->getStartTimeInterval() : 0.0;

	mInternals->mRenderProcFrameIndex = 0;
	mInternals->mRenderProcFrameCount = ~0;

	// Resume
	mInternals->mAudioPlayerBufferThread->resume();
}

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudio::ProcessingSetup> CAudioPlayer::getInputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup if necessary
	static	SAudio::ProcessingSetup*	sAudioProcessingSetup = nil;
	if (sAudioProcessingSetup == nil) {
		// Compose SAudio::ProcessingSetup
		AudioStreamBasicDescription	asbd = CAudioEngine::mShared.getInputFormat();
		sAudioProcessingSetup =
				new SAudio::ProcessingSetup(asbd.mBitsPerChannel, asbd.mSampleRate,
						SAudio::ChannelMap((UInt8) asbd.mChannelsPerFrame),
						((asbd.mFormatFlags & kAudioFormatFlagIsFloat) != 0) ?
								SAudio::ProcessingSetup::SampleTypeOption::kSampleTypeFloat :
								SAudio::ProcessingSetup::SampleTypeOption::kSampleTypeSignedInteger,
						SAudio::ProcessingSetup::EndianOption::kEndianNative,
						(asbd.mFormatFlags & kAudioFormatFlagIsNonInterleaved) ?
								SAudio::ProcessingSetup::InterleavedOption::kNonInterleaved :
								SAudio::ProcessingSetup::InterleavedOption::kInterleaved);
	}

	return TNArray<SAudio::ProcessingSetup>(*sAudioProcessingSetup);
}

// MARK: CAudioDestination methods

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::setupComplete()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mAudioPlayerBufferThread->resume();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CString& CAudioPlayer::getIdentifier() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mIdentifier;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::setGain(Float32 gain)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mGain = gain;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::setGain(const TNumberArray<Float32>& channelGains)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mChannelGains = channelGains;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::play()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if have audio engine index
	if (!mInternals->mAudioEngineIndex.hasValue()) {
		// Add player
		mInternals->mAudioEngineIndex = CAudioEngine::mShared.addAudioPlayer(Internals::renderProc, mInternals);
		CAudioEngine::mShared.setAudioPlayerGain(*mInternals->mAudioEngineIndex, 1.0);
	}
	if (!mInternals->mAudioEngineIndex.hasValue())
		// No available slots
		return;

	// We are now playing
	mInternals->mIsPlaying = true;
	mInternals->mIsSeeking = false;

	// Send frames
	mInternals->mRenderProcShouldSendFrames = true;
	mInternals->mRenderProcShouldNotifyEndOfData = true;
	mInternals->mRenderProcFrameCount = ~0;

	// Start the engine if not already started
	CAudioEngine::mShared.play();
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::pause()
//----------------------------------------------------------------------------------------------------------------------
{
	// Pause
	mInternals->mIsPlaying = false;

	// Don't send frames
	mInternals->mRenderProcShouldSendFrames = false;
}

//----------------------------------------------------------------------------------------------------------------------
bool CAudioPlayer::isPlaying() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mIsPlaying;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::startSeek()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if have audio engine index
	if (!mInternals->mAudioEngineIndex.hasValue()) {
		// Add player
		mInternals->mAudioEngineIndex = CAudioEngine::mShared.addAudioPlayer(Internals::renderProc, mInternals);
		CAudioEngine::mShared.setAudioPlayerGain(*mInternals->mAudioEngineIndex, 1.0);
	}
	if (!mInternals->mAudioEngineIndex.hasValue())
		// No available slots
		return;

	// Stop sending frames
	mInternals->mRenderProcShouldSendFrames = false;
	mInternals->mRenderProcShouldNotifyEndOfData = false;

	// Setup
	mInternals->mIsSeeking = true;
	mInternals->mLastSeekTimeInterval = mInternals->mCurrentPlaybackTimeInterval;

	// Start the engine if not already started
	CAudioEngine::mShared.play();
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::finishSeek()
//----------------------------------------------------------------------------------------------------------------------
{
	// Stop sending frames
	mInternals->mRenderProcShouldSendFrames = false;
	while (mInternals->mRenderProcIsSendingFrames)
		// Sleep
		CThread::sleepFor(0.001);

	// Pause Reader Thread
	mInternals->mAudioPlayerBufferThread->pause();

	// Reset buffer
	mInternals->mQueue->reset();

	// Seek to the last seek time interval
	CAudioDestination::reset();
	CAudioDestination::seek(mInternals->mLastSeekTimeInterval);

	// Reset stuffs
	mInternals->mIsSeeking = false;
	mInternals->mCurrentPlaybackTimeInterval = mInternals->mLastSeekTimeInterval;

	mInternals->mRenderProcFrameIndex = 0;
	mInternals->mRenderProcFrameCount = ~0;

	// Resume
	mInternals->mAudioPlayerBufferThread->resume();

	// Begin sending frames again
	mInternals->mRenderProcShouldSendFrames = mInternals->mIsPlaying;
	mInternals->mRenderProcShouldNotifyEndOfData = true;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::setMaxAudioPlayers(UInt32 maxAudioPlayers)
//----------------------------------------------------------------------------------------------------------------------
{
	CAudioEngine::mShared.setMaxAudioPlayers(maxAudioPlayers);
}
