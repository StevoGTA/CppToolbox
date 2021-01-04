//----------------------------------------------------------------------------------------------------------------------
//	CAudioPlayer-Windows.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioPlayer.h"

#include "CLogServices.h"
#include "SError-Windows.h"
#include "TMFAsyncCallback.h"

#undef Delete

#include <mfapi.h>
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <wrl\implements.h>

#pragma comment( lib, "mfuuid.lib" )

#define Delete(x)	{ delete x; x = nil; }

using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::Media::Devices;

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioPlayerImplementation

class CAudioPlayerImplementation :
		public RuntimeClass<RuntimeClassFlags<ClassicCom>, FtmBase, IActivateAudioInterfaceCompletionHandler> {
	public:
		enum State {
			kInitializing,
			kInitialized,
			kPlaybackStarting,
			kPlaying,
			kPausing,
			kPaused,

			kError,
		};

						CAudioPlayerImplementation(CAudioPlayer& audioPlayer, const CString& identifier,
								const CAudioPlayer::Procs& procs) :
							mAudioPlayer(audioPlayer), mIdentifier(identifier), mProcs(procs),
									mFinishSeekShouldPause(false), mAudioClient(nullptr), mAudioRenderClient(nullptr),
									mFillBufferAsyncResult(nullptr), mSimpleAudioVolume(nullptr), mFillBufferKey(0),
									mState(kInitializing), mFillBufferAsyncCallback(*this, onFillBuffer),
									mPlayAsyncCallback(*this, onPlay), mPauseAsyncCallback(*this, onPause),
									mBufferFrames(0), mDefaultPeriodInFrames(0), mFundamentalPeriodInFrames(0),
									mMaxPeriodInFrames(0), mMinPeriodInFrames(0),  mMixFormat(nullptr),
									mStartTimeInterval(0.0),
									mOnFillBufferShouldSendFrames(true), mOnFillBufferIsSendingFrames(false),
									mOnFillBufferPreviousFrameCount(0), mOnFillBufferFrameIndex(0)
							{
								// Setup
								HRESULT	result;

								// Create sample ready event
								mFillBufferEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
								if (mFillBufferEvent == nullptr)
									// Error
									processWindowsError(OSSTR("CreateEventEx() for sample ready event"));

								// Startup Media Foundation
								result = MFStartup(MF_VERSION, MFSTARTUP_LITE);
								processHRESULT(result, OSSTR("MFStartup()"));

								// Get Audio Render ID
								String^	audioRenderID = MediaDevice::GetDefaultAudioRenderId(AudioDeviceRole::Default);

								if (mState != kError) {
									// Activate Audio Interface
									IActivateAudioInterfaceAsyncOperation*	activateAudioInterfaceAsyncOperation;
									result =
											ActivateAudioInterfaceAsync(audioRenderID->Data(), __uuidof(IAudioClient3),
													nullptr, this, &activateAudioInterfaceAsyncOperation);
									processHRESULT(result, OSSTR("ActivateAudioInterfaceAsync"));
									if (activateAudioInterfaceAsyncOperation != nullptr)
										// Release
										activateAudioInterfaceAsyncOperation->Release();
								}
							}

				HRESULT	ActivateCompleted(IActivateAudioInterfaceAsyncOperation* activateAudioInterfaceAsyncOperation)
							{
								// Check state
								if (mState == kError)
									return S_OK;

								// Setup
								HRESULT	result;

								// Check activation result
								HRESULT		activationResult;
								IUnknown*	audioInterface;
								result =
										activateAudioInterfaceAsyncOperation->GetActivateResult(&activationResult,
												&audioInterface);
								if (SUCCEEDED(result) && SUCCEEDED(activationResult)) {
									// Get the Audio Client
									audioInterface->QueryInterface(IID_PPV_ARGS(&mAudioClient));

									// Configure
									AudioClientProperties	audioClientProperties = {0};
									audioClientProperties.cbSize = sizeof(AudioClientProperties);
									audioClientProperties.eCategory = AudioCategory_Media;
									result = mAudioClient->SetClientProperties(&audioClientProperties);
									processHRESULT(result, OSSTR("SetClientProperties("));

									if (mState != kError) {
										// Get Mix Format
										result = mAudioClient->GetMixFormat(&mMixFormat);
										processHRESULT(result, OSSTR("GetMixFormat()"));
									}

									if (mState != kError) {
										// Get periods
										result =
												mAudioClient->GetSharedModeEnginePeriod(mMixFormat,
														&mDefaultPeriodInFrames, &mFundamentalPeriodInFrames,
														&mMinPeriodInFrames, &mMaxPeriodInFrames);
										processHRESULT(result, OSSTR("GetSharedModeEnginePeriod()"));
									}

									if (mState != kError) {
										// Initialize
										result =
												mAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
														AUDCLNT_STREAMFLAGS_EVENTCALLBACK |
																AUDCLNT_STREAMFLAGS_NOPERSIST,
														0, 0, mMixFormat, nullptr);
										processHRESULT(result, OSSTR("Initialize()"));
									}

									if (mState != kError) {
										// Get the buffer size
										result = mAudioClient->GetBufferSize(&mBufferFrames);
										processHRESULT(result, OSSTR("GetBufferSize()"));
									}

									if (mState != kError) {
										// Get the Render Client
										result =
												mAudioClient->GetService(__uuidof(IAudioRenderClient),
														(void**) &mAudioRenderClient);
										processHRESULT(result, OSSTR("GetService() for IAudioRenderClient"));
									}

									if (mState != kError) {
										// Create async callback for sample events
										result =
												MFCreateAsyncResult(nullptr, &mFillBufferAsyncCallback, nullptr,
														&mFillBufferAsyncResult);
										processHRESULT(result, OSSTR("MFCreateAsyncResult()"));
									}

									if (mState != kError) {
										// Set Event Handler
										result = mAudioClient->SetEventHandle(mFillBufferEvent);
										processHRESULT(result, OSSTR("SetEventHandle()"));
									}

									if (mState != kError) {
										// Get Simple Audio Volume
										result =
												mAudioClient->GetService(__uuidof(ISimpleAudioVolume),
														(void**) &mSimpleAudioVolume);
										processHRESULT(result, OSSTR("GetService() for ISimpleAudioVolume"));
									}

									// Done
									if (mState != kError)
										mState = kInitialized;
								} else if (FAILED(activationResult))
									// Activation failed
									processHRESULT(activationResult, OSSTR("checking status in ActivateCompleted()"));
								else
									// Query failed
									processHRESULT(result, OSSTR("GetActivateResult()"));

								return S_OK;
							}

				HRESULT	onFillBuffer(__RPC__in_opt IMFAsyncResult& asyncResult)
							{
								// Check state
								if (mState == kError)
									return S_OK;

								// Setup
								HRESULT	result;

								// Check if rendered any frames
								if (mOnFillBufferPreviousFrameCount > 0) {
									// Update position
									mOnFillBufferFrameIndex += mOnFillBufferPreviousFrameCount;
									mOnFillBufferPreviousFrameCount = 0;

									// Notify playback position updated
									UniversalTime	position =
															mStartTimeInterval +
																	(Float32) mOnFillBufferFrameIndex /
																			mMixFormat->nSamplesPerSec;
									//mProcs.positionUpdated(mAudioPlayer, position);

									// Notify queue read complete
									mAudioPlayerReaderThread->noteQueueReadComplete();
								}

								// Get current padding
								UINT32	paddingFrames;
								result = mAudioClient->GetCurrentPadding(&paddingFrames);
								processHRESULT(result, OSSTR("GetCurrentPadding()"));

								// Figure frames available
								UINT32	framesAvailable = SUCCEEDED(result) ? mBufferFrames - paddingFrames : 0;

								// Get buffer
								BYTE*	data;
								result = mAudioRenderClient->GetBuffer(framesAvailable, &data);
								processHRESULT(result, OSSTR("GetBuffer()"));

								// Check if should send frames
								if ((mState == kPlaying) && mOnFillBufferShouldSendFrames) {
									// Sending frames
									CSRSWBIPSegmentedQueue::ReadBufferInfo	readBufferInfo = mQueue->requestRead();
									UInt32									requiredByteCount =
																					framesAvailable *
																							mMixFormat->nBlockAlign;

									// Copy frames
									UInt32	byteOffset = 0;
									while ((requiredByteCount > 0) && readBufferInfo.hasBuffer()) {
										// Must copy
										UInt32	bytesToCopy = std::min<UInt32>(requiredByteCount, readBufferInfo.mSize);
										::memcpy(data + byteOffset, readBufferInfo.bufferAtIndex(0), bytesToCopy);
										mQueue->commitRead(bytesToCopy);

										// Update info
										byteOffset += bytesToCopy;
										requiredByteCount -= bytesToCopy;
										mOnFillBufferPreviousFrameCount += bytesToCopy / mMixFormat->nBlockAlign;

										// Get next read buffer info
										readBufferInfo = mQueue->requestRead();
									}

									// Commit frames
//CLogServices::logMessage(mIdentifier + CString(OSSTR(": Filling playback buffer with ")) + CString(mOnFillBufferPreviousFrameCount) + CString(OSSTR(" frames.")));
									result = mAudioRenderClient->ReleaseBuffer(mOnFillBufferPreviousFrameCount, 0);
									processHRESULT(result, OSSTR("ReleaseBuffer()"));

									// Check situation
									if (requiredByteCount > 0) {
										// Check situation
										if ((mOnFillBufferFrameIndex == 0) && (mOnFillBufferPreviousFrameCount == 0))
											// Was recently started, but the reader thread doesn't yet have frames
											//	queued.
											mOnFillBufferIsSendingFrames = true;
										else if (!mAudioPlayerReaderThread->getDidReachEndOfData() &&
												(mOnFillBufferPreviousFrameCount == 0)) {
											// Have a discontinuity in the queued frames
											CLogServices::logError(
													CString(OSSTR("CAudioPlayer ")) + mIdentifier +
															CString(OSSTR(" requires frames but queue is empty, will glitch...")));

											mOnFillBufferIsSendingFrames = true;
										} else if (mOnFillBufferPreviousFrameCount > 0)
											// At the end, but have queued frames
											mOnFillBufferIsSendingFrames = true;
										else {
											// At the end, did not queue any frames
											if (mOnFillBufferIsSendingFrames)
												// Just reached the end of data
												mProcs.endOfData(mAudioPlayer);

											mOnFillBufferIsSendingFrames = false;
										}
									} else
										// Update
										mOnFillBufferIsSendingFrames = true;
								} else {
									// Not sending frames
									result =
											mAudioRenderClient->ReleaseBuffer(framesAvailable,
													AUDCLNT_BUFFERFLAGS_SILENT);
									processHRESULT(result, OSSTR("ReleaseBuffer()"));

									// Update
									mOnFillBufferIsSendingFrames = false;
								}

								// Check if need to queue another call
								if (mState == kPlaying) {
									// Put Waiting Work Item
									result =
											MFPutWaitingWorkItem(mFillBufferEvent, 0, mFillBufferAsyncResult,
													&mFillBufferKey);
									processHRESULT(result, OSSTR("MFPutWaitingWorkItem() for FillBuffer"));
								}

								return S_OK;
							}
				void	play()
							{
								// Check state
								if ((mState != kInitialized) && (mState != kPaused))
									return;

								// Update state
								mState = kPlaybackStarting;

								// Start
								HRESULT	result =
												MFPutWorkItem2(MFASYNC_CALLBACK_QUEUE_MULTITHREADED, 0,
														&mPlayAsyncCallback, nullptr);
								processHRESULT(result, OSSTR("MFPutWorkItem2() in start()"));

								// Wait
								while (mState == kPlaybackStarting)
									// Sleep
									CThread::sleepFor(0.001);
							}
				HRESULT	onPlay(__RPC__in_opt IMFAsyncResult& asyncResult)
							{
								// Check state
								if (mState != kPlaybackStarting)
									return S_OK;

								// Start
								HRESULT	result = mAudioClient->Start();
								processHRESULT(result, OSSTR("Start()"));

								// Update
								if (mState != kError) {
									// Put Waiting Work Item
									result =
											MFPutWaitingWorkItem(mFillBufferEvent, 0, mFillBufferAsyncResult,
													&mFillBufferKey);
									processHRESULT(result, OSSTR("MFPutWaitingWorkItem() for FillBuffer"));
								}

								if (mState != kError)
									// Success
									mState = kPlaying;

								return S_OK;
							}
				void	pause()
							{
								// Check state
								if (mState != kPlaying)
									return;

								// Update state
								mState = kPausing;

								// Pause
								HRESULT	result =
												MFPutWorkItem2(MFASYNC_CALLBACK_QUEUE_MULTITHREADED, 0,
														&mPauseAsyncCallback, nullptr);
								processHRESULT(result, OSSTR("MFPutWaitingWorkItem() for PausePlayback"));

								// Wait
								while (mState == kPausing)
									// Sleep
									CThread::sleepFor(0.001);
							}
				HRESULT	onPause(__RPC__in_opt IMFAsyncResult& asyncResult)
							{
								// Check state
								if (mState != kPausing)
									return S_OK;

								// Stop
								HRESULT	result = mAudioClient->Stop();
								if (SUCCEEDED(result))
									// Success
									mState = kPaused;
								else
									// Error
									processHRESULT(result, OSSTR("Stop()"));

								return S_OK;
							}
				void	setGain(Float32 gain)
							{
								// Check state
								if (mState == kError)
									return;

								// Set gain
								HRESULT	result = mSimpleAudioVolume->SetMasterVolume(gain, nullptr);
								processHRESULT(result, OSSTR("SetMasterVolume()"));
							}
				void	shutdown()
							{
								// Cleanup
								if (mFillBufferEvent != nullptr)
									CloseHandle(mFillBufferEvent);

								MFShutdown();
							}

				void	processHRESULT(HRESULT result, OSStringType method)
							{
								// Check result
								if (FAILED(result)) {
									// Error
									mState = kError;
									mError = OI<SError>(SErrorFromHRESULT(result));
									CLogServices::logError(
											CString(method) + CString(OSSTR(" returned ")) + mError->getDescription());
								}
							}
				void	processWindowsError(OSStringType method)
							{
								// Error
								mState = kError;
								mError = OI<SError>(SErrorFromWindowsError(GetLastError()));
								CLogServices::logError(
										CString(method) + CString(OSSTR(" returned ")) + mError->getDescription());
							}

		static	HRESULT	onFillBuffer(__RPC__in_opt IMFAsyncResult& asyncResult,
								CAudioPlayerImplementation& implementation)
							{ return implementation.onFillBuffer(asyncResult); }
		static	HRESULT	onPlay(__RPC__in_opt IMFAsyncResult& asyncResult, CAudioPlayerImplementation& implementation)
							{ return implementation.onPlay(asyncResult); } 
		static	HRESULT	onPause(__RPC__in_opt IMFAsyncResult& asyncResult, CAudioPlayerImplementation& implementation)
							{ return implementation.onPause(asyncResult); }
 
		CAudioPlayer&									mAudioPlayer;
		CString											mIdentifier;
		CAudioPlayer::Procs								mProcs;

		bool											mFinishSeekShouldPause;
		HANDLE											mFillBufferEvent;
		IAudioClient3*									mAudioClient;
		IAudioRenderClient*								mAudioRenderClient;
		IMFAsyncResult*									mFillBufferAsyncResult;
		ISimpleAudioVolume*								mSimpleAudioVolume;
		MFWORKITEM_KEY									mFillBufferKey;
		OI<CAudioPlayerReaderThread>					mAudioPlayerReaderThread;
		OI<CSRSWBIPSegmentedQueue>						mQueue;
		OI<SError>										mError;
		State											mState;
		TMFAsyncCallback<CAudioPlayerImplementation>	mFillBufferAsyncCallback;
		TMFAsyncCallback<CAudioPlayerImplementation>	mPlayAsyncCallback;
		TMFAsyncCallback<CAudioPlayerImplementation>	mPauseAsyncCallback;
		UINT32											mBufferFrames;
		UINT32											mDefaultPeriodInFrames;
		UINT32											mFundamentalPeriodInFrames;
		UINT32											mMaxPeriodInFrames;
		UINT32											mMinPeriodInFrames;
		WAVEFORMATEX*									mMixFormat;

		UniversalTimeInterval							mStartTimeInterval;
		OV<UniversalTimeInterval>						mDurationTimeInterval;

		bool											mOnFillBufferShouldSendFrames;
		bool											mOnFillBufferIsSendingFrames;
		UInt32											mOnFillBufferPreviousFrameCount;
		UInt32											mOnFillBufferFrameIndex;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioPlayerInternals

