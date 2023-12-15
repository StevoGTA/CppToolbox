//----------------------------------------------------------------------------------------------------------------------
//	CMediaFoundationResampler.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMediaFoundationResampler.h"

#include "CMediaFoundationServices.h"

#undef Delete

#include <mfapi.h>
#include <mftransform.h>

#define Delete(x)	{ delete x; x = nil; }

#pragma comment(lib, "wmcodecdspuuid")

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

EXTERN_C const CLSID CLSID_CResamplerMediaObject;
static const PROPERTYKEY MFPKEY_WMRESAMP_FILTERQUALITY = { { 0xaf1adc73, 0xa210, 0x4b05, {0x96, 0x6e, 0x54, 0x91, 0xcf, 0xf4, 0x8b, 0x1d} }, 0x01 }; 

static	CString	sErrorDomain(OSSTR("CMediaFoundationResampler"));
static	SError	sSetupDidNotCompleteError(sErrorDomain, 1, CString(OSSTR("Setup did not complete")));

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaFoundationResamplerInternals

class CMediaFoundationResamplerInternals {
	public:
							CMediaFoundationResamplerInternals(CAudioConverter& audioConverter) :
								mAudioConverter(audioConverter),
										mSourceTimeInterval(0.0)
								{}
							~CMediaFoundationResamplerInternals()
								{
									// Cleanup
									MFShutdown();
								}

		static	OV<SError>	fillInputBuffer(IMFSample* sample, IMFMediaBuffer* mediaBuffer, void* userData)
								{
									// Setup
									CMediaFoundationResamplerInternals&	internals =
																				*((CMediaFoundationResamplerInternals*)
																						userData);

									// Load
									TVResult<SMedia::SourceInfo>	mediaSourceInfo =
																			CMediaFoundationServices::load(mediaBuffer,
																					internals.mAudioConverter,
																					*internals.mInputAudioProcessingFormat);
									ReturnErrorIfResultError(mediaSourceInfo);

									// Store
									internals.mSourceTimeInterval = mediaSourceInfo->getTimeInterval();

									return OV<SError>();
								}

		CAudioConverter&				mAudioConverter;
		OV<SAudio::ProcessingFormat>	mInputAudioProcessingFormat;

		OCI<IMFTransform>				mResamplerTransform;

		OCI<IMFSample>					mInputSample;
		UniversalTimeInterval			mSourceTimeInterval;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaFoundationResampler

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CMediaFoundationResampler::CMediaFoundationResampler() : CAudioConverter()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CMediaFoundationResamplerInternals(*this);
}

