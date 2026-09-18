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

using namespace Microsoft::WRL;

#if defined(__cplusplus_winrt)
	// C++/CX
	using namespace Windows::Media::Devices;
#else
	// C++/WinRT
	#include <winrt/Windows.Media.Devices.h>
	using namespace winrt::Windows::Media::Devices;
#endif

#define Delete(x)	{ delete x; x = nil; }

#pragma comment(lib, "mfuuid.lib")

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioPlayerAudioClient

class CAudioPlayerAudioClient {
	public:
										// Lifecycle methods
										CAudioPlayerAudioClient(IAudioClient3* audioClient,
												IAudioRenderClient* audioRenderClient,
												ISimpleAudioVolume* simpleAudioVolume, WAVEFORMATEX* mixFormat,
												UINT32 bufferFrames, UINT32 maxPeriodInFrames) :
											mAudioClient(audioClient), mAudioRenderClient(audioRenderClient),
													mSimpleAudioVolume(simpleAudioVolume), mMixFormat(mixFormat),
													mBufferFrames(bufferFrames),
													mMaxPeriodInFrames(maxPeriodInFrames)
											{}
										CAudioPlayerAudioClient(const CAudioPlayerAudioClient& other) :
											mAudioClient(other.mAudioClient),
													mAudioRenderClient(other.mAudioRenderClient),
													mSimpleAudioVolume(other.mSimpleAudioVolume),
													mMixFormat(other.mMixFormat),
													mBufferFrames(other.mBufferFrames),
													mMaxPeriodInFrames(other.mMaxPeriodInFrames)
											{}

										// Instance methods
			SAudio::ProcessingSetup		getAudioProcessingSetup() const
											{ return SAudio::ProcessingSetup((UInt8) mMixFormat->wBitsPerSample,
													(Float32) mMixFormat->nSamplesPerSec,
													SAudio::ChannelMap((UInt8) mMixFormat->nChannels),
													SAudio::ProcessingSetup::SampleTypeOption::kSampleTypeFloat,
													SAudio::ProcessingSetup::EndianOption::kEndianNative,
													SAudio::ProcessingSetup::InterleavedOption::kInterleaved); }

		IAudioClient3*		mAudioClient;
		IAudioRenderClient*	mAudioRenderClient;
		ISimpleAudioVolume*	mSimpleAudioVolume;
		WAVEFORMATEX*		mMixFormat;
		UINT32				mBufferFrames;
		UINT32				mMaxPeriodInFrames;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioPlayerAudioClientActivation

class CAudioPlayerAudioClientActivation :
		public RuntimeClass<RuntimeClassFlags<ClassicCom>, FtmBase, IActivateAudioInterfaceCompletionHandler> {
	public:
													// Lifecycle methods
													CAudioPlayerAudioClientActivation() :
														mAudioClient(nullptr), mAudioRenderClient(nullptr),
																mSimpleAudioVolume(nullptr), mMixFormat(nullptr),
																mBufferFrames(0), mMaxPeriodInFrames(0),
																mIsComplete(false)
														{}

													// IActivateAudioInterfaceCompletionHandler methods
				HRESULT								ActivateCompleted(
															IActivateAudioInterfaceAsyncOperation*
																	activateAudioInterfaceAsyncOperation);

													// Instance methods
				void								processHRESULT(HRESULT result, OSStringType method);

													// Class methods
		static	TVResult<CAudioPlayerAudioClient>	activate();

		IAudioClient3*		mAudioClient;
		IAudioRenderClient*	mAudioRenderClient;
		ISimpleAudioVolume*	mSimpleAudioVolume;
		WAVEFORMATEX*		mMixFormat;
		UINT32				mBufferFrames;
		UINT32				mMaxPeriodInFrames;
		OV<SError>			mError;
		bool				mIsComplete;
};

// MARK: IActivateAudioInterfaceCompletionHandler methods

//----------------------------------------------------------------------------------------------------------------------
HRESULT CAudioPlayerAudioClientActivation::ActivateCompleted(
		IActivateAudioInterfaceAsyncOperation* activateAudioInterfaceAsyncOperation)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	HRESULT	result;

