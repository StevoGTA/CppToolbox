//----------------------------------------------------------------------------------------------------------------------
//	CWAVEMediaSource.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CWAVEMediaSource.h"

#include "CIMAADPCMAudioCodec.h"
#include "CPCMAudioCodec.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Errors

CString	CWAVEMediaSource::mErrorDomain(OSSTR("CWAVEMediaSource"));
static	SError	sInvalidWAVEFileError(CWAVEMediaSource::mErrorDomain, 1, CString(OSSTR("Invalid WAVE file")));
static	SInt32	kUnsupportedCodecCode = 2;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CWAVEMediaSource

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
SMediaSource::QueryTracksResult CWAVEMediaSource::queryTracks(const I<CSeekableDataSource>& seekableDataSource,
		const OR<const CAppleResourceManager>& appleResourceManager, SMediaSource::Options options)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	I<CWAVEMediaSourceImportTracker>	waveMediaSourceImportTracker = CWAVEMediaSourceImportTracker::instantiate();
	OI<CChunkReader>					chunkReader = waveMediaSourceImportTracker->setup(seekableDataSource);
	if (!chunkReader.hasInstance())
		// Not a WAVE file
		return SMediaSource::QueryTracksResult();

	// Process chunks
	OI<SError>	error;
	while (true) {
		// Read next chunk info
		TIResult<CChunkReader::ChunkInfo>	chunkInfo = chunkReader->readChunkInfo();
		if (chunkInfo.hasError())
			// Done reading chunks
			break;

		// Note chunk
		waveMediaSourceImportTracker->note(*chunkInfo, *chunkReader);

		// Seek to next chunk
		error = chunkReader->seekToNext(*chunkInfo);
		if (error.hasInstance())
			// Done reading chunks
			break;
	}

	// Check results
	if (!waveMediaSourceImportTracker->canFinalize())
		// Can't finalize
		return SMediaSource::QueryTracksResult(sInvalidWAVEFileError);

	// Finalize setup
	CAudioTrack			audioTrack = waveMediaSourceImportTracker->composeAudioTrack();
	CMediaTrackInfos	mediaTrackInfos;
	if (options & SMediaSource::kCreateDecoders)
		// Requesting decode info
		mediaTrackInfos.add(
				CMediaTrackInfos::AudioTrackInfo(audioTrack,
						waveMediaSourceImportTracker->createAudioCodec(seekableDataSource)));
	else
		// Not requesting decode info
		mediaTrackInfos.add(CMediaTrackInfos::AudioTrackInfo(audioTrack));

	return SMediaSource::QueryTracksResult(mediaTrackInfos);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CWAVEMediaSourceImportTrackerInternals

class CWAVEMediaSourceImportTrackerInternals {
	public:
		CWAVEMediaSourceImportTrackerInternals() {}

		OV<UInt16>				mFormatTag;
		OV<UInt16>				mSampleSize;
		OV<UInt16>				mBlockAlign;
		OI<SAudioStorageFormat>	mAudioStorageFormat;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CWAVEMediaSourceImportTracker

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CWAVEMediaSourceImportTracker::CWAVEMediaSourceImportTracker()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CWAVEMediaSourceImportTrackerInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CWAVEMediaSourceImportTracker::~CWAVEMediaSourceImportTracker()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
OI<CChunkReader> CWAVEMediaSourceImportTracker::setup(const I<CSeekableDataSource>& seekableDataSource)
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify it's a WAVE Media Source
	if (!seekableDataSource->canReadData(0, sizeof(SWAVEFORMChunk32)))
		return OI<CChunkReader>();
	TIResult<CData>	data = seekableDataSource->readData(0, sizeof(SWAVEFORMChunk32));
	ReturnValueIfResultError(data, OI<CChunkReader>());

	const	SWAVEFORMChunk32&	formChunk32 = *((SWAVEFORMChunk32*) data->getBytePtr());
	if ((formChunk32.getID() == kWAVEFORMChunkID) && (formChunk32.getFormType() == kWAVEFORMType)) {
		// Success
		OI<CChunkReader>	chunkReader(new CChunkReader(seekableDataSource, CChunkReader::kFormat32BitLittleEndian));
		chunkReader->setPos(CByteReader::kPositionFromBeginning, sizeof(SWAVEFORMChunk32));

		return chunkReader;
	}

	return OI<CChunkReader>();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CWAVEMediaSourceImportTracker::note(const CChunkReader::ChunkInfo& chunkInfo, CChunkReader& chunkReader)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check chunk ID
	switch (chunkInfo.getID()) {
		case kWAVEFormatChunkID: {
			// Format chunk
			TIResult<CData>	readPayload = chunkReader.readPayload(chunkInfo);
			ReturnErrorIfResultError(readPayload);

			const	SWAVEFORMAT&	waveFormat = *((SWAVEFORMAT*) readPayload->getBytePtr());
			if (readPayload->getByteCount() >= sizeof(SWAVEFORMATEX)) {
				// Have WAVEFORMATEX
				const	SWAVEFORMATEX&	waveFormatEx = *((SWAVEFORMATEX*) readPayload->getBytePtr());
				mInternals->mSampleSize = OV<UInt16>(waveFormatEx.getBitsPerSample());
				mInternals->mBlockAlign = OV<UInt16>(waveFormatEx.getBlockAlign());
			} else if ((waveFormat.getChannels() > 0) && (waveFormat.getSamplesPerSecond() > 0) &&
					(waveFormat.getAverageBytesPerSec() > 0)) {
				// Use WAVEFORMAT
				mInternals->mSampleSize =
						OV<UInt16>(
								(UInt16) (waveFormat.getAverageBytesPerSec() / waveFormat.getSamplesPerSecond())
										/ waveFormat.getChannels() * 8);
				mInternals->mBlockAlign = OV<UInt16>(waveFormat.getBlockAlign());
			}

			if (note(waveFormat, mInternals->mSampleSize, *readPayload))
				// Success
				return OI<SError>();
			else
				// Not supported
				return OI<SError>(
						SError(CWAVEMediaSource::mErrorDomain, kUnsupportedCodecCode,
								CString(OSSTR("Unsupported codec: ")) +
										CString(waveFormat.getFormatTag(), 4, true, true)));
		}

		case kWAVEDataChunkID:
			// Data chunk
			mDataChunkStartByteOffset = OV<UInt64>(chunkInfo.getThisChunkPos());
			mDataChunkByteCount =
					OV<UInt64>(std::min<SInt64>(chunkInfo.getByteCount(),
							chunkReader.getByteCount() - *mDataChunkStartByteOffset));

			return OI<SError>();

		default:
			// Unknown
			return OI<SError>();
	}
}

