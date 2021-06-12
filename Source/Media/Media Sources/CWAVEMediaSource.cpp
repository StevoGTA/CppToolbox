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
	OI<SAudioStorageFormat>	audioStorageFormat;
	SInt64					dataStartOffset = 0;
	SInt64					dataSize = 0;
	while (true) {
		// Read next chunk info
		TIResult<CChunkReader::ChunkInfo>	chunkInfo = chunkReader.readChunkInfo();
		ReturnValueIfError(chunkInfo.getError(), TIResult<SMediaTracks>(*chunkInfo.getError()));

		// What did we get?
		switch (chunkInfo.getValue()->mID) {
			case kWAVEFormatChunkID: {
				// Format chunk
				TIResult<CData>	data = chunkReader.readData(*chunkInfo.getValue());
				ReturnValueIfError(data.getError(), TIResult<SMediaTracks>(*data.getError()));

				const	SWAVEFORMAT&	waveFormat = *((SWAVEFORMAT*) data.getValue()->getBytePtr());

				switch (waveFormat.getFormatTag()) {
					case 0x0011:
						// DVI/Intel ADPCM
						audioStorageFormat =
								OI<SAudioStorageFormat>(
										SAudioStorageFormat(CDVIIntelIMAADPCMAudioCodec::mID, 16,
												(Float32) waveFormat.getSamplesPerSec(),
												AUDIOCHANNELMAP_FORUNKNOWN(waveFormat.getChannels())));
						break;

					default:
						// Not supported
						return TIResult<SMediaTracks>(sUnsupportedCodecError);
				}
			} break;

			case kWAVEDataChunkID:
				// Data chunk
				dataStartOffset = chunkReader.getPos();
				dataSize = std::min<SInt64>(chunkInfo.getValue()->mSize, chunkReader.getSize() - dataStartOffset);
				break;
		}

		// Check if done
		if ((audioStorageFormat.hasInstance() && (dataStartOffset != 0) && (dataSize != 0)))
			// Done
			break;

		// Seek to next chunk
		error = chunkReader.seekToNextChunk(*chunkInfo.getValue());
		ReturnValueIfError(error, TIResult<SMediaTracks>(*error));
	}

	// Store
	audioTracks +=
			CAudioTrack(*audioStorageFormat,
					I<CCodec::DecodeInfo>(new CAudioCodec::DataDecodeInfo(dataStartOffset, dataSize)));

	return TIResult<SMediaTracks>(SMediaTracks(audioTracks));
}
