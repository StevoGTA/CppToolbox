//----------------------------------------------------------------------------------------------------------------------
//	CAudioPlayer-Apple.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioPlayer.h"

#include "CBits.h"
#include "CLogServices.h"
#include "ConcurrencyPrimitives.h"
#include "CQueue.h"
#include "CThread.h"
#include "SVersionInfo.h"

#import <AudioToolbox/AudioToolbox.h>

/*
	References:
		https://github.com/thestk/rtaudio
*/

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sErrorDomain(OSSTR("CAudioPlayer-Apple"));
static	SError	sUnableToLoadTracks(sErrorDomain, 1, CString(OSSTR("No available tracks")));

#define LOG_ERROR(method, status)																					\
				if (status != noErr) {																				\
					CLogServices::logError(CString(method) + CString(" returned OSStatus of ") + CString(status));	\
				}

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
		LOG_ERROR(CFSTR("AudioComponentInstanceNew(OutputAudioUnit)"), status);

		status = ::AudioUnitInitialize(mOutputAudioUnit);
		LOG_ERROR(CFSTR("AudioUnitInitialize(OutputAudioUnit)"), status);

		AudioStreamBasicDescription	asbd;
		UInt32						size = sizeof(AudioStreamBasicDescription);
		status =
				::AudioUnitGetProperty(mOutputAudioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0,
						&asbd, &size);
		LOG_ERROR(CFSTR("AudioUnitGetProperty(OutputAudioUnit, StreamFormat)"), status);
		Float32	outputAudioUnitSampleRate = asbd.mSampleRate;

		// Setup Mixer Audio Unit
		audioComponentDescription.componentType = kAudioUnitType_Mixer;
		audioComponentDescription.componentSubType =
				kAudioUnitSubType_MultiChannelMixer;
		audioComponent = ::AudioComponentFindNext(nil, &audioComponentDescription);

		status = ::AudioComponentInstanceNew(audioComponent, &mMixerAudioUnit);
		LOG_ERROR(CFSTR("AudioComponentInstanceNew(MixerAudioUnit)"), status);

		status = ::AudioUnitInitialize(mMixerAudioUnit);
		LOG_ERROR(CFSTR("AudioUnitInitialize(MixerAudioUnit)"), status);

		size = sizeof(UInt32);
		status =
				::AudioUnitGetProperty(mMixerAudioUnit, kAudioUnitProperty_MaximumFramesPerSlice,
						kAudioUnitScope_Global, 0, &mMaxOutputFrames, &size);
		LOG_ERROR(CFSTR("AudioUnitGetProperty(MixerAudioUnit, MaximumFramesPerSlice)"), status);

		size = sizeof(UInt32);
		status =
				::AudioUnitSetProperty(mMixerAudioUnit, kAudioUnitProperty_ElementCount, kAudioUnitScope_Input, 0,
						&mMaxAudioPlayers, size);
		LOG_ERROR(CFSTR("AudioUnitSetProperty(MixerAudioUnit, ElementCount)"), status);

		status =
				::AudioUnitSetParameter(mMixerAudioUnit, kMultiChannelMixerParam_Volume, kAudioUnitScope_Output, 0, 1.0,
						0);
		LOG_ERROR(CFSTR("AudioUnitSetParameter(MixerAudioUnit, Volume)"), status);

		// Connect Mixer Audio Unit to Output Audio Unit
		AudioUnitConnection	audioUnitConnection;
		audioUnitConnection.sourceAudioUnit = mMixerAudioUnit;
		audioUnitConnection.sourceOutputNumber = 0;
		audioUnitConnection.destInputNumber = 0;
		size = sizeof(AudioUnitConnection);
		status =
				::AudioUnitSetProperty(mOutputAudioUnit, kAudioUnitProperty_MakeConnection, kAudioUnitScope_Input, 0,
						&audioUnitConnection, size);
		LOG_ERROR(CFSTR("AudioUnitSetProperty(Connecting MixerAudioUnit to OutputAudioUnit)"), status);

		size = sizeof(AudioStreamBasicDescription);
		status =
				::AudioUnitGetProperty(mMixerAudioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0,
						&mASBD, &size);
		LOG_ERROR(CFSTR("AudioUnitGetProperty(MixerAudioUnit, StreamFormat)"), status);

		mASBD.mSampleRate = outputAudioUnitSampleRate;
		status =
				::AudioUnitSetProperty(mMixerAudioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0,
						&mASBD, size);
		LOG_ERROR(CFSTR("AudioUnitSetProperty(MixerAudioUnit, StreamFormat)"), status);
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
			LOG_ERROR(CFSTR("AudioUnitSetProperty(MixerAudioUnit, RenderCallback)"), status);

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
	LOG_ERROR(CFSTR("AudioUnitSetParameter(MixerAudioUnit, Volume)"), status);
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioEngine::play()
//----------------------------------------------------------------------------------------------------------------------
{
	// Start
	OSStatus	status = ::AudioOutputUnitStart(mOutputAudioUnit);
	LOG_ERROR(CFSTR("AudioOutputUnitStart()"), status);
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
	LOG_ERROR(CFSTR("AudioUnitSetProperty(MixerAudioUnit, RenderCallback)"), status);

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
	LOG_ERROR(CFSTR("AudioOutputUnitStop()"), status);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioPlayerReaderThread

class CAudioPlayerReaderThread : public CThread {
	public:
		enum State {
			kStateStarting,
			kStateWaiting,
			kStateReading,
		};

		typedef	void	(*ErrorProc)(const SError& error, void* userData);

				CAudioPlayerReaderThread(CAudioPlayer& audioPlayer, CSRSWBIPSegmentedQueue& queue,
						UInt32 bytesPerFrame, ErrorProc errorProc, void* procsUserData) :
					CThread(audioPlayer.getIdentifier()),
							mAudioPlayer(audioPlayer), mErrorProc(errorProc), mQueue(queue),
							mBytesPerFrame(bytesPerFrame), mProcsUserData(procsUserData),
							mResumeRequesed(false), mStopReadingRequested(false), mShutdownRequested(false),
							mReachedEndOfData(false), mState(kStateStarting),
							mMediaPosition(SMediaPosition::fromStart(0.0)), mFramesToRead(~0)
					{}

		void	run()
					{
						// Run until shutdown
						while (!mShutdownRequested) {
							// Check state
							switch (mState) {
								case kStateStarting:
									// Starting
									if (mResumeRequesed) {
										// Start reading
										mState = kStateReading;
										break;
									} else {
										// Wait for resume
										mState = kStateWaiting;

										// Fall through
									}

								case kStateWaiting:
									// Waiting
									mSemaphore.waitFor();

									// Check if reset requested
									if (!mStopReadingRequested)
										// Go for reading
										mState = kStateReading;
									break;

								case kStateReading:
									// Reading
									if (mStopReadingRequested || !tryRead())
										// Go to waiting
										mState = kStateWaiting;
									break;
							}
						}
					}

		bool	tryRead()
					{
						// Setup
// Not sure yet the best way to approach this...
//	A) Just use maxOutputFrameCount
//	B) Use some multiple of maxOutputFrameCount (like 4)
//	C) Use a constant like 4096
// -We want the first read to go fast enough that we have the initial audio data ASAP, so not too big
// -But we also want to be efficient with system resources, so not too small
						UInt32	framesToRead =
									std::min<UInt32>(CAudioEngine::mShared.getMaxOutputFrames() * 4, mFramesToRead);
						UInt32	bytesToRead = framesToRead * mBytesPerFrame;

						// Request write
						CSRSWBIPSegmentedQueue::SWriteBufferInfo	writeBufferInfo = mQueue.requestWrite(bytesToRead);
						if (!writeBufferInfo.hasBuffer())
							// No space
							return false;

						// Perform read
						CAudioData			audioData(writeBufferInfo.mBuffer, mQueue.getSegmentCount(),
													writeBufferInfo.mSegmentSize / mBytesPerFrame,
													writeBufferInfo.mSize / mBytesPerFrame, mBytesPerFrame);
						SAudioReadStatus	audioReadStatus = mAudioPlayer.perform(mMediaPosition, audioData);
						if (audioReadStatus.isSuccess()) {
							// Success
							mQueue.commitWrite(audioData.getCurrentFrameCount() * mBytesPerFrame);
							mMediaPosition = SMediaPosition::fromCurrent();
							mFramesToRead -= audioData.getCurrentFrameCount();

							return true;
						} else {
							// Finished
							mReachedEndOfData = true;
							if (*audioReadStatus.getError() != SError::mEndOfData)
								// Error
								mErrorProc(*audioReadStatus.getError(), mProcsUserData);

							return false;
						}
					}
		void	noteQueueReadComplete()
					{
						// Signal
						mSemaphore.signal();
					}
		void	seek(UniversalTimeInterval timeInterval, UInt32 maxFrames)
					{
						// Update
						mMediaPosition = SMediaPosition::fromStart(timeInterval);
						mFramesToRead = maxFrames;
						mReachedEndOfData = false;
					}
		void	resume()
					{
						// Update
						mResumeRequesed = true;
						mStopReadingRequested = false;

						// Signal
						mSemaphore.signal();
					}
		void	stopReading()
					{
						// Request reset
						mStopReadingRequested = true;

						// Wait until waiting
						while (mState != kStateWaiting)
							// Sleep
							CThread::sleepFor(0.001);
					}
		void	shutdown()
					{
						// Request shutdown
						mShutdownRequested = true;

						// Signal if waiting
						mSemaphore.signal();

						// Wait until is no lonnger running
						while (getIsRunning())
							// Sleep
							CThread::sleepFor(0.001);
					}

		CAudioPlayer&			mAudioPlayer;
		ErrorProc				mErrorProc;
		CSemaphore				mSemaphore;
		CSRSWBIPSegmentedQueue&	mQueue;
		UInt32					mBytesPerFrame;
		void*					mProcsUserData;

		bool					mResumeRequesed;
		bool					mStopReadingRequested;
		bool					mShutdownRequested;
		bool					mReachedEndOfData;
		State					mState;
		SMediaPosition			mMediaPosition;
		UInt32					mFramesToRead;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioPlayerInternals

class CAudioPlayerInternals {
	public:
							CAudioPlayerInternals(const CString& identifier,
									const CAudioPlayer::SAudioPlayerProcs& audioPlayerProcs) :
								mIdentifier(identifier), mAudioPlayerProcs(audioPlayerProcs),
										mIsPlaying(false), mStartTimeInterval(0.0), mGain(1.0),
										mRenderProcShouldSendFrames(false),
										mRenderProcShouldStopSendingFramesAtEndOfData(false),
										mRenderProcIsSendingFrames(false), mRenderProcPreviousReadSize(0),
										mRenderProcPreviousFrameCount(0), mRenderProcFrameIndex(0)
								{}

		static	void		readerThreadError(const SError& error, void* userData)
								{
									// Setup
									CAudioPlayerInternals&	internals = *((CAudioPlayerInternals*) userData);

									// Call proc
									internals.mAudioPlayerProcs.error(error);
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

										// Notify player
										CThread::runOnMainThread(renderProcPositionUpdated, &internals);

										// Inform the reader thread
										internals.mAudioPlayerReaderThread->noteQueueReadComplete();
									}

									// Check if should send frames
									if (internals.mRenderProcShouldSendFrames) {
										// Sending frames
										CSRSWBIPSegmentedQueue::SReadBufferInfo readBufferInfo =
																						internals.mQueue->requestRead();
										UInt32	requiredByteCount = inNumFrames * *internals.mBytesPerFrame;
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
												// Still need to add more frames
												for (UInt32 i = 0; i < ioData->mNumberBuffers; i++)
													// Copy
													::bzero((UInt8*) ioData->mBuffers[i].mData + byteOffset,
															requiredByteCount);

												// Check if at the end
												if (internals.mAudioPlayerReaderThread->mReachedEndOfData) {
													// End of data
													internals.mRenderProcIsSendingFrames = false;

													// Notify player
													CThread::runOnMainThread(renderProcEndOfData, &internals);
												} else if ((internals.mRenderProcFrameIndex > 0) &&
														(internals.mRenderProcPreviousFrameCount > 0)) {
													// We ran out of data...  we will glitch
													CLogServices::logError(
															CString(OSSTR("CAudioPlayer ")) + internals.mIdentifier +
																	CString(OSSTR(" requires ")) +
																	CString(inNumFrames) +
																	CString(OSSTR(" frames, but is ")) +
																	CString(inNumFrames -
																			internals.mRenderProcPreviousFrameCount) +
																	CString(OSSTR(" frames short, will glitch...")));

													// Update
													internals.mRenderProcIsSendingFrames = true;
												} else
													// Just getting started.  We are asked for frames before the reader
													//	has them ready initially.  All good.
													*inActionFlags = kAudioUnitRenderAction_OutputIsSilence;
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
		static	void		renderProcPositionUpdated(void* userData)
								{
									// Setup
									CAudioPlayerInternals&	internals = *((CAudioPlayerInternals*) userData);

									// Check if still around
									if (mActiveInternals.contains(internals))
										// Call proc
										internals.mAudioPlayerProcs.positionUpdated(
												internals.mStartTimeInterval +
														(Float32) internals.mRenderProcFrameIndex /
																*internals.mSampleRate);
								}
		static	void		renderProcEndOfData(void* userData)
								{
									// Setup
									CAudioPlayerInternals&	internals = *((CAudioPlayerInternals*) userData);

									// Update
									internals.mIsPlaying = false;
									internals.mRenderProcShouldSendFrames = false;

									// Call proc
									internals.mAudioPlayerProcs.endOfData();
								}

				CString							mIdentifier;
				CAudioPlayer::SAudioPlayerProcs	mAudioPlayerProcs;

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
				bool							mRenderProcShouldStopSendingFramesAtEndOfData;
				bool							mRenderProcIsSendingFrames;
				UInt32							mRenderProcPreviousReadSize;
				UInt32							mRenderProcPreviousFrameCount;
				UInt32							mRenderProcFrameIndex;

		static	TIArray<CAudioPlayerInternals>	mActiveInternals;
};

TIArray<CAudioPlayerInternals>	CAudioPlayerInternals::mActiveInternals;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioPlayer

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioPlayer::CAudioPlayer(const CString& identifier, const SAudioPlayerProcs& audioPlayerProcs) : CAudioDestination()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CAudioPlayerInternals(identifier, audioPlayerProcs);
	CAudioPlayerInternals::mActiveInternals += mInternals;
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

	// Stop Audio Player Reader Thread
	if (mInternals->mAudioPlayerReaderThread.hasInstance())
		// Shutdown
		mInternals->mAudioPlayerReaderThread->shutdown();

	// Cleanup
	CAudioPlayerInternals::mActiveInternals -= *mInternals;
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
							CAudioPlayerInternals::readerThreadError, mInternals));

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

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::logInfo()
//----------------------------------------------------------------------------------------------------------------------
{
#if TARGET_OS_MACOS
	// Setup
	OSStatus	status;
	UInt32		count;

	// Collect info on default Audio Objects
	AudioObjectPropertyAddress	propertyAddress;
	UInt32						dataSize;

	propertyAddress.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
	propertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
	propertyAddress.mElement = kAudioObjectPropertyElementMaster;

	AudioDeviceID	defaultOutputAudioDeviceID;
	dataSize = sizeof(AudioDeviceID);
	status =
			::AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, nil, &dataSize,
					&defaultOutputAudioDeviceID);


	propertyAddress.mSelector = kAudioHardwarePropertyDefaultSystemOutputDevice;
	propertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
	propertyAddress.mElement = kAudioObjectPropertyElementMaster;

	AudioDeviceID	defaultSystemOutputAudioDeviceID;
	dataSize = sizeof(AudioDeviceID);
	status =
			::AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, nil, &dataSize,
					&defaultSystemOutputAudioDeviceID);

	// Log Audio Objects
	propertyAddress.mSelector = kAudioHardwarePropertyDevices;
	propertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
	propertyAddress.mElement = kAudioObjectPropertyElementMaster;

	status = ::AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &propertyAddress, 0, nil, &dataSize);
	count = dataSize / sizeof(AudioDeviceID);

	AudioDeviceID	deviceIDs[count];
	status = ::AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, nil, &dataSize, &deviceIDs);

	CLogServices::logMessage(
			CString(OSSTR("Found the following Audio Objects (")) + CString(count) + CString(OSSTR("): ")));
	for (UInt32 i = 0; i < count; i++) {
		// Collect info on this Audio Object
		CFStringRef	nameStringRef;
		dataSize = sizeof(CFStringRef);
		propertyAddress.mSelector = kAudioObjectPropertyName;
		status = ::AudioObjectGetPropertyData(deviceIDs[i], &propertyAddress, 0, nil, &dataSize, &nameStringRef);

		CFStringRef	manufacturerStringRef;
		dataSize = sizeof(CFStringRef);
		propertyAddress.mSelector = kAudioObjectPropertyManufacturer;
		status =
				::AudioObjectGetPropertyData(deviceIDs[i], &propertyAddress, 0, nil, &dataSize, &manufacturerStringRef);

		CFStringRef	modelNameStringRef;
		dataSize = sizeof(CFStringRef);
		propertyAddress.mSelector = kAudioObjectPropertyModelName;
		status = ::AudioObjectGetPropertyData(deviceIDs[i], &propertyAddress, 0, nil, &dataSize, &modelNameStringRef);

		CFStringRef	firmwareVersionStringRef;
		dataSize = sizeof(CFStringRef);
		propertyAddress.mSelector = kAudioObjectPropertyFirmwareVersion;
		status =
				::AudioObjectGetPropertyData(deviceIDs[i], &propertyAddress, 0, nil, &dataSize,
						&firmwareVersionStringRef);

		CFStringRef	serialNumberStringRef;
		dataSize = sizeof(CFStringRef);
		propertyAddress.mSelector = kAudioObjectPropertySerialNumber;
		status =
				::AudioObjectGetPropertyData(deviceIDs[i], &propertyAddress, 0, nil, &dataSize, &serialNumberStringRef);

		CString	infoString = CString(nameStringRef) + CString(OSSTR(", ")) + CString(manufacturerStringRef);
		if (modelNameStringRef != nil)
			infoString += CString(OSSTR(", Model: ")) + CString(modelNameStringRef);
		if (firmwareVersionStringRef != nil)
			infoString += CString(OSSTR(", Firmware Version: ")) + CString(firmwareVersionStringRef);
		if (serialNumberStringRef != nil)
			infoString += CString(OSSTR(", Serial Number: ")) + CString(serialNumberStringRef);
		if (defaultOutputAudioDeviceID == deviceIDs[i])
			infoString += CString(OSSTR(", Default Output Device"));
		if (defaultSystemOutputAudioDeviceID == deviceIDs[i])
			infoString += CString(OSSTR(", Default System Output Device"));
		CLogServices::logMessage(
				CString(OSSTR("    ")) + infoString + CString(OSSTR(", with the following streams:")));

		// Log info on the input streams for this Audio Object
		propertyAddress.mSelector = kAudioDevicePropertyStreamConfiguration;
		propertyAddress.mScope = kAudioDevicePropertyScopeInput;
		propertyAddress.mElement = kAudioObjectPropertyElementMaster;
		::AudioObjectGetPropertyDataSize(deviceIDs[i], &propertyAddress, 0, nil, &dataSize);

		AudioBufferList*	audioBufferList = (AudioBufferList*) ::malloc(dataSize);
		::AudioObjectGetPropertyData(deviceIDs[i], &propertyAddress, 0, nil, &dataSize, audioBufferList);

		for (UInt32 j = 0; j < audioBufferList->mNumberBuffers; j++)
			//
			CLogServices::logMessage(
					CString(OSSTR("        Stream ")) + CString(j) + CString(OSSTR(": ")) +
							CString(audioBufferList->mBuffers[j].mNumberChannels) +
							CString(OSSTR(" input channel(s)")));
		::free(audioBufferList);

		// Log info on the output streams for this Audio Object
		propertyAddress.mSelector = kAudioDevicePropertyStreamConfiguration;
		propertyAddress.mScope = kAudioDevicePropertyScopeOutput;
		propertyAddress.mElement = kAudioObjectPropertyElementMaster;
		::AudioObjectGetPropertyDataSize(deviceIDs[i], &propertyAddress, 0, nil, &dataSize);

		audioBufferList = (AudioBufferList*) ::malloc(dataSize);
		::AudioObjectGetPropertyData(deviceIDs[i], &propertyAddress, 0, nil, &dataSize, audioBufferList);

		for (UInt32 j = 0; j < audioBufferList->mNumberBuffers; j++)
			//
			CLogServices::logMessage(
					CString(OSSTR("        Stream ")) + CString(j) + CString(OSSTR(": ")) +
							CString(audioBufferList->mBuffers[j].mNumberChannels) +
							CString(OSSTR(" output channel(s)")));
		::free(audioBufferList);
	}

	// Log info on output Audio Components
	AudioComponentDescription	desc = {0};
	desc.componentType = kAudioUnitType_Output;

	count = ::AudioComponentCount(&desc);
	CLogServices::logMessage(
			CString(OSSTR("Found the following Output Units (")) + CString(count) + CString(OSSTR("): ")));

	AudioComponent	outputUnitComponent = nil;
	do {
		outputUnitComponent = ::AudioComponentFindNext(outputUnitComponent, &desc);
		if (outputUnitComponent != nil) {
			// Get info on this Audio Component
			CFStringRef	stringRef;
			status = ::AudioComponentCopyName(outputUnitComponent, &stringRef);

			AudioComponentDescription	fullDesc;
			status = ::AudioComponentGetDescription(outputUnitComponent, &fullDesc);

			UInt32	version;
			status = ::AudioComponentGetVersion(outputUnitComponent, &version);
			SVersionInfo	versionInfo(version);

			CFDictionaryRef	dictionaryRef;
			status = ::AudioComponentCopyConfigurationInfo(outputUnitComponent, &dictionaryRef);

			CLogServices::logMessage(
					CString(OSSTR("    ")) + CString(stringRef) +
							CString(OSSTR(" v")) + versionInfo.getString() +
							CString(OSSTR(" (")) + CString(fullDesc.componentType, true) +
									CString(OSSTR(", ")) + CString(fullDesc.componentSubType, true) +
									CString(OSSTR(", ")) + CString(fullDesc.componentManufacturer, true) +
									CString(OSSTR(")")));
		}
	} while (outputUnitComponent != nil);
#endif
}
