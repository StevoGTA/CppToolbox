//----------------------------------------------------------------------------------------------------------------------
//	CWAVEMediaSource.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CChunkReader.h"
#include "CDVIIntelIMAADPCMAudioCodec.h"
#include "CMediaSourceRegistry.h"
#include "SWAVEMediaInfo.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sErrorDomain(OSSTR("CWAVEMediaSource"));
static	SError	sNotAWAVEFileError(sErrorDomain, 1, CString(OSSTR("Not a WAVE file")));
static	SError	sUnsupportedCodecError(sErrorDomain, 2, CString(OSSTR("Unsupported codec")));

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

static	TIResult<SMediaTracks>	sQueryWAVETracksProc(const I<CSeekableDataSource>& seekableDataSource);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Register media source

CString	sWAVEExtensions[] = { CString(OSSTR("wav")) };

REGISTER_MEDIA_SOURCE(wave,
		SMediaSource::Info(MAKE_OSTYPE('w', 'a', 'v', 'e'), CString(OSSTR("WAVE")),
				TSArray<CString>(sWAVEExtensions, 1), sQueryWAVETracksProc));

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
TIResult<SMediaTracks> sQueryWAVETracksProc(const I<CSeekableDataSource>& seekableDataSource)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CChunkReader			chunkReader(seekableDataSource, false);
	TNArray<CAudioTrack>	audioTracks;

	OI<SError>	error;

	// Verify it's a WAVE Media Source
	SWAVEFORMChunk32	formChunk32;
	error = chunkReader.CByteReader::readData(&formChunk32, sizeof(SWAVEFORMChunk32));
	ReturnValueIfError(error, TIResult<SMediaTracks>(*error));
	if ((formChunk32.getID() != kWAVEFORMChunkID) || (formChunk32.getFormType() != kWAVEFORMType))
		// Not a WAVE file
		return TIResult<SMediaTracks>(sNotAWAVEFileError);

	// Process chunks
	OI<SAudioStorageFormat>				audioStorageFormat;
	UInt64								dataChunkStartByteOffset = 0;
	UInt64								dataChunkByteCount = 0;
	CAudioCodec::ComposeFrameCountProc	composeFrameCountProc = nil;
	CAudioCodec::ComposeDecodeInfoProc	composeDecodeInfoProc = nil;
	while (true) {
		// Read next chunk info
		TIResult<CChunkReader::ChunkInfo>	chunkInfo = chunkReader.readChunkInfo();
		ReturnValueIfResultError(chunkInfo, TIResult<SMediaTracks>(chunkInfo.getError()));

		// What did we get?
		switch (chunkInfo.getValue().mID) {
			case kWAVEFormatChunkID: {
				// Format chunk
				TIResult<CData>	data = chunkReader.readData(chunkInfo.getValue());
				ReturnValueIfResultError(data, TIResult<SMediaTracks>(data.getError()));

				const	SWAVEFORMAT&	waveFormat = *((SWAVEFORMAT*) data.getValue().getBytePtr());
				switch (waveFormat.getFormatTag()) {
					case 0x0011:
						// DVI/Intel ADPCM
						audioStorageFormat =
								OI<SAudioStorageFormat>(
										CDVIIntelIMAADPCMAudioCodec::composeAudioStorageFormat(
												(Float32) waveFormat.getSamplesPerSec(),
												AUDIOCHANNELMAP_FORUNKNOWN(waveFormat.getChannels())));
						composeFrameCountProc = CDVIIntelIMAADPCMAudioCodec::composeFrameCount;
						composeDecodeInfoProc = CDVIIntelIMAADPCMAudioCodec::composeDecodeInfo;
						break;

					default:
						// Not supported
						return TIResult<SMediaTracks>(sUnsupportedCodecError);
				}
			} break;

			case kWAVEDataChunkID:
				// Data chunk
				dataChunkStartByteOffset = chunkReader.getPos();
				dataChunkByteCount =
						std::min<SInt64>(chunkInfo.getValue().mSize, chunkReader.getSize() - dataChunkStartByteOffset);
				break;
		}

		// Check if done
		if ((audioStorageFormat.hasInstance() && (dataChunkStartByteOffset != 0) && (dataChunkByteCount != 0)))
			// Done
			break;

		// Seek to next chunk
		error = chunkReader.seekToNextChunk(chunkInfo.getValue());
		ReturnValueIfError(error, TIResult<SMediaTracks>(*error));
	}

	// Store
	UInt64	frameCount = composeFrameCountProc(*audioStorageFormat, dataChunkByteCount);
	
	audioTracks +=
			CAudioTrack(CAudioTrack::composeInfo(*audioStorageFormat, frameCount, dataChunkByteCount),
					*audioStorageFormat,
					composeDecodeInfoProc(*audioStorageFormat, dataChunkStartByteOffset, dataChunkByteCount));

	return TIResult<SMediaTracks>(SMediaTracks(audioTracks));
}