class CAudioPlayerInternals {
	public:
						CAudioPlayerInternals(CAudioPlayer& audioPlayer, const CString& identifier,
									const CAudioPlayer::Procs& procs) :
							mAudioPlayer(audioPlayer), mProcs(procs),
									mImplementation(Make<CAudioPlayerImplementation>(audioPlayer, identifier, procs))
							{}
						~CAudioPlayerInternals()
							{
								mImplementation->shutdown();
							}

		static	void	readerThreadError(const SError& error, void* userData)
							{
								// Setup
								CAudioPlayerInternals&	internals = *((CAudioPlayerInternals*) userData);

								// Call proc
								internals.mProcs.error(internals.mAudioPlayer, error);
							}

		CAudioPlayer&						mAudioPlayer;
		CAudioPlayer::Procs					mProcs;
		ComPtr<CAudioPlayerImplementation>	mImplementation;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioProcessor

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
	mInternals->mImplementation->pause();
	while (mInternals->mImplementation->mOnFillBufferIsSendingFrames)
		// Sleep
		CThread::sleepFor(0.001);

	// Stop threads
	if (mInternals->mImplementation->mAudioPlayerReaderThread.hasInstance())
		// Shutdown
		mInternals->mImplementation->mAudioPlayerReaderThread->shutdown();

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
		// Wait until initialized
		while (mInternals->mImplementation->mState == CAudioPlayerImplementation::kInitializing)
			// Sleep
			CThread::sleepFor(0.001);