//----------------------------------------------------------------------------------------------------------------------
CMediaFoundationResampler::~CMediaFoundationResampler()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CAudioProcessor methods

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CMediaFoundationResampler::connectInput(const I<CAudioProcessor>& audioProcessor,
	const SAudio::ProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Startup Media Foundation
	HRESULT	result = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	ReturnErrorIfFailed(result, OSSTR("MFStartup()"));

	// Store
	mInternals->mInputAudioProcessingFormat.setValue(audioProcessingFormat);

	// Setup transform
	IUnknown*	unknown;
	result = CoCreateInstance(CLSID_CResamplerMediaObject, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&unknown));
	ReturnErrorIfFailed(result, OSSTR("CoCreateInstance"));

	// Query Transform
	IMFTransform*	transform;
	result = unknown->QueryInterface(IID_PPV_ARGS(&transform));
	OCI<IMFTransform>	audioResampler(transform);
	ReturnErrorIfFailed(result, OSSTR("Querying Resampler Transform"));

	// Query Resampler Property Store
	IPropertyStore*	propertyStore;
	result = unknown->QueryInterface(IID_PPV_ARGS(&propertyStore));
	OCI<IPropertyStore>	resamplerPropertyStore(propertyStore);
	ReturnErrorIfFailed(result, OSSTR("Querying Resampler Property Store"));

	// Configure
	PROPVARIANT	pv;
	pv.vt = VT_I4;
	pv.intVal = 60;
	result = propertyStore->SetValue(MFPKEY_WMRESAMP_FILTERQUALITY, pv);
	ReturnErrorIfFailed(result, OSSTR("Setting Filter Quality"));

	// Setup input media type
	TCIResult<IMFMediaType>	inputMediaType = CMediaFoundationServices::createMediaType(audioProcessingFormat);
	ReturnErrorIfResultError(inputMediaType);

	result = transform->SetInputType(0, *(inputMediaType.getInstance()), 0);
	ReturnErrorIfFailed(result, OSSTR("SetInputType"));

	// Setup output media type
	TCIResult<IMFMediaType>	outputMediaType = CMediaFoundationServices::createMediaType(*mOutputAudioProcessingFormat);
	ReturnErrorIfResultError(outputMediaType);

	result = transform->SetOutputType(0, *(outputMediaType.getInstance()), 0);
	ReturnErrorIfFailed(result, OSSTR("SetOutputType"));

	// Store
	mInternals->mResamplerTransform = audioResampler;

	// Begin streaming!
	result = mInternals->mResamplerTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
	ReturnErrorIfFailed(result, OSSTR("ProcessMessage to begin streaming"));

	return CAudioProcessor::connectInput(audioProcessor, audioProcessingFormat);
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CString> CMediaFoundationResampler::getSetupDescription(const CString& indent)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get upstream setup descriptions
	TNArray<CString>	setupDescriptions = CBasicAudioProcessor::getSetupDescription(indent);

	// Add our setup description
	setupDescriptions += indent + CString(OSSTR("Media Foundation Resampler"));
	setupDescriptions +=
			indent + CString(OSSTR("    From: ")) + mInternals->mInputAudioProcessingFormat->getDescription();
	setupDescriptions += indent + CString(OSSTR("      To: ")) + mOutputAudioProcessingFormat->getDescription();

	return setupDescriptions;
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<SMedia::SourceInfo> CMediaFoundationResampler::performInto(CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	if (!mInternals->mResamplerTransform.hasInstance())
		// Failed initialization
		return TVResult<SMedia::SourceInfo>(sSetupDidNotCompleteError);

	// Check if need to create input sample
	if (!mInternals->mInputSample.hasInstance()) {
		// Create input sample
		CAudioFrames::Requirements	audioFramesRequirements = queryRequirements();
		UInt32						byteCount =
											audioFramesRequirements.getFrameCount(1024) *
													mInternals->mInputAudioProcessingFormat->getBytesPerFrame();
		TCIResult<IMFSample>			sample = CMediaFoundationServices::createSample(byteCount);
		ReturnValueIfResultError(sample, TVResult<SMedia::SourceInfo>(sample.getError()));
		mInternals->mInputSample = sample.getInstance();
	}

	// Fill audio frames as much as we can
	while (audioFrames.getCurrentFrameCount() < audioFrames.getAllocatedFrameCount()) {
		// Create output sample
		TCIResult<IMFSample>	sample =
										CMediaFoundationServices::createSample(
												audioFrames.getAllocatedFrameCount() *
														mOutputAudioProcessingFormat->getBytesPerFrame());
		ReturnValueIfResultError(sample, TVResult<SMedia::SourceInfo>(sample.getError()));

		// Process output
		OV<SError>	error =
							CMediaFoundationServices::processOutput(*mInternals->mResamplerTransform,
									*sample.getInstance(),
									CMediaFoundationServices::ProcessOutputInfo(
											CMediaFoundationResamplerInternals::fillInputBuffer,
											mInternals->mInputSample, mInternals));
		ReturnValueIfError(error, TVResult<SMedia::SourceInfo>(*error));

		// Complete write
		TVResult<UInt32>	result =
									CMediaFoundationServices::completeWrite(*sample.getInstance(), 0, audioFrames,
											*mOutputAudioProcessingFormat);
		ReturnValueIfResultError(result, TVResult<SMedia::SourceInfo>(result.getError()));
	}

	return TVResult<SMedia::SourceInfo>(SMedia::SourceInfo(mInternals->mSourceTimeInterval));
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaFoundationResampler::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Do super
	CAudioProcessor::reset();

	// Reset Resampler
    mInternals->mResamplerTransform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);
    mInternals->mResamplerTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
    mInternals->mResamplerTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, 0);
}

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudio::ProcessingSetup> CMediaFoundationResampler::getInputSetups() const
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudio::ProcessingSetup>(
			SAudio::ProcessingSetup(SAudio::ProcessingSetup::BitsInfo::mUnspecified,
					SAudio::ProcessingSetup::SampleRateInfo::mUnspecified,
					SAudio::ProcessingSetup::ChannelMapInfo(mOutputAudioProcessingFormat->getChannelMap()),
					SAudio::ProcessingSetup::kSampleTypeUnspecified, SAudio::ProcessingSetup::kEndianLittle,
					SAudio::ProcessingSetup::kInterleaved));
}
