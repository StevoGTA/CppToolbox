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
	CChunkReader						chunkReader(seekableDataSource, false);
	I<CWAVEMediaSourceImportTracker>	waveMediaSourceImportTracker = CWAVEMediaSourceImportTracker::instantiate();
	OI<SError>							error;

	// Verify it's a WAVE Media Source
	SWAVEFORMChunk32	formChunk32;
	error = chunkReader.CByteReader::readData(&formChunk32, sizeof(SWAVEFORMChunk32));
	ReturnValueIfError(error, SMediaSource::QueryTracksResult());
	if ((formChunk32.getID() != kWAVEFORMChunkID) || (formChunk32.getFormType() != kWAVEFORMType))
		// Not a WAVE file
		return SMediaSource::QueryTracksResult();

	// Process chunks
	OV<UInt16>	sampleSize;
	OV<UInt64>	dataChunkStartByteOffset;
	OV<UInt64>	dataChunkByteCount;
	while (true) {
		// Read next chunk info
		TIResult<CChunkReader::ChunkInfo>	chunkInfo = chunkReader.readChunkInfo();
		if (chunkInfo.hasError())
			// Done reading chunks
			break;

		// What did we get?
		if (chunkInfo.getValue().mID.hasValue()) {
			// Check chunk type by ID
			switch (*chunkInfo.getValue().mID) {
				case kWAVEFormatChunkID: {
					// Format chunk
					TIResult<CData>	readPayload = chunkReader.readPayload(chunkInfo.getValue());
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

					if (!waveMediaSourceImportTracker->note(waveFormat, sampleSize))
						// Not supported
						return SMediaSource::QueryTracksResult(
								SError(mErrorDomain, kUnsupportedCodecCode,
										CString(OSSTR("Unsupported codec: ")) +
												CString(waveFormat.getFormatTag(), 4, true, true)));
				} break;

				case kWAVEDataChunkID:
					// Data chunk
					dataChunkStartByteOffset = OV<UInt64>(chunkInfo.getValue().mThisChunkPos);
					dataChunkByteCount =
							OV<UInt64>(std::min<SInt64>(chunkInfo.getValue().mByteCount,
									chunkReader.getByteCount() - *dataChunkStartByteOffset));
					break;

				default:
					// Other
					waveMediaSourceImportTracker->note(chunkInfo.getValue());
			}
		} else {
			// Check chunk type by UUID
		}

		// Seek to next chunk
		error = chunkReader.seekToNext(chunkInfo.getValue());
		if (error.hasInstance())
			// Done reading chunks
			break;
	}

	// Check results
	if (!dataChunkStartByteOffset.hasValue() || !dataChunkByteCount.hasValue() ||
			!waveMediaSourceImportTracker->canFinalize())
		// Can't finalize
		return SMediaSource::QueryTracksResult(sInvalidWAVEFileError);

	// Finalize setup
	CAudioTrack			audioTrack = waveMediaSourceImportTracker->composeAudioTrack(*sampleSize, *dataChunkByteCount);
	CMediaTrackInfos	mediaTrackInfos;
	if (options & SMediaSource::kComposeDecodeInfo)
		// Requesting decode info
		mediaTrackInfos.add(
				CMediaTrackInfos::AudioTrackInfo(audioTrack,
						waveMediaSourceImportTracker->composeDecodeInfo(seekableDataSource, *dataChunkStartByteOffset,
								*dataChunkByteCount)));
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
bool CWAVEMediaSourceImportTracker::note(const SWAVEFORMAT& waveFormat, const OV<UInt16>& sampleSize)
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
	return mInternals->mAudioStorageFormat.hasInstance();
}

//----------------------------------------------------------------------------------------------------------------------
CAudioTrack CWAVEMediaSourceImportTracker::composeAudioTrack(UInt16 sampleSize, UInt64 dataChunkByteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check format tag
	UInt64	frameCount;
	switch (*mInternals->mFormatTag) {
		case 0x0001:	// Integer PCM
		case 0x0003:	// IEEE Float
			frameCount = CPCMAudioCodec::composeFrameCount(*mInternals->mAudioStorageFormat, dataChunkByteCount);
			break;

		case 0x0011:	// DVI/Intel ADPCM
			frameCount =
					CDVIIntelIMAADPCMAudioCodec::composeFrameCount(*mInternals->mAudioStorageFormat,
							dataChunkByteCount);
			break;

		default:		// Not possible
			AssertFail();
			frameCount = 0;
			break;
	}

	return CAudioTrack(CAudioTrack::composeInfo(*mInternals->mAudioStorageFormat, frameCount, dataChunkByteCount),
			*mInternals->mAudioStorageFormat);
}

//----------------------------------------------------------------------------------------------------------------------
I<CCodec::DecodeInfo> CWAVEMediaSourceImportTracker::composeDecodeInfo(const I<CSeekableDataSource>& seekableDataSource,
		UInt64 dataChunkStartByteOffset, UInt64 dataChunkByteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check format tag
	switch (*mInternals->mFormatTag) {
		case 0x0001:	// Integer PCM
		case 0x0003:	// IEEE Float
			return CPCMAudioCodec::composeDecodeInfo(*mInternals->mAudioStorageFormat, seekableDataSource,
					dataChunkStartByteOffset, dataChunkByteCount, true);

		case 0x0011:	// DVI/Intel ADPCM
			return CDVIIntelIMAADPCMAudioCodec::composeDecodeInfo(*mInternals->mAudioStorageFormat, seekableDataSource,
					dataChunkStartByteOffset, dataChunkByteCount);

		default:		// Not possible
			AssertFail();
			return I<CCodec::DecodeInfo>(nil);
	}
}
