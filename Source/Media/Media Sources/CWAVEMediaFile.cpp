//----------------------------------------------------------------------------------------------------------------------
//	CWAVEMediaFile.cpp			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CWAVEMediaFile.h"

#include "CIMAADPCMAudioCodec.h"
#include "CPCMAudioCodec.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CWAVEMediaFile

// MARK: Properties

CString	CWAVEMediaFile::mErrorDomain(OSSTR("CWAVEMediaFile"));
SError	CWAVEMediaFile::mInvalidWAVEFileError(mErrorDomain, 1, CString(OSSTR("Invalid WAVE file")));

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
OI<CChunkReader> CWAVEMediaFile::createChunkReader(const I<CRandomAccessDataSource>& randomAccessDataSource)
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify it's a WAVE Media Source
	if (!randomAccessDataSource->canReadData(0, sizeof(SWAVEFORMChunk32)))
		return OI<CChunkReader>();
	TIResult<CData>	data = randomAccessDataSource->readData(0, sizeof(SWAVEFORMChunk32));
	ReturnValueIfResultError(data, OI<CChunkReader>());

	const	SWAVEFORMChunk32&	formChunk32 = *((SWAVEFORMChunk32*) data->getBytePtr());
	if ((formChunk32.getID() == kWAVEFORMChunkID) && (formChunk32.getFormType() == kWAVEFORMType)) {
		// Success
		OI<CChunkReader>	chunkReader(
									new CChunkReader(randomAccessDataSource, CChunkReader::kFormat32BitLittleEndian));
		chunkReader->setPos(CByteReader::kPositionFromBeginning, sizeof(SWAVEFORMChunk32));

		return chunkReader;
	} else
		// Don't know how to proceed
		return OI<CChunkReader>();
}