	// Check activation result
	HRESULT		activationResult;
	IUnknown*	audioInterface;
	result = activateAudioInterfaceAsyncOperation->GetActivateResult(&activationResult, &audioInterface);
	if (FAILED(result))
		// Query failed
		processHRESULT(result, OSSTR("GetActivateResult()"));
	else if (FAILED(activationResult))
		// Activation failed
		processHRESULT(activationResult, OSSTR("checking status in ActivateCompleted()"));
	else {
		// Get the Audio Client
		audioInterface->QueryInterface(IID_PPV_ARGS(&mAudioClient));

		// Configure
		AudioClientProperties	audioClientProperties = {0};
		audioClientProperties.cbSize = sizeof(AudioClientProperties);
		audioClientProperties.eCategory = AudioCategory_Media;
		result = mAudioClient->SetClientProperties(&audioClientProperties);
		processHRESULT(result, OSSTR("SetClientProperties()"));

		if (!mError.hasValue()) {
			// Get Mix Format
			result = mAudioClient->GetMixFormat(&mMixFormat);
			processHRESULT(result, OSSTR("GetMixFormat()"));
		}

		if (!mError.hasValue()) {
			// Get periods
			UINT32	defaultPeriodInFrames, fundamentalPeriodInFrames, minPeriodInFrames;
			result =
					mAudioClient->GetSharedModeEnginePeriod(mMixFormat, &defaultPeriodInFrames,
							&fundamentalPeriodInFrames, &minPeriodInFrames, &mMaxPeriodInFrames);
			processHRESULT(result, OSSTR("GetSharedModeEnginePeriod()"));
		}

		if (!mError.hasValue()) {
			// Initialize
			result =
					mAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
							AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST, 0, 0, mMixFormat,
							nullptr);
			processHRESULT(result, OSSTR("Initialize()"));
		}

		if (!mError.hasValue()) {
			// Get the buffer size
			result = mAudioClient->GetBufferSize(&mBufferFrames);
			processHRESULT(result, OSSTR("GetBufferSize()"));
		}

		if (!mError.hasValue()) {
			// Get the Render Client
			result = mAudioClient->GetService(__uuidof(IAudioRenderClient), (void**) &mAudioRenderClient);
			processHRESULT(result, OSSTR("GetService() for IAudioRenderClient"));
		}

		if (!mError.hasValue()) {
			// Get Simple Audio Volume
			result = mAudioClient->GetService(__uuidof(ISimpleAudioVolume), (void**) &mSimpleAudioVolume);
			processHRESULT(result, OSSTR("GetService() for ISimpleAudioVolume"));
		}
	}

	// Done
	mIsComplete = true;

	return S_OK;
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayerAudioClientActivation::processHRESULT(HRESULT result, OSStringType method)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check result
	if (FAILED(result)) {
		// Error
		mError = OV<SError>(SErrorFromHRESULT(result));
		CLogServices::logError(CString(method) + CString(OSSTR(" returned ")) + mError->getInternalDescription());
	}
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TVResult<CAudioPlayerAudioClient> CAudioPlayerAudioClientActivation::activate()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	ComPtr<CAudioPlayerAudioClientActivation>	activation = Make<CAudioPlayerAudioClientActivation>();

	// Get Audio Render ID
	auto	audioRenderID = MediaDevice::GetDefaultAudioRenderId(AudioDeviceRole::Default);
#if defined(__cplusplus_winrt)
	// C++/CX
	auto	audioRenderIDString = audioRenderID->Data();
#else
	// C++/WinRT
	auto	audioRenderIDString = audioRenderID.data();