		if (!mInternals->mImplementation->mError.hasInstance()) {
			// Compose SAudioProcessingSetup
			WAVEFORMATEX&	format = *mInternals->mImplementation->mMixFormat;
			sAudioProcessingSetup =
					new SAudioProcessingSetup((UInt8) format.wBitsPerSample, (Float32) format.nSamplesPerSec,
							(EAudioChannelMap) format.nChannels,
							SAudioProcessingSetup::SampleTypeOption::kSampleTypeFloat,
							SAudioProcessingSetup::EndianOption::kEndianNative,
							SAudioProcessingSetup::InterleavedOption::kInterleaved);
		}
	}

	return (sAudioProcessingSetup != nullptr) ?
			TNArray<SAudioProcessingSetup>(*sAudioProcessingSetup) : TNArray<SAudioProcessingSetup>();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CAudioPlayer::connectInput(const I<CAudioProcessor>& audioProcessor,
		const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Wait until initialized
	while (mInternals->mImplementation->mState == CAudioPlayerImplementation::kInitializing)
		// Sleep
		CThread::sleepFor(0.001);

	// Setup
	UInt32	frameCount = (UInt32) (CAudioPlayer::getPlaybackBufferDuration() * audioProcessingFormat.getSampleRate());
	mInternals->mImplementation->mQueue =
			OI<CSRSWBIPSegmentedQueue>(
					new CSRSWBIPSegmentedQueue(frameCount * mInternals->mImplementation->mMixFormat->nBlockAlign, 1));
	mInternals->mImplementation->mAudioPlayerReaderThread =
			OI<CAudioPlayerReaderThread>(
					new CAudioPlayerReaderThread(*this, *mInternals->mImplementation->mQueue,
							mInternals->mImplementation->mMixFormat->nBlockAlign,
							mInternals->mImplementation->mMaxPeriodInFrames, CAudioPlayerInternals::readerThreadError,
							mInternals));

	// Do super
	return CAudioProcessor::connectInput(audioProcessor, audioProcessingFormat);
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CAudioPlayer::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Stop sending frames
	mInternals->mImplementation->pause();
	while (mInternals->mImplementation->mOnFillBufferIsSendingFrames)
		// Sleep
		CThread::sleepFor(0.001);

	// Reset Reader Thread
	mInternals->mImplementation->mAudioPlayerReaderThread->stopReading();

	// Reset buffer
	mInternals->mImplementation->mQueue->reset();

	// Setup to play requested frames
	mInternals->mImplementation->mAudioPlayerReaderThread->seek(mInternals->mImplementation->mStartTimeInterval,
			mInternals->mImplementation->mDurationTimeInterval.hasValue() ?
					(UInt32) (*mInternals->mImplementation->mDurationTimeInterval *
							(UniversalTimeInterval) mInternals->mImplementation->mMixFormat->nSamplesPerSec) :
					~0);

	// Reset the pipeline
	OI<SError>	error = CAudioDestination::reset();
	ReturnErrorIfError(error);

	// Reset frame index
	mInternals->mImplementation->mOnFillBufferFrameIndex = 0;

	// Resume
	mInternals->mImplementation->mAudioPlayerReaderThread->resume();

	return OI<SError>();
}

