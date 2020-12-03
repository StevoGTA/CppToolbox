//----------------------------------------------------------------------------------------------------------------------
//	CWAVEMediaSource.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CWAVEMediaSource.h"

#include "CDVIIntelIMAADPCMAudioCodec.h"
#include "SWAVEMediaInfo.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sErrorDomain(OSSTR("CWAVEMediaSource"));
static	SError	sNotAWAVEFile(sErrorDomain, 1, CString(OSSTR("Not a WAVE file")));
static	SError	sUnsupportedCodec(sErrorDomain, 2, CString(OSSTR("Unsupported codec")));

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
CWAVEMediaSource::CWAVEMediaSource() : CMediaSource()
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
OI<SError> CWAVEMediaSource::loadTracks(const CByteParceller& byteParceller)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	OI<SError>	error;

	// Reset to beginning
	error = byteParceller.setPos(kDataSourcePositionFromBeginning, 0);
	ReturnErrorIfError(error);

	// Verify it's a WAVE Media Source
	SWAVEFORMChunk32	formChunk32;
	error = byteParceller.readData(&formChunk32, sizeof(SWAVEFORMChunk32));
	ReturnErrorIfError(error);
	if ((formChunk32.getNativeChunkID() != kWAVEFORMChunkID) || (formChunk32.getNativeFormType() != kWAVEFORMType))
		return OI<SError>(sNotAWAVEFile);

	// Process chunks
	OI<SAudioStorageFormat>	audioStorageFormat;
	SInt64					dataStartOffset = 0;
	SInt64					dataSize = 0;
	while (!audioStorageFormat.hasInstance() || (dataStartOffset == 0) || (dataSize == 0)) {
		// Read next chunk header
		SWAVEChunkHeader32	chunkHeader32;
		error = byteParceller.readData(&chunkHeader32, sizeof(SWAVEChunkHeader32));
		ReturnErrorIfError(error);

		UInt32	chunkDataSize = chunkHeader32.getNativeChunkSize();
		SInt64	nextChunkPos = byteParceller.getPos() + chunkDataSize + (chunkDataSize % 1);

		// What did we get?
		switch (chunkHeader32.getNativeChunkID()) {
			case kWAVEFormatChunkID: {
				// Format chunk
				error = byteParceller.setPos(kDataSourcePositionFromCurrent, -((SInt64) sizeof(SWAVEChunkHeader32)));
				ReturnErrorIfError(error);

				SWAVEFORMAT	waveFormat;
				error = byteParceller.readData(&waveFormat, sizeof(SWAVEFORMAT));
				ReturnErrorIfError(error);

				switch (waveFormat.getNativeFormatTag()) {
					case 0x0011:
						// DVI/Intel ADPCM
						audioStorageFormat =
								OI<SAudioStorageFormat>(
										SAudioStorageFormat(CDVIIntelIMAADPCMAudioCodec::mID, 16,
												(Float32) waveFormat.getNativeSamplesPerSec(),
												AUDIOCHANNELMAP_FORUNKNOWN(waveFormat.getNativeChannels())));
						break;

					default:
						// Not supported
						return OI<SError>(sUnsupportedCodec);
				}
			} break;

			case kWAVEDataChunkID:
				// Data chunk
				dataStartOffset = byteParceller.getPos();
				dataSize = std::min<SInt64>(chunkDataSize, byteParceller.getSize() - dataStartOffset);
				break;

			default:
				// Something else
				break;
		}

		// Seek to next chunk
		error = byteParceller.setPos(kDataSourcePositionFromBeginning, nextChunkPos);
		ReturnErrorIfError(error);
	}

	// Store
	mInternals->mAudioTracks +=
			CAudioTrack(*audioStorageFormat,
					I<CAudioCodec::CDecodeInfo>(new CAudioCodec::SDataDecodeInfo(dataStartOffset, dataSize)));

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CAudioTrack> CWAVEMediaSource::getAudioTracks()
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mAudioTracks;
}