//----------------------------------------------------------------------------------------------------------------------
SMediaSource::ImportResult CWAVEMediaFile::import(CChunkReader& chunkReader,
		const CChunkReader::ChunkInfo& formatChunkInfo, const CChunkReader::ChunkInfo& dataChunkInfo,
		const TArray<CChunkReader::ChunkInfo>& otherChunkInfos, SMediaSource::Options options) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Process Format chunk
	TIResult<CData>	payload = chunkReader.readPayload(formatChunkInfo);
	ReturnValueIfResultError(payload, SMediaSource::ImportResult(payload.getError()));

	const	SWAVEFORMAT&	waveFormat = *((SWAVEFORMAT*) payload->getBytePtr());
			UInt16			sampleSize;
	if (payload->getByteCount() >= sizeof(SWAVEFORMATEX)) {
		// Have WAVEFORMATEX
		const	SWAVEFORMATEX&	waveFormatEx = *((SWAVEFORMATEX*) payload->getBytePtr());
		sampleSize = waveFormatEx.getBitsPerSample();
	} else if ((waveFormat.getChannels() > 0) && (waveFormat.getSamplesPerSecond() > 0) &&
			(waveFormat.getAverageBytesPerSec() > 0))
		// Use WAVEFORMAT
		sampleSize =
				(UInt16) (waveFormat.getAverageBytesPerSec() / waveFormat.getSamplesPerSecond()) /
						waveFormat.getChannels() * 8;
	else
		// ???
		return SMediaSource::ImportResult(
				CCodec::unsupportedConfigurationError(CString(waveFormat.getFormatTag(), 4, true, true)));

	// Process Data chunk
	UInt64	dataChunkByteCount =
					std::min<SInt64>(dataChunkInfo.getByteCount(),
							chunkReader.getByteCount() - dataChunkInfo.getThisChunkPos());

	// Process by format tag
	OI<SAudioStorageFormat>		audioStorageFormat;
	OI<I<CDecodeAudioCodec> >	decodeAudioCodec;
	UInt64						frameCount;
	switch (waveFormat.getFormatTag()) {
		case 0x0000:	// Illegal/Unknown
			return SMediaSource::ImportResult(
					CCodec::unsupportedConfigurationError(CString(waveFormat.getFormatTag(), 4, true, true)));

		case 0x0001:	// Integer PCM
			if (sampleSize > 32)
				// Nope
				return SMediaSource::ImportResult(
						CCodec::unsupportedConfigurationError(CString(waveFormat.getFormatTag(), 4, true, true)));

			audioStorageFormat =
					CPCMAudioCodec::composeAudioStorageFormat(false, (UInt8) sampleSize,
							(Float32) waveFormat.getSamplesPerSecond(), (UInt8) waveFormat.getChannels());
			frameCount = CPCMAudioCodec::composeFrameCount(*audioStorageFormat, dataChunkByteCount);
			if (options & SMediaSource::kCreateDecoders)
				// Create
				decodeAudioCodec =
						CPCMAudioCodec::create(*audioStorageFormat, chunkReader.getRandomAccessDataSource(),
								dataChunkInfo.getThisChunkPos(), dataChunkByteCount,
								(*audioStorageFormat->getBits() > 8) ?
										CPCMAudioCodec::kFormatLittleEndian : CPCMAudioCodec::kFormat8BitUnsigned);
			break;

		case 0x0003:	// IEEE Float
			if (sampleSize > 32)
				// Nope
				return SMediaSource::ImportResult(
						CCodec::unsupportedConfigurationError(CString(waveFormat.getFormatTag(), 4, true, true)));

			audioStorageFormat =
					CPCMAudioCodec::composeAudioStorageFormat(true, (UInt8) sampleSize,
							(Float32) waveFormat.getSamplesPerSecond(), (UInt8) waveFormat.getChannels());
			frameCount = CPCMAudioCodec::composeFrameCount(*audioStorageFormat, dataChunkByteCount);
			if (options & SMediaSource::kCreateDecoders)
				// Create
				decodeAudioCodec =
						CPCMAudioCodec::create(*audioStorageFormat, chunkReader.getRandomAccessDataSource(),
								dataChunkInfo.getThisChunkPos(), dataChunkByteCount,
								(*audioStorageFormat->getBits() > 8) ?
										CPCMAudioCodec::kFormatLittleEndian : CPCMAudioCodec::kFormat8BitUnsigned);
			break;

		case 0x0011:	// DVI/Intel ADPCM
			audioStorageFormat =
					CDVIIntelIMAADPCMAudioCodec::composeAudioStorageFormat((Float32) waveFormat.getSamplesPerSecond(),
							AUDIOCHANNELMAP_FORUNKNOWN(waveFormat.getChannels()));
			frameCount =
					CDVIIntelIMAADPCMAudioCodec::composeFrameCount(*audioStorageFormat, dataChunkByteCount,
							waveFormat.getBlockAlign());
			if (options & SMediaSource::kCreateDecoders)
				// Create
				decodeAudioCodec =
						CDVIIntelIMAADPCMAudioCodec::create(*audioStorageFormat,
								chunkReader.getRandomAccessDataSource(), dataChunkInfo.getThisChunkPos(),
								dataChunkByteCount, waveFormat.getBlockAlign());
			break;

		default:
			// Not supported
			return SMediaSource::ImportResult(
					CCodec::unsupportedConfigurationError(CString(waveFormat.getFormatTag(), 4, true, true)));
	}

	// Finalize
	CAudioTrack			audioTrack(CAudioTrack::composeInfo(*audioStorageFormat, frameCount, dataChunkByteCount),
								*audioStorageFormat);
	CMediaTrackInfos	mediaTrackInfos;
	mediaTrackInfos.add(CMediaTrackInfos::AudioTrackInfo(audioTrack, decodeAudioCodec));

	return SMediaSource::ImportResult(mediaTrackInfos);
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
SMediaSource::ImportResult CWAVEMediaFile::import(const I<CRandomAccessDataSource>& randomAccessDataSource,
		const OI<CAppleResourceManager>& appleResourceManager, SMediaSource::Options options)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	I<CWAVEMediaFile>	waveMediaFile = create();
	OI<CChunkReader>	chunkReader = waveMediaFile->createChunkReader(randomAccessDataSource);
	if (!chunkReader.hasInstance())
		// Not a WAVE file
		return SMediaSource::ImportResult();

	// Process chunks
	OI<SError>	error;
	OV<CChunkReader::ChunkInfo>			formatChunkInfo;
	OV<CChunkReader::ChunkInfo>			dataChunkInfo;
	TNArray<CChunkReader::ChunkInfo>	otherChunkInfos;
	while (true) {
		// Read next chunk info
		TIResult<CChunkReader::ChunkInfo>	chunkInfo = chunkReader->readChunkInfo();
		if (chunkInfo.hasError())
			// Done reading chunks
			break;

		// Check chunk type
		if (waveMediaFile->isFormatChunk(*chunkInfo))
			// Format chunk
			formatChunkInfo = OV<CChunkReader::ChunkInfo>(*chunkInfo);
		else if (waveMediaFile->isDataChunk(*chunkInfo))
			// Data chunk
			dataChunkInfo = OV<CChunkReader::ChunkInfo>(*chunkInfo);
		else
			// Other chunk
			otherChunkInfos += *chunkInfo;

		// Seek to next chunk
		error = chunkReader->seekToNext(*chunkInfo);
		if (error.hasInstance())
			// Done reading chunks
			break;
	}

	// Check results
	if (!formatChunkInfo.hasValue() || !dataChunkInfo.hasValue())
		// Did not get requisite format chunk or data chunk
		return SMediaSource::ImportResult(mInvalidWAVEFileError);

	return waveMediaFile->import(*chunkReader, *formatChunkInfo, *dataChunkInfo, otherChunkInfos, options);
}
