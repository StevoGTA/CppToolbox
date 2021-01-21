//----------------------------------------------------------------------------------------------------------------------
//	CWAVEMediaSource.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CWAVEMediaSource.h"

#include "CDVIIntelIMAADPCMAudioCodec.h"
#include "SWAVEMediaInfo.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sErrorDomain(OSSTR("CWAVEMediaSource"));
static	SError	sNotAWAVEFileError(sErrorDomain, 1, CString(OSSTR("Not a WAVE file")));
static	SError	sUnsupportedCodecError(sErrorDomain, 2, CString(OSSTR("Unsupported codec")));

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CWAVEMediaSourceInternals

class CWAVEMediaSourceInternals {
	public:
		CWAVEMediaSourceInternals() {}

		TNArray<CAudioTrack>	mAudioTracks;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CWAVEMediaSource

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CWAVEMediaSource::CWAVEMediaSource(const CByteParceller& byteParceller) :
	CChunkMediaSource(byteParceller, kLittleEndian)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CWAVEMediaSourceInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CWAVEMediaSource::~CWAVEMediaSource()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CMediaSource methods

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CWAVEMediaSource::loadTracks()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	OI<SError>	error;

	// Reset to beginning
	error = reset();
	ReturnErrorIfError(error);

	// Verify it's a WAVE Media Source
	SWAVEFORMChunk32	formChunk32;
	error = mByteParceller.readData(&formChunk32, sizeof(SWAVEFORMChunk32));
	ReturnErrorIfError(error);
	if ((formChunk32.getID() != kWAVEFORMChunkID) || (formChunk32.getFormType() != kWAVEFORMType))
		// Not a WAVE file
		return OI<SError>(sNotAWAVEFileError);

	// Process chunks
	OI<SAudioStorageFormat>	audioStorageFormat;
	SInt64					dataStartOffset = 0;
	SInt64					dataSize = 0;
	while (true) {
		// Read next chunk info
		OI<ChunkInfo>	chunkInfo = getChunkInfo(error);
		ReturnErrorIfError(error);

		// What did we get?
		switch (chunkInfo->mID) {
			case kWAVEFormatChunkID: {
				// Format chunk
				OI<CData>	data = getChunk(*chunkInfo, error);
				ReturnErrorIfError(error);

				const	SWAVEFORMAT&	waveFormat = *((SWAVEFORMAT*) data->getBytePtr());

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
						return OI<SError>(sUnsupportedCodecError);
				}
			} break;

			case kWAVEDataChunkID:
				// Data chunk
				dataStartOffset = mByteParceller.getPos();
				dataSize = std::min<SInt64>(chunkInfo->mSize, mByteParceller.getSize() - dataStartOffset);
				break;
		}

		// Check if done
		if ((audioStorageFormat.hasInstance() && (dataStartOffset != 0) && (dataSize != 0)))
			// Done
			break;

		// Seek to next chunk
		error = seekToNextChunk(*chunkInfo);
		ReturnErrorIfError(error);
	}

	// Store
	mInternals->mAudioTracks +=
			CAudioTrack(*audioStorageFormat,
					I<CAudioCodec::DecodeInfo>(new CAudioCodec::DataDecodeInfo(dataStartOffset, dataSize)));

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CAudioTrack> CWAVEMediaSource::getAudioTracks()
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mAudioTracks;
}
