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
					mVideoFormat(videoFormat), mVideoCodec(videoCodec), mIdentifier(identifier),
					mCurrentTimeInterval(0.0)
			{}

		SVideo::Format				mVideoFormat;
		I<CDecodeVideoCodec>		mVideoCodec;
		CString						mIdentifier;

		OV<CVideoProcessor::Format>	mVideoProcessorFormat;
		SMedia::Segment				mMediaSegment;
		UniversalTimeInterval		mCurrentTimeInterval;
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
void CVideoDecoder::setMediaSegment(const SMedia::Segment& mediaSegment)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	bool	performSeek = mediaSegment.getStartTimeInterval() != mInternals->mMediaSegment.getStartTimeInterval();

	// Store
	mInternals->mMediaSegment = mediaSegment;

	// Check if need seek
	if (performSeek)
		// Seek
		seek(mInternals->mMediaSegment.getStartTimeInterval());
}

//----------------------------------------------------------------------------------------------------------------------
void CVideoDecoder::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Bound the given time
	timeInterval = std::max<UniversalTimeInterval>(timeInterval, mInternals->mMediaSegment.getStartTimeInterval());
	if (mInternals->mMediaSegment.getDurationTimeInterval().hasValue())
		// Limit to duration
		timeInterval = std::min<UniversalTimeInterval>(timeInterval, *mInternals->mMediaSegment.getEndTimeInterval());

	// Update
	mInternals->mCurrentTimeInterval = timeInterval;

	// Inform codec
	mInternals->mVideoCodec->seek(timeInterval);
}

//----------------------------------------------------------------------------------------------------------------------
CVideoProcessor::PerformResult CVideoDecoder::perform()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UniversalTimeInterval	currentTimeInterval = mInternals->mCurrentTimeInterval;

	// Decode
	TIResult<CVideoFrame>	videoFrame = mInternals->mVideoCodec->decode();
	ReturnValueIfResultError(videoFrame, PerformResult(videoFrame.getError()));

	// Update
	currentTimeInterval += 1.0 / mInternals->mVideoProcessorFormat->getFramerate();

	return PerformResult(currentTimeInterval, *videoFrame);
}

//----------------------------------------------------------------------------------------------------------------------
void CVideoDecoder::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Seek
	mInternals->mVideoCodec->seek(mInternals->mMediaSegment.getStartTimeInterval());
	mInternals->mCurrentTimeInterval = mInternals->mMediaSegment.getStartTimeInterval();
}