#endif

	// Activate Audio Interface
	IActivateAudioInterfaceAsyncOperation*	activateAudioInterfaceAsyncOperation;
	HRESULT									result =
													ActivateAudioInterfaceAsync(audioRenderIDString,
															__uuidof(IAudioClient3), nullptr, activation.Get(),
															&activateAudioInterfaceAsyncOperation);
	activation->processHRESULT(result, OSSTR("ActivateAudioInterfaceAsync()"));
	if (activation->mError.hasValue())
		// Error
		return TVResult<CAudioPlayerAudioClient>(*activation->mError);
	activateAudioInterfaceAsyncOperation->Release();

	// Wait until complete
	while (!activation->mIsComplete)
		// Sleep
		CThread::sleepFor(0.001);

	return !activation->mError.hasValue() ?
			TVResult<CAudioPlayerAudioClient>(
					CAudioPlayerAudioClient(activation->mAudioClient, activation->mAudioRenderClient,
							activation->mSimpleAudioVolume, activation->mMixFormat, activation->mBufferFrames,
							activation->mMaxPeriodInFrames)) :
			TVResult<CAudioPlayerAudioClient>(*activation->mError);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioPlayerImplementation

class CAudioPlayerImplementation : public RuntimeClass<RuntimeClassFlags<ClassicCom>, FtmBase> {
	public:
		enum State {
			kInitialized,
			kPlaybackStarting,
			kPlaying,
			kResetting,

			kError,
		};