//----------------------------------------------------------------------------------------------------------------------
bool CWAVEMediaSourceImportTracker::note(const SWAVEFORMAT& waveFormat, const OV<UInt16>& sampleSize,
		const CData& chunkPayload)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mFormatTag = OV<UInt16>(waveFormat.getFormatTag());

	// Check format tag
	switch (waveFormat.getFormatTag()) {
		case 0x0000:	// Illegal/Unknown
			return false;

		case 0x0001:	// Integer PCM
			if (!sampleSize.hasValue() || (*sampleSize > 32))
				// Nope
				return false;

			mInternals->mAudioStorageFormat =
					CPCMAudioCodec::composeAudioStorageFormat(false, (UInt8) *sampleSize,
							(Float32) waveFormat.getSamplesPerSecond(), (UInt8) waveFormat.getChannels());

			return true;

		case 0x0003:	// IEEE Float
			if (!sampleSize.hasValue() || (*sampleSize > 32))
				// Nope
				return false;

			mInternals->mAudioStorageFormat =
					CPCMAudioCodec::composeAudioStorageFormat(true, (UInt8) *sampleSize,
							(Float32) waveFormat.getSamplesPerSecond(), (UInt8) waveFormat.getChannels());

			return true;

		case 0x0011:	// DVI/Intel ADPCM
			mInternals->mAudioStorageFormat =
					CDVIIntelIMAADPCMAudioCodec::composeAudioStorageFormat((Float32) waveFormat.getSamplesPerSecond(),
							AUDIOCHANNELMAP_FORUNKNOWN(waveFormat.getChannels()));
			return true;

		default:
			// Not supported
			return false;
	}
}

//----------------------------------------------------------------------------------------------------------------------
bool CWAVEMediaSourceImportTracker::canFinalize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mAudioStorageFormat.hasInstance() && mDataChunkStartByteOffset.hasValue() &&
			mDataChunkByteCount.hasValue();
}

//----------------------------------------------------------------------------------------------------------------------
CAudioTrack CWAVEMediaSourceImportTracker::composeAudioTrack()
//----------------------------------------------------------------------------------------------------------------------
{
	return composeAudioTrack(*mInternals->mSampleSize);
}

//----------------------------------------------------------------------------------------------------------------------
I<CDecodeAudioCodec> CWAVEMediaSourceImportTracker::createAudioCodec(const I<CSeekableDataSource>& seekableDataSource)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check format tag
	switch (*mInternals->mFormatTag) {
		case 0x0001:	// Integer PCM
		case 0x0003:	// IEEE Float
			return CPCMAudioCodec::create(*mInternals->mAudioStorageFormat, seekableDataSource,
					*mDataChunkStartByteOffset, *mDataChunkByteCount,
					(*mInternals->mAudioStorageFormat->getBits() > 8) ?
							CPCMAudioCodec::kFormatLittleEndian : CPCMAudioCodec::kFormat8BitUnsigned);

		case 0x0011:	// DVI/Intel ADPCM
			return CDVIIntelIMAADPCMAudioCodec::create(*mInternals->mAudioStorageFormat, seekableDataSource,
					*mDataChunkStartByteOffset, *mDataChunkByteCount, *mInternals->mBlockAlign);

		default:		// Not possible
			AssertFail();
			return I<CDecodeAudioCodec>(nil);
	}
}

// MARK: Subclass methods

//----------------------------------------------------------------------------------------------------------------------
CAudioTrack CWAVEMediaSourceImportTracker::composeAudioTrack(UInt16 sampleSize)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check format tag
	UInt64	frameCount;
	switch (*mInternals->mFormatTag) {
		case 0x0001:	// Integer PCM
		case 0x0003:	// IEEE Float
			frameCount = CPCMAudioCodec::composeFrameCount(*mInternals->mAudioStorageFormat, *mDataChunkByteCount);
			break;

		case 0x0011:	// DVI/Intel ADPCM
			frameCount =
					CDVIIntelIMAADPCMAudioCodec::composeFrameCount(*mInternals->mAudioStorageFormat,
							*mDataChunkByteCount, *mInternals->mBlockAlign);
			break;

		default:		// Not possible
			AssertFail();
			frameCount = 0;
			break;
	}

	return CAudioTrack(CAudioTrack::composeInfo(*mInternals->mAudioStorageFormat, frameCount, *mDataChunkByteCount),
			*mInternals->mAudioStorageFormat);
}
