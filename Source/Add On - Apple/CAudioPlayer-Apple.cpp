//----------------------------------------------------------------------------------------------------------------------
//	CAudioPlayer-Apple.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioPlayer.h"

#include "CBits.h"
#include "CLogServices-Apple.h"
#include "SError-Apple.h"

#import <AudioToolbox/AudioToolbox.h>

/*
	References:
		https://github.com/thestk/rtaudio
*/

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sErrorDomain(OSSTR("CAudioPlayer-Apple"));
static	SError	sUnableToLoadTracks(sErrorDomain, 1, CString(OSSTR("No available tracks")));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioEngine

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
#if TARGET_OS_IOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
		audioComponentDescription.componentSubType = kAudioUnitSubType_RemoteIO;
#else
		audioComponentDescription.componentSubType = kAudioUnitSubType_DefaultOutput;
#endif
		AudioComponent	audioComponent = ::AudioComponentFindNext(nil, &audioComponentDescription);

		status = ::AudioComponentInstanceNew(audioComponent, &mOutputAudioUnit);
		LOG_OSSTATUS_IF_FAILED(status, OSSTR("AudioComponentInstanceNew(OutputAudioUnit)"));

		status = ::AudioUnitInitialize(mOutputAudioUnit);
		LOG_OSSTATUS_IF_FAILED(status, OSSTR("AudioUnitInitialize(OutputAudioUnit)"));

		AudioStreamBasicDescription	asbd;
		UInt32						size = sizeof(AudioStreamBasicDescription);
		status =
				::AudioUnitGetProperty(mOutputAudioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0,
						&asbd, &size);
		LOG_OSSTATUS_IF_FAILED(status, OSSTR("AudioUnitGetProperty(OutputAudioUnit, StreamFormat)"));
		Float32	outputAudioUnitSampleRate = asbd.mSampleRate;

		// Setup Mixer Audio Unit
		audioComponentDescription.componentType = kAudioUnitType_Mixer;
		audioComponentDescription.componentSubType =
				kAudioUnitSubType_MultiChannelMixer;
		audioComponent = ::AudioComponentFindNext(nil, &audioComponentDescription);

		status = ::AudioComponentInstanceNew(audioComponent, &mMixerAudioUnit);
		LOG_OSSTATUS_IF_FAILED(status, OSSTR("AudioComponentInstanceNew(MixerAudioUnit)"));

		status = ::AudioUnitInitialize(mMixerAudioUnit);
		LOG_OSSTATUS_IF_FAILED(status, OSSTR("AudioUnitInitialize(MixerAudioUnit)"));

		size = sizeof(UInt32);
		status =
				::AudioUnitGetProperty(mMixerAudioUnit, kAudioUnitProperty_MaximumFramesPerSlice,
						kAudioUnitScope_Global, 0, &mMaxOutputFrames, &size);
		LOG_OSSTATUS_IF_FAILED(status, OSSTR("AudioUnitGetProperty(MixerAudioUnit, MaximumFramesPerSlice)"));

		size = sizeof(UInt32);
		status =
				::AudioUnitSetProperty(mMixerAudioUnit, kAudioUnitProperty_ElementCount, kAudioUnitScope_Input, 0,
						&mMaxAudioPlayers, size);
		LOG_OSSTATUS_IF_FAILED(status, OSSTR("AudioUnitSetProperty(MixerAudioUnit, ElementCount)"));

		status =
				::AudioUnitSetParameter(mMixerAudioUnit, kMultiChannelMixerParam_Volume, kAudioUnitScope_Output, 0, 1.0,
						0);
		LOG_OSSTATUS_IF_FAILED(status, OSSTR("AudioUnitSetParameter(MixerAudioUnit, Volume)"));

		// Connect Mixer Audio Unit to Output Audio Unit
		AudioUnitConnection	audioUnitConnection;
		audioUnitConnection.sourceAudioUnit = mMixerAudioUnit;
		audioUnitConnection.sourceOutputNumber = 0;
		audioUnitConnection.destInputNumber = 0;
		size = sizeof(AudioUnitConnection);
		status =
				::AudioUnitSetProperty(mOutputAudioUnit, kAudioUnitProperty_MakeConnection, kAudioUnitScope_Input, 0,
						&audioUnitConnection, size);
		LOG_OSSTATUS_IF_FAILED(status, OSSTR("AudioUnitSetProperty(Connecting MixerAudioUnit to OutputAudioUnit)"));

		size = sizeof(AudioStreamBasicDescription);
		status =
				::AudioUnitGetProperty(mMixerAudioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0,
						&mASBD, &size);
		LOG_OSSTATUS_IF_FAILED(status, OSSTR("AudioUnitGetProperty(MixerAudioUnit, StreamFormat)"));

		mASBD.mSampleRate = outputAudioUnitSampleRate;
		status =
				::AudioUnitSetProperty(mMixerAudioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0,
						&mASBD, size);
		LOG_OSSTATUS_IF_FAILED(status, OSSTR("AudioUnitSetProperty(MixerAudioUnit, StreamFormat)"));
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
			LOG_OSSTATUS_IF_FAILED(status, OSSTR("AudioUnitSetProperty(MixerAudioUnit, RenderCallback)"));

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
	LOG_OSSTATUS_IF_FAILED(status, OSSTR("AudioUnitSetParameter(MixerAudioUnit, Volume)"));
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioEngine::play()
//----------------------------------------------------------------------------------------------------------------------
{
	// Start
	OSStatus	status = ::AudioOutputUnitStart(mOutputAudioUnit);
	LOG_OSSTATUS_IF_FAILED(status, OSSTR("AudioOutputUnitStart()"));
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
	LOG_OSSTATUS_IF_FAILED(status, OSSTR("AudioUnitSetProperty(MixerAudioUnit, RenderCallback)"));

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
	LOG_OSSTATUS_IF_FAILED(status, OSSTR("AudioOutputUnitStop()"));
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioPlayerInternals

class CAudioPlayerInternals {
	public:
							CAudioPlayerInternals(CAudioPlayer& audioPlayer, const CString& identifier,
									const CAudioPlayer::Procs& procs) :
								mAudioPlayer(audioPlayer), mIdentifier(identifier), mProcs(procs),
										mIsPlaying(false), mStartTimeInterval(0.0), mGain(1.0),
										mRenderProcShouldSendFrames(false),
										mRenderProcIsSendingFrames(false), mRenderProcPreviousReadSize(0),
										mRenderProcPreviousFrameCount(0), mRenderProcFrameIndex(0)
								{}

		static	void		readerThreadError(const SError& error, void* userData)
								{
									// Setup
									CAudioPlayerInternals&	internals = *((CAudioPlayerInternals*) userData);

									// Call proc
									internals.mProcs.error(internals.mAudioPlayer, error);
								}

		static	OSStatus	renderProc(void* inRefCon, AudioUnitRenderActionFlags* inActionFlags,
									const AudioTimeStamp* inTimeStamp, UInt32 inBusNumber, UInt32 inNumFrames,
									AudioBufferList* ioData)
								{
									// Setup
									CAudioPlayerInternals&	internals = *((CAudioPlayerInternals*) inRefCon);

									// Check if rendered any frames from the buffer
									if (internals.mRenderProcPreviousReadSize > 0) {
										// Commit previous read
										internals.mQueue->commitRead(internals.mRenderProcPreviousReadSize);
										internals.mRenderProcPreviousReadSize = 0;
									}

									// Check if rendered any frames
									if (internals.mRenderProcPreviousFrameCount > 0) {
										// Update position
										internals.mRenderProcFrameIndex += internals.mRenderProcPreviousFrameCount;
										internals.mRenderProcPreviousFrameCount = 0;

										// Notify playback position updated
										UniversalTime	position =
																internals.mStartTimeInterval +
																		(Float32) internals.mRenderProcFrameIndex /
																				*internals.mSampleRate;
										internals.mProcs.positionUpdated(internals.mAudioPlayer, position);

										// Notify queue read complete
										internals.mAudioPlayerReaderThread->noteQueueReadComplete();
									}

									// Check if should send frames
									if (internals.mRenderProcShouldSendFrames) {
										// Sending frames
										CSRSWBIPSegmentedQueue::ReadBufferInfo	readBufferInfo =
																						internals.mQueue->requestRead();
										UInt32									requiredByteCount =
																						inNumFrames *
																								*internals.mBytesPerFrame;
										if (readBufferInfo.hasBuffer() && (readBufferInfo.mSize >= requiredByteCount)) {
											// Can point to buffer
											for (UInt32 i = 0; i < ioData->mNumberBuffers; i++)
												// Prepare this buffer
												ioData->mBuffers[i].mData = readBufferInfo.bufferAtIndex(i);

											// Store
											internals.mRenderProcIsSendingFrames = true;
											internals.mRenderProcPreviousReadSize = requiredByteCount;
											internals.mRenderProcPreviousFrameCount = inNumFrames;
										} else {
											// Must copy buffers
											UInt32	byteOffset = 0;
											while ((requiredByteCount > 0) && readBufferInfo.hasBuffer()) {
												// Must copy
												UInt32	bytesToCopy =
																std::min<UInt32>(requiredByteCount,
																		readBufferInfo.mSize);
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
											if (requiredByteCount > 0) {
												// Fill the rest with silence
												for (UInt32 i = 0; i < ioData->mNumberBuffers; i++)
													// Copy
													::bzero((UInt8*) ioData->mBuffers[i].mData + byteOffset,
															requiredByteCount);

												// Check if silence
												if (internals.mRenderProcPreviousFrameCount == 0)
													// Buffer is entirely silence
													*inActionFlags = kAudioUnitRenderAction_OutputIsSilence;

												// Check situation
												if ((internals.mRenderProcFrameIndex == 0) &&
														(internals.mRenderProcPreviousFrameCount == 0))
													// Was recently started, but the reader thread doesn't yet have
													//	frames queued.
													internals.mRenderProcIsSendingFrames = true;
												else if (!internals.mAudioPlayerReaderThread->getDidReachEndOfData()) {
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
													if (internals.mRenderProcIsSendingFrames)
														// Just reached the end of data
														internals.mProcs.endOfData(internals.mAudioPlayer);

													internals.mRenderProcIsSendingFrames = false;
												}
											} else
												// Update
												internals.mRenderProcIsSendingFrames = true;
										}
									} else {
										// Not sending frames
										for (UInt32 i = 0; i < ioData->mNumberBuffers; i++)
											// Clear
											::bzero(ioData->mBuffers[i].mData, ioData->mBuffers[i].mDataByteSize);
										*inActionFlags = kAudioUnitRenderAction_OutputIsSilence;

										internals.mRenderProcIsSendingFrames = false;
									}

									return noErr;
								}

		CAudioPlayer&					mAudioPlayer;
		CString							mIdentifier;
		CAudioPlayer::Procs				mProcs;

		OI<CAudioPlayerReaderThread>	mAudioPlayerReaderThread;
		OI<CSRSWBIPSegmentedQueue>		mQueue;
		OV<UInt32>						mAudioEngineIndex;
		OV<Float32>						mSampleRate;
		OV<UInt32>						mBytesPerFrame;

		bool							mIsPlaying;
		UniversalTimeInterval			mStartTimeInterval;
		OV<UniversalTimeInterval>		mDurationTimeInterval;
		Float32							mGain;

		bool							mRenderProcShouldSendFrames;
		bool							mRenderProcIsSendingFrames;
		UInt32							mRenderProcPreviousReadSize;
		UInt32							mRenderProcPreviousFrameCount;
		UInt32							mRenderProcFrameIndex;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioPlayer

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioPlayer::CAudioPlayer(const CString& identifier, const Procs& procs) : CAudioDestination()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CAudioPlayerInternals(*this, identifier, procs);
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
		CAudioEngine::mShared.removeAudioPlayer(mInternals->mAudioEngineIndex.getValue());

	// Stop threads
	if (mInternals->mAudioPlayerReaderThread.hasInstance())
		// Shutdown
		mInternals->mAudioPlayerReaderThread->shutdown();

	// Cleanup
	Delete(mInternals);
}

// MARK: CAudioProcessor methods

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudioProcessingSetup> CAudioPlayer::getInputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup if necessary
	static	SAudioProcessingSetup*	sAudioProcessingSetup = nil;
	if (sAudioProcessingSetup == nil) {
		// Compose SAudioProcessingSetup
		AudioStreamBasicDescription	asbd = CAudioEngine::mShared.getInputFormat();
		sAudioProcessingSetup =
				new SAudioProcessingSetup(asbd.mBitsPerChannel, asbd.mSampleRate,
						(EAudioChannelMap) asbd.mChannelsPerFrame,
						((asbd.mFormatFlags & kAudioFormatFlagIsFloat) != 0) ?
								SAudioProcessingSetup::SampleTypeOption::kSampleTypeFloat :
								SAudioProcessingSetup::SampleTypeOption::kSampleTypeSignedInteger,
						SAudioProcessingSetup::EndianOption::kEndianNative,
						(asbd.mFormatFlags & kAudioFormatFlagIsNonInterleaved) ?
								SAudioProcessingSetup::InterleavedOption::kNonInterleaved :
								SAudioProcessingSetup::InterleavedOption::kInterleaved);
	}

	return TNArray<SAudioProcessingSetup>(*sAudioProcessingSetup);
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CAudioPlayer::connectInput(const I<CAudioProcessor>& audioProcessor,
		const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt32	segmentCount;
	mInternals->mSampleRate = OV<Float32>(audioProcessingFormat.getSampleRate());
	if (audioProcessingFormat.getIsInterleaved()) {
		// Interleaved
		mInternals->mBytesPerFrame =
				OV<UInt32>(audioProcessingFormat.getBits() / 8 * audioProcessingFormat.getChannels());
		segmentCount = 1;
	} else {
		// Non-interleaved
		mInternals->mBytesPerFrame = OV<UInt32>(audioProcessingFormat.getBits() / 8);
		segmentCount = audioProcessingFormat.getChannels();
	}

	UInt32	frameCount = CAudioPlayer::getPlaybackBufferDuration() * audioProcessingFormat.getSampleRate();
	mInternals->mQueue =
			OI<CSRSWBIPSegmentedQueue>(
					new CSRSWBIPSegmentedQueue(frameCount * *mInternals->mBytesPerFrame, segmentCount));
	mInternals->mAudioPlayerReaderThread =
			OI<CAudioPlayerReaderThread>(
					new CAudioPlayerReaderThread(*this, *mInternals->mQueue, *mInternals->mBytesPerFrame,
							CAudioEngine::mShared.getMaxOutputFrames(), CAudioPlayerInternals::readerThreadError,
							mInternals));

	// Do super
	return CAudioProcessor::connectInput(audioProcessor, audioProcessingFormat);
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CAudioPlayer::reset()
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
		CAudioEngine::mShared.removeAudioPlayer(mInternals->mAudioEngineIndex.getValue());
		mInternals->mAudioEngineIndex = OV<UInt32>();
	}

	// No longer playing
	mInternals->mIsPlaying = false;

	// Reset Reader Thread
	mInternals->mAudioPlayerReaderThread->stopReading();

	// Reset buffer
	mInternals->mQueue->reset();

	// Setup to play requested frames
	mInternals->mAudioPlayerReaderThread->seek(mInternals->mStartTimeInterval,
			mInternals->mDurationTimeInterval.hasValue() ?
					(UInt32) (*mInternals->mDurationTimeInterval * *mInternals->mSampleRate) : ~0);

	// Reset the pipeline
	OI<SError>	error = CAudioDestination::reset();
	ReturnErrorIfError(error);

	// Reset frame index
	mInternals->mRenderProcFrameIndex = 0;

	// Resume
	mInternals->mAudioPlayerReaderThread->resume();

	return OI<SError>();
}

// MARK: CAudioDestination methods

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::setupComplete()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mAudioPlayerReaderThread->resume();
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

	// Check if attached
	if (mInternals->mAudioEngineIndex.hasValue())
		// Set gain
		CAudioEngine::mShared.setAudioPlayerGain(mInternals->mAudioEngineIndex.getValue(), gain);
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::play()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if have audio engine index
	if (!mInternals->mAudioEngineIndex.hasValue()) {
		// Add player
		mInternals->mAudioEngineIndex =
				CAudioEngine::mShared.addAudioPlayer(CAudioPlayerInternals::renderProc, mInternals);
		CAudioEngine::mShared.setAudioPlayerGain(mInternals->mAudioEngineIndex.getValue(), mInternals->mGain);
	}
	if (!mInternals->mAudioEngineIndex.hasValue())
		// No available slots
		return;

	// We are now playing
	mInternals->mIsPlaying = true;

	// Send frames
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
	// Stop sending frames
	mInternals->mRenderProcShouldSendFrames = false;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::seek(UniversalTimeInterval timeInterval, OV<UniversalTimeInterval> durationTimeInterval,
		bool playPreview)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mStartTimeInterval = timeInterval;
	mInternals->mDurationTimeInterval = durationTimeInterval;

	// Stop sending frames
	mInternals->mRenderProcShouldSendFrames = false;
	while (mInternals->mRenderProcIsSendingFrames)
		// Sleep
		CThread::sleepFor(0.001);

	// Reset Reader Thread
	mInternals->mAudioPlayerReaderThread->stopReading();

	// Reset buffer
	mInternals->mQueue->reset();

	// Reset the pipeline
	CAudioDestination::reset();

	// Reset frame index
	mInternals->mRenderProcFrameIndex = 0;

	// Check for preview
	if (playPreview) {
		// Setup to play preview length frames
		mInternals->mAudioPlayerReaderThread->seek(timeInterval,
				(UInt32) (timeInterval * CAudioPlayer::kPreviewDuration));

		// Send frames
		mInternals->mRenderProcShouldSendFrames = true;
	}
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

	// Reset Reader Thread
	mInternals->mAudioPlayerReaderThread->stopReading();

	// Reset buffer
	mInternals->mQueue->reset();

	// Setup to play requested frames
	mInternals->mAudioPlayerReaderThread->seek(mInternals->mStartTimeInterval,
			mInternals->mDurationTimeInterval.hasValue() ?
					(UInt32) (*mInternals->mDurationTimeInterval * *mInternals->mSampleRate) : ~0);

	// Reset the pipeline
	CAudioDestination::reset();

	// Reset frame index
	mInternals->mRenderProcFrameIndex = 0;

	// Resume
	mInternals->mAudioPlayerReaderThread->resume();

	// Begin sending frames again
	mInternals->mRenderProcShouldSendFrames = mInternals->mIsPlaying;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::setMaxAudioPlayers(UInt32 maxAudioPlayers)
//----------------------------------------------------------------------------------------------------------------------
{
	CAudioEngine::mShared.setMaxAudioPlayers(maxAudioPlayers);
}