// MARK: CAudioDestination methods

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::setupComplete()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mImplementation->mAudioPlayerReaderThread->resume();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CString& CAudioPlayer::getIdentifier() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mImplementation->mIdentifier;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::setGain(Float32 gain)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mImplementation->setGain(gain);
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::play()
//----------------------------------------------------------------------------------------------------------------------
{
	// Wait until initialized
	while (mInternals->mImplementation->mState == CAudioPlayerImplementation::kInitializing)
		// Sleep
		CThread::sleepFor(0.001);

	// Start
	mInternals->mImplementation->play();
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::pause()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mImplementation->pause();
}

//----------------------------------------------------------------------------------------------------------------------
bool CAudioPlayer::isPlaying() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mImplementation->mState == CAudioPlayerImplementation::kPlaying;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::startSeek()
//----------------------------------------------------------------------------------------------------------------------
{
	// Stop sending frames
	mInternals->mImplementation->mOnFillBufferShouldSendFrames = false;

	// Setup for finish seek
	mInternals->mImplementation->mFinishSeekShouldPause =
			mInternals->mImplementation->mState == CAudioPlayerImplementation::kPaused;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::seek(UniversalTimeInterval timeInterval, OV<UniversalTimeInterval> durationTimeInterval, bool playPreview)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mImplementation->mStartTimeInterval = timeInterval;
	mInternals->mImplementation->mDurationTimeInterval = durationTimeInterval;

	// Stop sending frames
	mInternals->mImplementation->mOnFillBufferShouldSendFrames = false;
	while (mInternals->mImplementation->mOnFillBufferIsSendingFrames)
		// Sleep
		CThread::sleepFor(0.001);

	// Reset Reader Thread
	mInternals->mImplementation->mAudioPlayerReaderThread->stopReading();

	// Reset buffer
	mInternals->mImplementation->mQueue->reset();

	// Reset the pipeline
	CAudioDestination::reset();

	// Reset frame index
	mInternals->mImplementation->mOnFillBufferFrameIndex = 0;

	// Check for preview
	if (playPreview) {
		// Setup to play preview length frames
		mInternals->mImplementation->mAudioPlayerReaderThread->seek(timeInterval,
				(UInt32) (timeInterval * CAudioPlayer::kPreviewDuration));

		// Send frames
		mInternals->mImplementation->mOnFillBufferShouldSendFrames = true;

		// Check if need to start playing
		if (mInternals->mImplementation->mState == CAudioPlayerImplementation::kPaused)
			// Start playing
			mInternals->mImplementation->play();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::finishSeek()
//----------------------------------------------------------------------------------------------------------------------
{
	// Stop sending frames
	mInternals->mImplementation->mOnFillBufferShouldSendFrames = false;
	while (mInternals->mImplementation->mOnFillBufferIsSendingFrames)
		// Sleep
		CThread::sleepFor(0.001);

	// Reset Reader Thread
	mInternals->mImplementation->mAudioPlayerReaderThread->stopReading();

	// Reset buffer
	mInternals->mImplementation->mQueue->reset();

	// Setup to play requested frames
	mInternals->mImplementation->mAudioPlayerReaderThread->seek(mInternals->mImplementation->mStartTimeInterval,
			mInternals->mImplementation->mDurationTimeInterval.hasValue() ?
					(UInt32) (*mInternals->mImplementation->mDurationTimeInterval *
							(UniversalTimeInterval) mInternals->mImplementation->mMixFormat->nSamplesPerSec) :
					~0);

	// Reset the pipeline
	CAudioDestination::reset();

	// Reset frame index
	mInternals->mImplementation->mOnFillBufferFrameIndex = 0;

	// Resume
	mInternals->mImplementation->mAudioPlayerReaderThread->resume();

	// Restore pre-seek state
	mInternals->mImplementation->mOnFillBufferShouldSendFrames = true;
	if (mInternals->mImplementation->mFinishSeekShouldPause)
		// Pause
		mInternals->mImplementation->pause();
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::setMaxAudioPlayers(UInt32 maxAudioPlayers)
//----------------------------------------------------------------------------------------------------------------------
{
}
