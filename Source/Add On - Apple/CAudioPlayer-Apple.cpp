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

				SAudio::ProcessingSetup		getAudioProcessingSetup();

				UInt32						getMaxOutputFrames();

				void						setMaxAudioPlayers(UInt32 maxAudioPlayers);
				OV<UInt32>					addAudioPlayer(AURenderCallback inputProc, void* userData);
				void						setAudioPlayerGains(UInt32 index, Float32 gain,
													const TNumberArray<Float32>& channelGains);
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
SAudio::ProcessingSetup CAudioEngine::getAudioProcessingSetup()
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
		LogOSStatusIfFailed(status, CString(OSSTR("AudioComponentInstanceNew(OutputAudioUnit)")));

		status = ::AudioUnitInitialize(mOutputAudioUnit);
		LogOSStatusIfFailed(status, CString(OSSTR("AudioUnitInitialize(OutputAudioUnit)")));

		AudioStreamBasicDescription	asbd;
		UInt32						size = sizeof(AudioStreamBasicDescription);
		status =
				::AudioUnitGetProperty(mOutputAudioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0,
						&asbd, &size);
		LogOSStatusIfFailed(status, CString(OSSTR("AudioUnitGetProperty(OutputAudioUnit, StreamFormat)")));
		Float32	outputAudioUnitSampleRate = asbd.mSampleRate;

		// Setup Mixer Audio Unit
		audioComponentDescription.componentType = kAudioUnitType_Mixer;
		audioComponentDescription.componentSubType =
				kAudioUnitSubType_MultiChannelMixer;
		audioComponent = ::AudioComponentFindNext(nil, &audioComponentDescription);

		status = ::AudioComponentInstanceNew(audioComponent, &mMixerAudioUnit);
		LogOSStatusIfFailed(status, CString(OSSTR("AudioComponentInstanceNew(MixerAudioUnit)")));

		status = ::AudioUnitInitialize(mMixerAudioUnit);
		LogOSStatusIfFailed(status, CString(OSSTR("AudioUnitInitialize(MixerAudioUnit)")));

		size = sizeof(UInt32);
		status =
				::AudioUnitGetProperty(mMixerAudioUnit, kAudioUnitProperty_MaximumFramesPerSlice,
						kAudioUnitScope_Global, 0, &mMaxOutputFrames, &size);
		LogOSStatusIfFailed(status, CString(OSSTR("AudioUnitGetProperty(MixerAudioUnit, MaximumFramesPerSlice)")));

		size = sizeof(UInt32);
		status =
				::AudioUnitSetProperty(mMixerAudioUnit, kAudioUnitProperty_ElementCount, kAudioUnitScope_Input, 0,
						&mMaxAudioPlayers, size);
		LogOSStatusIfFailed(status, CString(OSSTR("AudioUnitSetProperty(MixerAudioUnit, ElementCount)")));

		status =
				::AudioUnitSetParameter(mMixerAudioUnit, kMultiChannelMixerParam_Volume, kAudioUnitScope_Output, 0, 1.0,
						0);
		LogOSStatusIfFailed(status, CString(OSSTR("AudioUnitSetParameter(MixerAudioUnit, Volume)")));

		// Connect Mixer Audio Unit to Output Audio Unit
		AudioUnitConnection	audioUnitConnection;
		audioUnitConnection.sourceAudioUnit = mMixerAudioUnit;
		audioUnitConnection.sourceOutputNumber = 0;
		audioUnitConnection.destInputNumber = 0;
		size = sizeof(AudioUnitConnection);
		status =
				::AudioUnitSetProperty(mOutputAudioUnit, kAudioUnitProperty_MakeConnection, kAudioUnitScope_Input, 0,
						&audioUnitConnection, size);
		LogOSStatusIfFailed(status,
				CString(OSSTR("AudioUnitSetProperty(Connecting MixerAudioUnit to OutputAudioUnit)")));

		size = sizeof(AudioStreamBasicDescription);
		status =
				::AudioUnitGetProperty(mMixerAudioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0,
						&mASBD, &size);
		LogOSStatusIfFailed(status, CString(OSSTR("AudioUnitGetProperty(MixerAudioUnit, StreamFormat)")));

		mASBD.mSampleRate = outputAudioUnitSampleRate;
		status =
				::AudioUnitSetProperty(mMixerAudioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0,
						&mASBD, size);
		LogOSStatusIfFailed(status, CString(OSSTR("AudioUnitSetProperty(MixerAudioUnit, StreamFormat)")));
	}

	return SAudio::ProcessingSetup(mASBD.mBitsPerChannel, mASBD.mSampleRate,
			SAudio::ChannelMap((UInt8) mASBD.mChannelsPerFrame),
			((mASBD.mFormatFlags & kAudioFormatFlagIsFloat) != 0) ?
					SAudio::ProcessingSetup::SampleTypeOption::kSampleTypeFloat :
					SAudio::ProcessingSetup::SampleTypeOption::kSampleTypeSignedInteger,
			SAudio::ProcessingSetup::EndianOption::kEndianNative,
			(mASBD.mFormatFlags & kAudioFormatFlagIsNonInterleaved) ?
					SAudio::ProcessingSetup::InterleavedOption::kNonInterleaved :
					SAudio::ProcessingSetup::InterleavedOption::kInterleaved);
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
	mConnectedAudioPlayers = CBits(maxAudioPlayers);
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
			LogOSStatusIfFailed(status, CString(OSSTR("AudioUnitSetProperty(MixerAudioUnit, RenderCallback)")));

			return OV<UInt32>(i);
		}
	}

	// No available tracks
	CLogServices::logError(CString(OSSTR("CAudioEngine has no available tracks...")));

	return OV<UInt32>();
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioEngine::setAudioPlayerGains(UInt32 index, Float32 gain, const TNumberArray<Float32>& channelGains)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get output format
	AudioStreamBasicDescription	outputASBD;
	UInt32						size = sizeof(AudioStreamBasicDescription);
	OSStatus					status =
										::AudioUnitGetProperty(mMixerAudioUnit, kAudioUnitProperty_StreamFormat,
												kAudioUnitScope_Output, 0, &outputASBD, &size);
	LogOSStatusIfFailedAndReturn(status, CString(OSSTR("AudioUnitGetProperty(MixerAudioUnit, Output StreamFormat)")));

	// Compose matrix levels.  Each input channel goes to the same output channel at gain * its channel gain.
	TBuffer<Float32>	matrixLevels(mASBD.mChannelsPerFrame * outputASBD.mChannelsPerFrame);
	for (UInt32 i = 0; (i < mASBD.mChannelsPerFrame) && (i < outputASBD.mChannelsPerFrame); i++)
		// Set level
		matrixLevels[i * outputASBD.mChannelsPerFrame + i] =
				gain * ((i < channelGains.getCount()) ? channelGains[i] : 1.0f);

	// Set matrix levels
	status =
			::AudioUnitSetProperty(mMixerAudioUnit, kAudioUnitProperty_MatrixLevels, kAudioUnitScope_Input, index,
					*matrixLevels, (UInt32) matrixLevels.getByteCount());
	LogOSStatusIfFailed(status, CString(OSSTR("AudioUnitSetProperty(MixerAudioUnit, MatrixLevels)")));

	// Set volume.  An input's volume starts at 0 and multiplies with its matrix levels.
	status =
			::AudioUnitSetParameter(mMixerAudioUnit, kMultiChannelMixerParam_Volume, kAudioUnitScope_Input, index, 1.0,
					0);
	LogOSStatusIfFailed(status, CString(OSSTR("AudioUnitSetParameter(MixerAudioUnit, Volume)")));
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioEngine::play()
//----------------------------------------------------------------------------------------------------------------------
{
	// Start
	OSStatus	status = ::AudioOutputUnitStart(mOutputAudioUnit);
	LogOSStatusIfFailed(status, CString(OSSTR("AudioOutputUnitStart()")));
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
	LogOSStatusIfFailed(status, CString(OSSTR("AudioUnitSetProperty(MixerAudioUnit, RenderCallback)")));

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
	LogOSStatusIfFailed(status, CString(OSSTR("AudioOutputUnitStop()")));
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
										mRenderProcIsReadingQueue(false), mRenderProcShouldNotifyEndOfData(false),
										mRenderProcPreviousReadByteCount(0), mRenderProcPreviousFrameCount(0),
										mRenderProcFrameIndex(0), mRenderProcFrameCount(~0)
								{}

				void		waitForRenderProcToStopReadingQueue()
								{
									// Wait for the render proc to stop reading the queue.  If the output is no longer
									//	calling the render proc, nothing is reading the queue, so stop waiting.
									UniversalTime	deadline =
															SUniversalTime::getCurrent() +
																	CAudioPlayer::getPlaybackBufferDuration();
									while (mRenderProcIsReadingQueue && (SUniversalTime::getCurrent() < deadline))
										// Sleep
										CThread::sleepFor(0.001);

									mRenderProcIsReadingQueue = false;
								}

		static	void		readerThreadError(const SError& error, Internals* internals)
								{ internals->mInfo.error(internals->mAudioPlayer, error); }

		static	OSStatus	renderProc(void* inRefCon, AudioUnitRenderActionFlags* inActionFlags,
									const AudioTimeStamp* inTimeStamp, UInt32 inBusNumber, UInt32 inNumFrames,
									AudioBufferList* ioData)
								{
									// Setup
									Internals&	internals = *((Internals*) inRefCon);

									// Note reading queue
									internals.mRenderProcIsReadingQueue = true;

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
										internals.mCurrentPlaybackTimeInterval =
												internals.mCurrentPlaybackTimeInterval +
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
										internals.mBufferThread->noteQueueReadComplete();
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
												else if (!internals.mBufferThread->getDidReachEnd()) {
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
									} else {
										// Not sending frames
										*inActionFlags = kAudioUnitRenderAction_OutputIsSilence;
										for (UInt32 i = 0; i < ioData->mNumberBuffers; i++)
											// Clear
											::memset(ioData->mBuffers[i].mData, 0, ioData->mBuffers[i].mDataByteSize);

										internals.mRenderProcIsSendingFrames = false;
										internals.mRenderProcIsReadingQueue = false;
									}

									return noErr;
								}

		CAudioPlayer&						mAudioPlayer;
		CString								mIdentifier;
		CAudioPlayer::Info					mInfo;

		OI<BufferThread>					mBufferThread;
		OI<CSRSWBIPSegmentedQueue>			mQueue;
		OV<UInt32>							mAudioEngineIndex;
		OV<SAudio::ProcessingFormat>		mAudioProcessingFormat;
		OV<Float32>							mSampleRate;
		OV<UInt32>							mBytesPerFrame;

		bool								mIsPlaying;
		std::atomic<bool>					mIsSeeking;
		OV<SMedia::Segment>					mMediaSegment;
		UniversalTimeInterval				mLastSeekTimeInterval;
		std::atomic<UniversalTimeInterval>	mCurrentPlaybackTimeInterval;
		Float32								mGain;
		TNumberArray<Float32>				mChannelGains;

		std::atomic<bool>					mRenderProcShouldSendFrames;
		bool								mRenderProcIsSendingFrames;
		std::atomic<bool>					mRenderProcIsReadingQueue;
		std::atomic<bool>					mRenderProcShouldNotifyEndOfData;
		UInt32								mRenderProcPreviousReadByteCount;
		UInt32								mRenderProcPreviousFrameCount;
		UInt32								mRenderProcFrameIndex;
		std::atomic<UInt32>					mRenderProcFrameCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioPlayer

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioPlayer::CAudioPlayer(const CString& identifier, const Info& info) :
		CAudioDestination(CAudioEngine::mShared.getAudioProcessingSetup())
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
	mInternals->waitForRenderProcToStopReadingQueue();

	// Remove from engine
	if (mInternals->mAudioEngineIndex.hasValue())
		// Remove
		CAudioEngine::mShared.removeAudioPlayer(*mInternals->mAudioEngineIndex);

	// Stop threads
	if (mInternals->mBufferThread.hasInstance())
		// Shutdown
		mInternals->mBufferThread->shutdown();

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
	mInternals->mBufferThread =
			OI<BufferThread>(
					new BufferThread(*this, *mInternals->mQueue, *mInternals->mBytesPerFrame,
							CAudioEngine::mShared.getMaxOutputFrames(),
							(BufferThread::ErrorProc) Internals::readerThreadError, mInternals));

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
		if (!mInternals->mAudioEngineIndex.hasValue())
			// No available slots
			return;

		CAudioEngine::mShared.setAudioPlayerGains(*mInternals->mAudioEngineIndex, mInternals->mGain,
				mInternals->mChannelGains);
	}

	// Stop sending frames
	mInternals->mRenderProcShouldSendFrames = false;
	mInternals->waitForRenderProcToStopReadingQueue();

	// Pause Reader Thread
	mInternals->mBufferThread->pause();

	// Reset buffer
	mInternals->mQueue->reset();

	// Seek
	CAudioDestination::seek(timeInterval);

	// Update stuffs
	mInternals->mIsSeeking = !mInternals->mIsPlaying;
	mInternals->mLastSeekTimeInterval = timeInterval;
	mInternals->mCurrentPlaybackTimeInterval = timeInterval;

	mInternals->mRenderProcFrameIndex = 0;
	mInternals->mRenderProcFrameCount =
			!mInternals->mIsPlaying ? (UInt32) (*mInternals->mSampleRate * CAudioPlayer::mPreviewDuration) : ~0;

	// Resume
	mInternals->mBufferThread->resume();

	// Send frames
	mInternals->mRenderProcShouldSendFrames = true;

	// Start the engine if not already started
	CAudioEngine::mShared.play();

	// Notify playback position updated
	mInternals->mInfo.positionUpdated(*this, timeInterval);
}

