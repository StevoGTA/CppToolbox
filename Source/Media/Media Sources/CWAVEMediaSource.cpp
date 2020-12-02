//----------------------------------------------------------------------------------------------------------------------
//	CWAVEMediaSource.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CWAVEMediaSource.h"

#include "CDVIIntelIMAADPCMAudioCodec.h"
#include "SWAVEMediaInfo.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sErrorDomain(OSSTR("CWAVEMediaSource"));
static	SError	sUnableToLoadTracks(sErrorDomain, 1, CString(OSSTR("Unable to load tracks")));
static	SError	sUnsupportedFormat(sErrorDomain, 2, CString(OSSTR("Unsupported format")));

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
	UError	error;

	// Reset to beginning
	error = byteParceller.setPos(kDataSourcePositionFromBeginning, 0);
	if (error != kNoError)
		return OI<SError>(sUnableToLoadTracks);

	// Verify it's a WAVE Media Source
	SWAVEFORMChunk32	formChunk32;
	error = byteParceller.readData(&formChunk32, sizeof(SWAVEFORMChunk32));
	if (error != kNoError)
		return OI<SError>(sUnableToLoadTracks);
	if ((formChunk32.getNativeChunkID() != kWAVEFORMChunkID) || (formChunk32.getNativeFormType() != kWAVEFORMType))
		return OI<SError>(sUnableToLoadTracks);

	// Process chunks
	OI<SAudioStorageFormat>	audioStorageFormat;
	SInt64					dataStartOffset = 0;
	SInt64					dataSize = 0;
	while (!audioStorageFormat.hasInstance() || (dataStartOffset == 0) || (dataSize == 0)) {
		// Read next chunk header
		SWAVEChunkHeader32	chunkHeader32;
		error = byteParceller.readData(&chunkHeader32, sizeof(SWAVEChunkHeader32));
		if (error != kNoError)
			return OI<SError>(sUnableToLoadTracks);

		UInt32	chunkDataSize = chunkHeader32.getNativeChunkSize();
		SInt64	nextChunkPos = byteParceller.getPos() + chunkDataSize + (chunkDataSize % 1);

		// What did we get?
		switch (chunkHeader32.getNativeChunkID()) {
			case kWAVEFormatChunkID: {
				// Format chunk
				error = byteParceller.setPos(kDataSourcePositionFromCurrent, -((SInt64) sizeof(SWAVEChunkHeader32)));
				if (error != kNoError)
					return OI<SError>(sUnableToLoadTracks);

				SWAVEFORMAT	waveFormat;
				error = byteParceller.readData(&waveFormat, sizeof(SWAVEFORMAT));
				if (error != kNoError)
					return OI<SError>(sUnableToLoadTracks);

				switch (waveFormat.getNativeFormatTag()) {
					case 0x0011:
						//
						audioStorageFormat =
								OI<SAudioStorageFormat>(
										SAudioStorageFormat(CDVIIntelIMAADPCMAudioCodec::mID, 16,
												waveFormat.getNativeSamplesPerSec(),
												AUDIOCHANNELMAP_FORUNKNOWN(waveFormat.getNativeChannels())));
						break;

					default:
						// Not supported
						return OI<SError>(sUnsupportedFormat);
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
		if (error != kNoError)
			return OI<SError>(sUnableToLoadTracks);
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
