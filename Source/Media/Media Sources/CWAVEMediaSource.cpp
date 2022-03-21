//----------------------------------------------------------------------------------------------------------------------
//	CWAVEMediaSource.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CWAVEMediaSource.h"

#include "CDVIIntelIMAADPCMAudioCodec.h"
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
		SMediaSource::Options options)
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
	OV<UInt16>	sampleSize;
	while (true) {
		// Read next chunk info
		TIResult<CChunkReader::ChunkInfo>	chunkInfo = chunkReader->readChunkInfo();
		if (chunkInfo.hasError())
			// Done reading chunks
			break;

		// What did we get?
//		if (chunkInfo.getValue().getID().hasValue()) {
			// Check chunk type by ID
			switch (chunkInfo.getValue().getID()) {
				case kWAVEFormatChunkID: {
					// Format chunk
					TIResult<CData>	readPayload = chunkReader->readPayload(chunkInfo.getValue());
					ReturnValueIfResultError(readPayload, SMediaSource::QueryTracksResult(readPayload.getError()));

					const	SWAVEFORMAT&	waveFormat = *((SWAVEFORMAT*) readPayload.getValue().getBytePtr());

					if (readPayload.getValue().getByteCount() >= sizeof(SWAVEFORMATEX)) {
						// Have WAVEFORMATEX
						const	SWAVEFORMATEX&	waveFormatEx = *((SWAVEFORMATEX*) readPayload.getValue().getBytePtr());
						sampleSize = OV<UInt16>(waveFormatEx.getBitsPerSample());
					} else if ((waveFormat.getChannels() > 0) && (waveFormat.getSamplesPerSecond() > 0) &&
							(waveFormat.getAverageBytesPerSec() > 0))
						// Use WAVEFORMAT
						sampleSize =
								OV<UInt16>(
										(UInt16) (waveFormat.getAverageBytesPerSec() / waveFormat.getSamplesPerSecond())
												/ waveFormat.getChannels() * 8);

					if (!waveMediaSourceImportTracker->note(waveFormat, sampleSize, readPayload.getValue()))
						// Not supported
						return SMediaSource::QueryTracksResult(
								SError(mErrorDomain, kUnsupportedCodecCode,
										CString(OSSTR("Unsupported codec: ")) +
												CString(waveFormat.getFormatTag(), 4, true, true)));
				} break;

				default:
					// Other
					waveMediaSourceImportTracker->note(chunkInfo.getValue(), *chunkReader);
			}
//		} else {
//			// Check chunk type by UUID
//		}

		// Seek to next chunk
		error = chunkReader->seekToNext(chunkInfo.getValue());
		if (error.hasInstance())
			// Done reading chunks
			break;
	}

	// Check results
	if (!waveMediaSourceImportTracker->canFinalize())
		// Can't finalize
		return SMediaSource::QueryTracksResult(sInvalidWAVEFileError);

	// Finalize setup
	CAudioTrack			audioTrack = waveMediaSourceImportTracker->composeAudioTrack(*sampleSize);
	CMediaTrackInfos	mediaTrackInfos;
	if (options & SMediaSource::kComposeDecodeInfo)
		// Requesting decode info
		mediaTrackInfos.add(
				CMediaTrackInfos::AudioTrackInfo(audioTrack,
						waveMediaSourceImportTracker->composeDecodeInfo(seekableDataSource)));
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
	SWAVEFORMChunk32	formChunk32;
	OI<SError>			error = seekableDataSource->readData(0, &formChunk32, sizeof(SWAVEFORMChunk32));
	ReturnValueIfError(error, OI<CChunkReader>());

	if ((formChunk32.getID() == kWAVEFORMChunkID) && (formChunk32.getFormType() == kWAVEFORMType)) {
		// Success
		OI<CChunkReader>	chunkReader(new CChunkReader(seekableDataSource, false));
		chunkReader->setPos(CByteReader::kPositionFromBeginning, sizeof(SWAVEFORMChunk32));

		return chunkReader;
	}

	return OI<CChunkReader>();
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
void CWAVEMediaSourceImportTracker::note(const CChunkReader::ChunkInfo& chunkInfo, CChunkReader& chunkReader)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check chunk ID
	switch (chunkInfo.getID()) {
		case kWAVEDataChunkID:
			// Data chunk
			mDataChunkStartByteOffset = OV<UInt64>(chunkInfo.getThisChunkPos());
			mDataChunkByteCount =
					OV<UInt64>(std::min<SInt64>(chunkInfo.getByteCount(),
							chunkReader.getByteCount() - *mDataChunkStartByteOffset));
			break;
	}
}

//----------------------------------------------------------------------------------------------------------------------
bool CWAVEMediaSourceImportTracker::canFinalize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mDataChunkStartByteOffset.hasValue() && mDataChunkByteCount.hasValue() &&
			mInternals->mAudioStorageFormat.hasInstance();
}

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
							*mDataChunkByteCount);
			break;

		default:		// Not possible
			AssertFail();
			frameCount = 0;
			break;
	}

	return CAudioTrack(CAudioTrack::composeInfo(*mInternals->mAudioStorageFormat, frameCount, *mDataChunkByteCount),
			*mInternals->mAudioStorageFormat);
}

//----------------------------------------------------------------------------------------------------------------------
I<CCodec::DecodeInfo> CWAVEMediaSourceImportTracker::composeDecodeInfo(const I<CSeekableDataSource>& seekableDataSource)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check format tag
	switch (*mInternals->mFormatTag) {
		case 0x0001:	// Integer PCM
		case 0x0003:	// IEEE Float
			return CPCMAudioCodec::composeDecodeInfo(*mInternals->mAudioStorageFormat, seekableDataSource,
					*mDataChunkStartByteOffset, *mDataChunkByteCount, true);

		case 0x0011:	// DVI/Intel ADPCM
			return CDVIIntelIMAADPCMAudioCodec::composeDecodeInfo(*mInternals->mAudioStorageFormat, seekableDataSource,
					*mDataChunkStartByteOffset, *mDataChunkByteCount);

		default:		// Not possible
			AssertFail();
			return I<CCodec::DecodeInfo>(nil);
	}
}