//----------------------------------------------------------------------------------------------------------------------
TNumberArray<Float32> CAudioPlayer::setChannelGains(const TNumberArray<Float32>& channelGains)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mChannelGains = CAudioDestination::setChannelGains(channelGains);

	// Check if have audio engine index
	if (mInternals->mAudioEngineIndex.hasValue())
		// Update
		CAudioEngine::mShared.setAudioPlayerGains(*mInternals->mAudioEngineIndex, mInternals->mGain,
				mInternals->mChannelGains);

	return TNumberArray<Float32>();
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::stop()
//----------------------------------------------------------------------------------------------------------------------
{
	// Stop sending frames
	mInternals->mRenderProcShouldSendFrames = false;
	mInternals->waitForRenderProcToStopReadingQueue();

	// Remove from engine
	if (mInternals->mAudioEngineIndex.hasValue()) {
		// Remove
		CAudioEngine::mShared.removeAudioPlayer(*mInternals->mAudioEngineIndex);
		mInternals->mAudioEngineIndex = OV<UInt32>();
	}

	// No longer playing
	mInternals->mIsPlaying = false;

	// Pause Reader Thread
	mInternals->mBufferThread->pause();

	// Reset buffer
	mInternals->mQueue->reset();

	// Seek to the start of the segment if present, or start
	CAudioDestination::seek(
			mInternals->mMediaSegment.hasValue() ? mInternals->mMediaSegment->getStartTimeInterval() : 0.0);

	// Reset
	mInternals->mCurrentPlaybackTimeInterval =
			mInternals->mMediaSegment.hasValue() ? mInternals->mMediaSegment->getStartTimeInterval() : 0.0;

	mInternals->mRenderProcFrameIndex = 0;
	mInternals->mRenderProcFrameCount = ~0;

	// Resume
	mInternals->mBufferThread->resume();
}