						CAudioPlayerImplementation(CAudioPlayer& audioPlayer, const CString& identifier,
								const CAudioPlayer::Info& info, const CAudioPlayerAudioClient& audioClient) :
							mAudioPlayer(audioPlayer), mIdentifier(identifier), mInfo(info),
									mFinishSeekShouldPause(false),
									mFillBufferEvent(CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS)),
									mAudioClient(audioClient.mAudioClient),
									mAudioRenderClient(audioClient.mAudioRenderClient), mFillBufferAsyncResult(nullptr),
									mSimpleAudioVolume(audioClient.mSimpleAudioVolume), mFillBufferKey(0),
									mState(kInitialized), mFillBufferAsyncCallback(*this, onFillBuffer),
									mPlayAsyncCallback(*this, onPlay), mResetAsyncCallback(*this, onReset),
									mBufferFrames(audioClient.mBufferFrames),
									mMaxPeriodInFrames(audioClient.mMaxPeriodInFrames),
									mMixFormat(audioClient.mMixFormat), mIsPlaying(false), mIsSeeking(false),
									mLastSeekTimeInterval(0.0), mCurrentPlaybackTimeInterval(0.0), mGain(1.0),
									mOnFillBufferShouldSendFrames(true), mOnFillBufferIsSendingFrames(false),
									mOnFillBufferShouldNotifyEndOfData(false), mOnFillBufferPreviousFrameCount(0),
									mOnFillBufferFrameIndex(0), mOnFillBufferFrameCount(~0U)
							{
								// Setup
								HRESULT	result;

								// Check sample ready event
								if (mFillBufferEvent == nullptr)
									// Error
									processWindowsError(OSSTR("CreateEventEx() for sample ready event"));

								// Startup Media Foundation
								result = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
								processHRESULT(result, OSSTR("MFStartup()"));

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
									// Update info
									mOnFillBufferFrameIndex += mOnFillBufferPreviousFrameCount;
									mCurrentPlaybackTimeInterval +=
											(UniversalTimeInterval) mOnFillBufferPreviousFrameCount /
													(UniversalTimeInterval) mMixFormat->nSamplesPerSec;

									mOnFillBufferFrameCount -= mOnFillBufferPreviousFrameCount;
									mOnFillBufferPreviousFrameCount = 0;

									// Check if seeking
									if (mOnFillBufferShouldSendFrames && !mIsSeeking)
										// Notify playback position updated
										mInfo.positionUpdated(mAudioPlayer, mCurrentPlaybackTimeInterval);

									// Notify queue read complete
									mAudioPlayerBufferThread->noteQueueReadComplete();
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
								if (mOnFillBufferShouldSendFrames) {
									// Sending frames
									CSRSWBIPSegmentedQueue::ReadBufferInfo	readBufferInfo = mQueue->requestRead();
									UInt32									frameCount =
																					std::min<UInt32>(framesAvailable,
																							mOnFillBufferFrameCount);
									UInt32									requiredByteCount =
																					frameCount *
																							mMixFormat->nBlockAlign;

									// Try to copy as many bytes as possible
									UInt32	byteOffset = 0;
									while ((requiredByteCount > 0) && readBufferInfo.hasBuffer()) {
										// Copy
										UInt32	bytesToCopy =
														std::min<UInt32>(requiredByteCount, readBufferInfo.mByteCount);
										::memcpy(data + byteOffset, readBufferInfo.bufferAtIndex(0), bytesToCopy);
										mQueue->commitRead(bytesToCopy);

										// Update info
										byteOffset += bytesToCopy;
										requiredByteCount -= bytesToCopy;
										mOnFillBufferPreviousFrameCount += bytesToCopy / mMixFormat->nBlockAlign;

										// Get next read buffer info
										readBufferInfo = mQueue->requestRead();
									}

									// Check if have frames
									if (mOnFillBufferPreviousFrameCount > 0) {
										// Iterate channels
										for (UInt32 channelIndex = 0; channelIndex < mMixFormat->nChannels;
												channelIndex++) {
											// Setup
											Float32		gain =
																(channelIndex < mChannelGains.getCount()) ?
																		mChannelGains[channelIndex] : mGain;
											Float32*	samplePtr = (Float32*) data + channelIndex;

											// Iterate samples
											for (UInt32 sampleIndex = 0; sampleIndex < mOnFillBufferPreviousFrameCount;
													sampleIndex++, samplePtr += mMixFormat->nChannels)
												// Apply gain
												*samplePtr *= gain;
										}
									}

									// Commit frames
									result = mAudioRenderClient->ReleaseBuffer(mOnFillBufferPreviousFrameCount, 0);
									processHRESULT(result, OSSTR("ReleaseBuffer()"));

									// Check situation
									if (requiredByteCount > 0) {
										// Check situation
										if (mOnFillBufferPreviousFrameCount > 0)
											// Added frames
											mOnFillBufferIsSendingFrames = true;
										else if (mOnFillBufferFrameIndex == 0)
											// Was recently started, but the reader thread doesn't yet have frames
											//	queued.
											mOnFillBufferIsSendingFrames = true;
										else if (paddingFrames > 0)
											// Did not add frames, but still have frames queued
											mOnFillBufferIsSendingFrames = true;
										else if (mAudioPlayerBufferThread->getDidReachEnd()) {
											// At the end
											if (mOnFillBufferIsSendingFrames && mOnFillBufferShouldNotifyEndOfData)
												// Just reached the end of data
												mInfo.endOfData(mAudioPlayer);

											mOnFillBufferIsSendingFrames = false;
										} else {
											// Have a discontinuity in the queued frames
											CLogServices::logError(
													CString(OSSTR("CAudioPlayer ")) + mIdentifier +
															CString(OSSTR(" requires frames but queue is empty, will glitch...")));

											mOnFillBufferIsSendingFrames = true;
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
								if (mState != kInitialized)
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
				void	reset()
							{
								// Check state
								if (mState != kPlaying)
									return;

								// Update state
								mState = kResetting;

								// Pause
								HRESULT	result =
												MFPutWorkItem2(MFASYNC_CALLBACK_QUEUE_MULTITHREADED, 0,
														&mResetAsyncCallback, nullptr);
								processHRESULT(result, OSSTR("MFPutWaitingWorkItem() for Reset"));

								// Wait
								while (mState == kResetting)
									// Sleep
									CThread::sleepFor(0.001);
							}
				HRESULT	onReset(__RPC__in_opt IMFAsyncResult& asyncResult)
							{
								// Check state
								if (mState != kResetting)
									return S_OK;

								// Stop
								HRESULT	result = mAudioClient->Stop();
								if (SUCCEEDED(result))
									// Success
									mState = kInitialized;
								else
									// Error
									processHRESULT(result, OSSTR("Stop()"));

								return S_OK;
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
									CLogServices::logError(
											CString(method) + CString(OSSTR(" returned ")) +
													SErrorFromHRESULT(result).getInternalDescription());
								}
							}
				void	processWindowsError(OSStringType method)
							{
								// Error
								mState = kError;
								CLogServices::logError(
										CString(method) + CString(OSSTR(" returned ")) +
												SErrorFromWindowsGetLastError().getInternalDescription());
							}

		static	HRESULT	onFillBuffer(__RPC__in_opt IMFAsyncResult& asyncResult,
								CAudioPlayerImplementation& implementation)
							{ return implementation.onFillBuffer(asyncResult); }
		static	HRESULT	onPlay(__RPC__in_opt IMFAsyncResult& asyncResult, CAudioPlayerImplementation& implementation)
							{ return implementation.onPlay(asyncResult); }
		static	HRESULT	onReset(__RPC__in_opt IMFAsyncResult& asyncResult, CAudioPlayerImplementation& implementation)
							{ return implementation.onReset(asyncResult); }
 
		CAudioPlayer&									mAudioPlayer;
		CString											mIdentifier;
		CAudioPlayer::Info								mInfo;

		bool											mFinishSeekShouldPause;
		HANDLE											mFillBufferEvent;
		IAudioClient3*									mAudioClient;
		IAudioRenderClient*								mAudioRenderClient;
		IMFAsyncResult*									mFillBufferAsyncResult;
		ISimpleAudioVolume*								mSimpleAudioVolume;
		MFWORKITEM_KEY									mFillBufferKey;
		OI<CAudioPlayerBufferThread>					mAudioPlayerBufferThread;
		OI<CSRSWBIPSegmentedQueue>						mQueue;
		State											mState;
		TMFAsyncCallback<CAudioPlayerImplementation>	mFillBufferAsyncCallback;
		TMFAsyncCallback<CAudioPlayerImplementation>	mPlayAsyncCallback;
		TMFAsyncCallback<CAudioPlayerImplementation>	mResetAsyncCallback;
		UINT32											mBufferFrames;
		UINT32											mMaxPeriodInFrames;
		WAVEFORMATEX*									mMixFormat;

		bool											mIsPlaying;
		bool											mIsSeeking;
		OV<SMedia::Segment>								mMediaSegment;
		UniversalTimeInterval							mLastSeekTimeInterval;
		UniversalTimeInterval							mCurrentPlaybackTimeInterval;
		Float32											mGain;
		TNumberArray<Float32>							mChannelGains;

		bool											mOnFillBufferShouldSendFrames;
		bool											mOnFillBufferIsSendingFrames;
		bool											mOnFillBufferShouldNotifyEndOfData;
		UInt32											mOnFillBufferPreviousFrameCount;
		UInt32											mOnFillBufferFrameIndex;
		UInt32											mOnFillBufferFrameCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioPlayer::Internals

class CAudioPlayer::Internals {
	public:
						Internals(CAudioPlayer& audioPlayer, const CString& identifier, const CAudioPlayer::Info& info,
								const CAudioPlayerAudioClient& audioClient) :
							mAudioPlayer(audioPlayer), mInfo(info),
									mImplementation(
											Make<CAudioPlayerImplementation>(audioPlayer, identifier, info,
													audioClient))
							{}
						~Internals()
							{
								mImplementation->shutdown();
							}

		static	void	readerThreadError(const SError& error, void* userData)
							{
								// Setup
								Internals&	internals = *((Internals*) userData);

								// Call proc
								internals.mInfo.error(internals.mAudioPlayer, error);
							}

		CAudioPlayer&						mAudioPlayer;
		CAudioPlayer::Info					mInfo;
		ComPtr<CAudioPlayerImplementation>	mImplementation;
		OV<SAudio::ProcessingFormat>		mAudioProcessingFormat;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioPlayer

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioPlayer::CAudioPlayer(const CString& identifier, const Info& info, const CAudioPlayerAudioClient& audioClient) :
		CAudioDestination(audioClient.getAudioProcessingSetup())
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(*this, identifier, info, audioClient);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioPlayer::~CAudioPlayer()
//----------------------------------------------------------------------------------------------------------------------
{
	// Stop sending frames
	mInternals->mImplementation->mOnFillBufferShouldSendFrames = false;
	while (mInternals->mImplementation->mOnFillBufferIsSendingFrames)
		// Sleep
		CThread::sleepFor(0.001);

	// Reset
	mInternals->mImplementation->reset();

	// Stop threads
	if (mInternals->mImplementation->mAudioPlayerBufferThread.hasInstance())
		// Shutdown
		mInternals->mImplementation->mAudioPlayerBufferThread->shutdown();

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
	UInt32	frameCount = (UInt32) (CAudioPlayer::getPlaybackBufferDuration() * audioProcessingFormat.getSampleRate());
	mInternals->mImplementation->mQueue =
			OI<CSRSWBIPSegmentedQueue>(
					new CSRSWBIPSegmentedQueue(1, frameCount * mInternals->mImplementation->mMixFormat->nBlockAlign));
	mInternals->mImplementation->mAudioPlayerBufferThread =
			OI<CAudioPlayerBufferThread>(
					new CAudioPlayerBufferThread(*this, *mInternals->mImplementation->mQueue,
							mInternals->mImplementation->mMixFormat->nBlockAlign,
							mInternals->mImplementation->mMaxPeriodInFrames, Internals::readerThreadError, mInternals));

	// Do super
	return __super::connectInput(audioProcessor, audioProcessingFormat);
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CString> CAudioPlayer::getSetupDescription(const CString& indent)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get upstream setup descriptions
	TNArray<CString>	setupDescriptions(CAudioDestination::getSetupDescription(indent));

	// Add our setup description
	setupDescriptions += indent + CString(OSSTR("Audio Player"));
	setupDescriptions += indent + CString("    ") + mInternals->mAudioProcessingFormat->getDescription();

	return setupDescriptions;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::setMediaSegment(const OV<SMedia::Segment>& mediaSegment)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mImplementation->mMediaSegment = mediaSegment;

	// Do super
	__super::setMediaSegment(mediaSegment);
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Stop sending frames
	mInternals->mImplementation->mOnFillBufferShouldSendFrames = false;
	while (mInternals->mImplementation->mOnFillBufferIsSendingFrames)
		// Sleep
		CThread::sleepFor(0.001);

	// Pause Reader Thread
	mInternals->mImplementation->mAudioPlayerBufferThread->pause();

	// Reset buffer
	mInternals->mImplementation->mQueue->reset();

	// Do super
	CAudioDestination::seek(timeInterval);

	// Update stuffs
	mInternals->mImplementation->mIsSeeking = !mInternals->mImplementation->mIsPlaying;
	mInternals->mImplementation->mLastSeekTimeInterval = timeInterval;
	mInternals->mImplementation->mCurrentPlaybackTimeInterval = timeInterval;

	mInternals->mImplementation->mOnFillBufferFrameIndex = 0;
	mInternals->mImplementation->mOnFillBufferFrameCount =
			!mInternals->mImplementation->mIsPlaying ?
					(UInt32) ((UniversalTimeInterval) mInternals->mImplementation->mMixFormat->nSamplesPerSec *
							CAudioPlayer::mPreviewDuration) :
					~0;

	// Resume
	mInternals->mImplementation->mAudioPlayerBufferThread->resume();

	// Send frames
	mInternals->mImplementation->mOnFillBufferShouldSendFrames = true;

	// Play if not already playing
	mInternals->mImplementation->play();

	// Notify playback position updated
	mInternals->mImplementation->mInfo.positionUpdated(*this, timeInterval);
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::stop()
//----------------------------------------------------------------------------------------------------------------------
{
	// Stop sending frames
	mInternals->mImplementation->mOnFillBufferShouldSendFrames = false;
	while (mInternals->mImplementation->mOnFillBufferIsSendingFrames)
		// Sleep
		CThread::sleepFor(0.001);

	// Reset
	mInternals->mImplementation->reset();

	// No longer playing
	mInternals->mImplementation->mIsPlaying = false;

	// Pause Reader Thread
	mInternals->mImplementation->mAudioPlayerBufferThread->pause();

	// Reset buffer
	mInternals->mImplementation->mQueue->reset();

	// Seek to the start of the segment if present, or start
	CAudioDestination::seek(
			mInternals->mImplementation->mMediaSegment.hasValue() ?
					mInternals->mImplementation->mMediaSegment->getStartTimeInterval() : 0.0);

	// Reset
	mInternals->mImplementation->mCurrentPlaybackTimeInterval =
			mInternals->mImplementation->mMediaSegment.hasValue() ?
					mInternals->mImplementation->mMediaSegment->getStartTimeInterval() : 0.0;

	mInternals->mImplementation->mOnFillBufferFrameIndex = 0;
	mInternals->mImplementation->mOnFillBufferFrameCount = ~0U;

	// Resume
	mInternals->mImplementation->mAudioPlayerBufferThread->resume();
}

// MARK: CAudioDestination methods

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::setupComplete()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mImplementation->mAudioPlayerBufferThread->resume();
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
	mInternals->mImplementation->mGain = gain;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::setGain(const TNumberArray<Float32>& channelGains)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mImplementation->mChannelGains = channelGains;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::play()
//----------------------------------------------------------------------------------------------------------------------
{
	// We are now playing
	mInternals->mImplementation->mIsPlaying = true;
	mInternals->mImplementation->mIsSeeking = false;

	// Send frames
	mInternals->mImplementation->mOnFillBufferShouldSendFrames = true;
	mInternals->mImplementation->mOnFillBufferShouldNotifyEndOfData = true;
	mInternals->mImplementation->mOnFillBufferFrameCount = ~0U;

	// Start
	mInternals->mImplementation->play();
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::pause()
//----------------------------------------------------------------------------------------------------------------------
{
	// Pause
	mInternals->mImplementation->mIsPlaying = false;

	// Don't send frames
	mInternals->mImplementation->mOnFillBufferShouldSendFrames = false;
}

//----------------------------------------------------------------------------------------------------------------------
bool CAudioPlayer::isPlaying() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mImplementation->mIsPlaying;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::startSeek()
//----------------------------------------------------------------------------------------------------------------------
{
	// Stop sending frames
	mInternals->mImplementation->mOnFillBufferShouldSendFrames = false;
	mInternals->mImplementation->mOnFillBufferShouldNotifyEndOfData = false;

	// Setup
	mInternals->mImplementation->mIsSeeking = true;
	mInternals->mImplementation->mLastSeekTimeInterval = mInternals->mImplementation->mCurrentPlaybackTimeInterval;

	// Start
	mInternals->mImplementation->play();
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

	// Pause Reader Thread
	mInternals->mImplementation->mAudioPlayerBufferThread->pause();

	// Reset buffer
	mInternals->mImplementation->mQueue->reset();

	// Seek to the last seek time interval
	CAudioDestination::seek(mInternals->mImplementation->mLastSeekTimeInterval);

	// Reset stuffs
	mInternals->mImplementation->mIsSeeking = false;
	mInternals->mImplementation->mCurrentPlaybackTimeInterval = mInternals->mImplementation->mLastSeekTimeInterval;

	mInternals->mImplementation->mOnFillBufferFrameIndex = 0;
	mInternals->mImplementation->mOnFillBufferFrameCount = ~0U;

	// Resume
	mInternals->mImplementation->mAudioPlayerBufferThread->resume();

	// Begin sending frames again
	mInternals->mImplementation->mOnFillBufferShouldSendFrames = mInternals->mImplementation->mIsPlaying;
	mInternals->mImplementation->mOnFillBufferShouldNotifyEndOfData = true;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TVResult<I<CAudioPlayer> > CAudioPlayer::create(const CString& identifier, const Info& info)
//----------------------------------------------------------------------------------------------------------------------
{
	// Activate audio client
	TVResult<CAudioPlayerAudioClient>	audioClient = CAudioPlayerAudioClientActivation::activate();

	return audioClient.hasValue() ?
			TVResult<I<CAudioPlayer> >(I<CAudioPlayer>(new CAudioPlayer(identifier, info, *audioClient))) :
			TVResult<I<CAudioPlayer> >(audioClient.getError());
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioPlayer::setMaxAudioPlayers(UInt32 maxAudioPlayers)
//----------------------------------------------------------------------------------------------------------------------
{
}
