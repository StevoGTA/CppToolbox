//----------------------------------------------------------------------------------------------------------------------
//	CWAVEMediaSource.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CChunkReader.h"
#include "CDVIIntelIMAADPCMAudioCodec.h"
#include "CMediaSourceRegistry.h"
#include "CWAVEMediaSource.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sErrorDomain(OSSTR("CWAVEMediaSource"));
static	SError	sNotAWAVEFileError(sErrorDomain, 1, CString(OSSTR("Not a WAVE file")));
static	SError	sInvalidWAVEFileError(sErrorDomain, 2, CString(OSSTR("Invalid WAVE file")));
static	SError	sUnsupportedCodecError(sErrorDomain, 3, CString(OSSTR("Unsupported codec")));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

static	TIResult<CMediaTrackInfos>	sQueryWAVETracksProc(const I<CSeekableDataSource>& seekableDataSource,
											SMediaSource::Options options);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Register media source

CString	sWAVEExtensions[] = { CString(OSSTR("wav")) };

REGISTER_MEDIA_SOURCE(wave,
		SMediaSource(MAKE_OSTYPE('w', 'a', 'v', 'e'), CString(OSSTR("WAVE")),
				TSArray<CString>(sWAVEExtensions, 1), sQueryWAVETracksProc));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDefaultWAVEMediaSourceImportTrackerInternals

class CDefaultWAVEMediaSourceImportTrackerInternals {
	public:
		CDefaultWAVEMediaSourceImportTrackerInternals() {}

		OI<SAudioStorageFormat>	mAudioStorageFormat;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDefaultWAVEMediaSourceImportTracker

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CDefaultWAVEMediaSourceImportTracker::CDefaultWAVEMediaSourceImportTracker() : CWAVEMediaSourceImportTracker()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CDefaultWAVEMediaSourceImportTrackerInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CDefaultWAVEMediaSourceImportTracker::~CDefaultWAVEMediaSourceImportTracker()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
bool CDefaultWAVEMediaSourceImportTracker::note(const SWAVEFORMAT& waveFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check format tag
	switch (waveFormat.getFormatTag()) {
		case 0x0011:
			// DVI/Intel ADPCM
			mInternals->mAudioStorageFormat =
					OI<SAudioStorageFormat>(
							CDVIIntelIMAADPCMAudioCodec::composeAudioStorageFormat(
									(Float32) waveFormat.getSamplesPerSec(),
									AUDIOCHANNELMAP_FORUNKNOWN(waveFormat.getChannels())));
			return true;

		default:
			// Not supported
			return false;
	}
}

//----------------------------------------------------------------------------------------------------------------------
bool CDefaultWAVEMediaSourceImportTracker::canFinalize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mAudioStorageFormat.hasInstance();
}

//----------------------------------------------------------------------------------------------------------------------
CAudioTrack CDefaultWAVEMediaSourceImportTracker::composeAudioTrack(UInt64 dataChunkByteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt64	frameCount =
					CDVIIntelIMAADPCMAudioCodec::composeFrameCount(*mInternals->mAudioStorageFormat,
							dataChunkByteCount);

	return CAudioTrack(CAudioTrack::composeInfo(*mInternals->mAudioStorageFormat, frameCount, dataChunkByteCount),
			*mInternals->mAudioStorageFormat);
}

//----------------------------------------------------------------------------------------------------------------------
I<CCodec::DecodeInfo> CDefaultWAVEMediaSourceImportTracker::composeDecodeInfo(
		const I<CSeekableDataSource>& seekableDataSource, UInt64 dataChunkStartByteOffset, UInt64 dataChunkByteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	return CDVIIntelIMAADPCMAudioCodec::composeDecodeInfo(*mInternals->mAudioStorageFormat, seekableDataSource,
			dataChunkStartByteOffset, dataChunkByteCount);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
TIResult<CMediaTrackInfos> sQueryWAVETracksProc(const I<CSeekableDataSource>& seekableDataSource,
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
	ReturnValueIfError(error, TIResult<CMediaTrackInfos>(*error));
	if ((formChunk32.getID() != kWAVEFORMChunkID) || (formChunk32.getFormType() != kWAVEFORMType))
		// Not a WAVE file
		return TIResult<CMediaTrackInfos>(sNotAWAVEFileError);

	// Process chunks
	UInt64	dataChunkStartByteOffset = 0;
	UInt64	dataChunkByteCount = 0;
	while (true) {
		// Read next chunk info
		TIResult<CChunkReader::ChunkInfo>	chunkInfo = chunkReader.readChunkInfo();
		if (chunkInfo.hasError())
			// Done reading chunks
			break;

		// What did we get?
		switch (chunkInfo.getValue().mID) {
			case kWAVEFormatChunkID: {
				// Format chunk
				TIResult<CData>	data = chunkReader.readData(chunkInfo.getValue());
				ReturnValueIfResultError(data, TIResult<CMediaTrackInfos>(data.getError()));

				const	SWAVEFORMAT&	waveFormat = *((SWAVEFORMAT*) data.getValue().getBytePtr());
				if (!waveMediaSourceImportTracker->note(waveFormat))
					// Not supported
					return TIResult<CMediaTrackInfos>(sUnsupportedCodecError);
			} break;

			case kWAVEDataChunkID:
				// Data chunk
				dataChunkStartByteOffset = chunkReader.getPos();
				dataChunkByteCount =
						std::min<SInt64>(chunkInfo.getValue().mByteCount,
								chunkReader.getByteCount() - dataChunkStartByteOffset);
				break;

			default:
				// Other
				waveMediaSourceImportTracker->note(chunkInfo.getValue());
		}

		// Seek to next chunk
		error = chunkReader.seekToNextChunk(chunkInfo.getValue());
		if (error.hasInstance())
			// Done reading chunks
			break;
	}

	// Check results
	if (!waveMediaSourceImportTracker->canFinalize())
		// Can't finalize
		return TIResult<CMediaTrackInfos>(sInvalidWAVEFileError);

	// Finalize setup
	CAudioTrack			audioTrack = waveMediaSourceImportTracker->composeAudioTrack(dataChunkByteCount);
	CMediaTrackInfos	mediaTrackInfos;
	if (options & SMediaSource::kComposeDecodeInfo)
		// Requesting decode info
		mediaTrackInfos.add(
				CMediaTrackInfos::AudioTrackInfo(audioTrack,
						waveMediaSourceImportTracker->composeDecodeInfo(seekableDataSource, dataChunkStartByteOffset,
								dataChunkByteCount)));
	else
		// Not requesting decode info
		mediaTrackInfos.add(CMediaTrackInfos::AudioTrackInfo(audioTrack));

	return TIResult<CMediaTrackInfos>(mediaTrackInfos);
}
