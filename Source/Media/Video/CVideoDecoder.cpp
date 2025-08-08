//----------------------------------------------------------------------------------------------------------------------
//	CVideoDecoder.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CVideoDecoder.h"

#include "CCodecRegistry.h"
#include "CReferenceCountable.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CVideoDecoder::Internals

class CVideoDecoder::Internals : public TReferenceCountableAutoDelete<Internals> {
	public:
		Internals(const SVideo::Format& videoFormat, const I<CDecodeVideoCodec>& videoCodec,
				const CString& identifier) :
			TReferenceCountableAutoDelete(),
					mVideoFormat(videoFormat), mVideoCodec(videoCodec), mIdentifier(identifier)
			{}

		SVideo::Format				mVideoFormat;
		I<CDecodeVideoCodec>		mVideoCodec;
		CString						mIdentifier;

		OV<CVideoProcessor::Format>	mVideoProcessorFormat;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CVideoDecoder

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CVideoDecoder::CVideoDecoder(const SVideo::Format& videoFormat, const I<CDecodeVideoCodec>& videoCodec,
		const CVideoProcessor::Format& videoProcessorFormat, const CString& identifier) : CVideoSource()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals(videoFormat, videoCodec, identifier);

	// Store
	mInternals->mVideoProcessorFormat = OV<CVideoProcessor::Format>(videoProcessorFormat);

	// Setup Video Codec
	mInternals->mVideoCodec->setup(videoProcessorFormat);
}

//----------------------------------------------------------------------------------------------------------------------
CVideoDecoder::CVideoDecoder(const CVideoDecoder& other) : CVideoSource(other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CVideoDecoder::~CVideoDecoder()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: CVideoProcessor methods

//----------------------------------------------------------------------------------------------------------------------
TArray<CString> CVideoDecoder::getSetupDescription(const CString& indent)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get upstream setup descriptions
	TNArray<CString>	setupDescriptions(CVideoSource::getSetupDescription(indent));

	// Add our setup description
	setupDescriptions += indent + CString(OSSTR("Video Decoder (")) + mInternals->mIdentifier + CString(OSSTR(")"));
	setupDescriptions +=
			indent + CString(OSSTR("    ")) +
					CCodecRegistry::mShared.getVideoCodecInfo(mInternals->mVideoFormat.getCodecID()).getDecodeName() +
					CString(OSSTR(", ")) + mInternals->mVideoFormat.getDescription();

	return setupDescriptions;
}

//----------------------------------------------------------------------------------------------------------------------
void CVideoDecoder::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Do super
	CVideoSource::seek(timeInterval);

	// Update codec
	mInternals->mVideoCodec->seek(getCurrentTimeInterval());
}

//----------------------------------------------------------------------------------------------------------------------
CVideoProcessor::PerformResult CVideoDecoder::perform()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UniversalTimeInterval	currentTimeInterval = getCurrentTimeInterval();

	// Decode
	TIResult<CVideoFrame>	videoFrame = mInternals->mVideoCodec->decode();
	ReturnValueIfResultError(videoFrame, PerformResult(videoFrame.getError()));

	// Update
	setCurrentTimeInterval(currentTimeInterval + 1.0 / mInternals->mVideoProcessorFormat->getFrameRate());

	return PerformResult(currentTimeInterval, *videoFrame);
}

//----------------------------------------------------------------------------------------------------------------------
void CVideoDecoder::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Do super
	CVideoSource::reset();

	// Update codec
	mInternals->mVideoCodec->seek(getCurrentTimeInterval());
}