// MARK: CAudioDestination methods

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::setupComplete()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mBufferThread->resume();
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

	// Check if have audio engine index
	if (mInternals->mAudioEngineIndex.hasValue())
		// Update
		CAudioEngine::mShared.setAudioPlayerGains(*mInternals->mAudioEngineIndex, mInternals->mGain,
				mInternals->mChannelGains);
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::play()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if have audio engine index
	if (!mInternals->mAudioEngineIndex.hasValue()) {
		// Add player
		mInternals->mAudioEngineIndex = CAudioEngine::mShared.addAudioPlayer(Internals::renderProc, mInternals);
		if (!mInternals->mAudioEngineIndex.hasValue())
			// No available slots
			return;

		CAudioEngine::mShared.setAudioPlayerGains(*mInternals->mAudioEngineIndex, mInternals->mGain,
				mInternals->mChannelGains);
	}

	// We are now playing
	mInternals->mIsPlaying = true;
	mInternals->mIsSeeking = false;

	// Send frames
	mInternals->mRenderProcShouldNotifyEndOfData = true;
	mInternals->mRenderProcFrameCount = ~0;
	mInternals->mRenderProcShouldSendFrames = true;

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
		if (!mInternals->mAudioEngineIndex.hasValue())
			// No available slots
			return;

		CAudioEngine::mShared.setAudioPlayerGains(*mInternals->mAudioEngineIndex, mInternals->mGain,
				mInternals->mChannelGains);
	}

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
	mInternals->waitForRenderProcToStopReadingQueue();

	// Pause Reader Thread
	mInternals->mBufferThread->pause();

	// Reset buffer
	mInternals->mQueue->reset();

	// Seek to the last seek time interval
	CAudioDestination::seek(mInternals->mLastSeekTimeInterval);

	// Reset stuffs
	mInternals->mIsSeeking = false;
	mInternals->mCurrentPlaybackTimeInterval = mInternals->mLastSeekTimeInterval;

	mInternals->mRenderProcFrameIndex = 0;
	mInternals->mRenderProcFrameCount = ~0;

	// Resume
	mInternals->mBufferThread->resume();

	// Begin sending frames again
	mInternals->mRenderProcShouldNotifyEndOfData = true;
	mInternals->mRenderProcShouldSendFrames = mInternals->mIsPlaying;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TVResult<I<CAudioPlayer> > CAudioPlayer::create(const CString& identifier, const Info& info)
//----------------------------------------------------------------------------------------------------------------------
{
	return TVResult<I<CAudioPlayer> >(I<CAudioPlayer>(new CAudioPlayer(identifier, info)));
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::setMaxAudioPlayers(UInt32 maxAudioPlayers)
//----------------------------------------------------------------------------------------------------------------------
{
	CAudioEngine::mShared.setMaxAudioPlayers(maxAudioPlayers);
}
