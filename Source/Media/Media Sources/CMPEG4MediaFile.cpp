//----------------------------------------------------------------------------------------------------------------------
//	CMPEG4MediaFile.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMPEG4MediaFile.h"

#include "CAACAudioCodec.h"
#include "CMediaSourceRegistry.h"
#include "CH264VideoCodec.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

/*
	Info on MPEG 4 files can be found here:
		http://www.geocities.com/xhelmboyx/quicktime/formats/mp4-layout.txt (no longer active)
		http://atomicparsley.sourceforge.net/mpeg-4files.html

	Terms:
		We retain "packet" from AudioFrames and VideoFrame to mean an individual unit of compressed data comprising 1 or
			more compressed frames.

		"chunk" is a group of packets which are all stored together and which may be separated from other chunks of
			packets by opaque data.  All packets in a chunk will have the same duration and sample description.

	Blocks are groups of packets.
	In the stsc Atom, packet counts are associated with blocks.  Blocks that have the same
		number of packets are collected together.  Each group of blocks gets an entry.  The
		number of blocks in the group is indicated by the next block group start index, or for
		the last block, it's just 1.
	In the stco Atom, specify file offsets for the start of data for each block.

	How packets are grouped into blocks seems arbitrary at this point
*/

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMPEG4MediaFile

// MARK: Properties

OSType	CMPEG4MediaFile::mID = MAKE_OSTYPE('M', 'P', '4', ' ');

// MARK: Internals

struct CMPEG4MediaFile::Internals {

								Internals(const CAtomReader& atomReader, const CAtomReader::Atom& stsdAtom,
										const CAtomReader::ContainerAtom& stblContainerAtom,
										const SMPEG4::STSDDescriptionHeader& stsdDescriptionHeader,
										const SMPEG4::STTSAtomPayload& sttsAtomPayload,
										const SMPEG4::STSCAtomPayload& stscAtomPayload,
										const SMPEG4::STSZAtomPayload& stszAtomPayload,
										SMPEG4::STCOAtomPayload* stcoAtomPayload,
										SMPEG4::CO64AtomPayload* co64AtomPayload) :
									mAtomReader(atomReader), mSTSDAtom(stsdAtom), mSTBLContainerAtom(stblContainerAtom),
											mSTSDDescriptionHeader(stsdDescriptionHeader),
											mSTTSAtomPayload(sttsAtomPayload), mSTSCAtomPayload(stscAtomPayload),
											mSTSZAtomPayload(stszAtomPayload), mSTCOAtomPayload(stcoAtomPayload),
											mCO64AtomPayload(co64AtomPayload)
									{}

			TVResult<CData>		getDecompressionData(SInt64 offset) const
									{
										// Read atom
										TVResult<CAtomReader::Atom>	atom =
																			mAtomReader.readAtom(mSTSDAtom,
																					sizeof(SMPEG4::STSDAtomPayload) +
																							offset);
										ReturnValueIfResultError(atom, TVResult<CData>(atom.getError()));

										return mAtomReader.readAtomPayload(*atom);
									}

			UInt32				getPacketGroupOffsetCount() const
									{
										return (mSTCOAtomPayload != nil) ?
												mSTCOAtomPayload->getPacketGroupOffsetCount() :
												mCO64AtomPayload->getPacketGroupOffsetCount();
									}
			UInt64				getPacketGroupOffset(UInt32 stcoBlockOffsetIndex) const
									{
										return (mSTCOAtomPayload != nil) ?
												mSTCOAtomPayload->getPacketGroupOffset(stcoBlockOffsetIndex) :
												mCO64AtomPayload->getPacketGroupOffset(stcoBlockOffsetIndex);
									}

	const	CAtomReader&					mAtomReader;
	const	CAtomReader::Atom&				mSTSDAtom;
	const	CAtomReader::ContainerAtom&		mSTBLContainerAtom;
	const	SMPEG4::STSDDescriptionHeader&	mSTSDDescriptionHeader;
	const	SMPEG4::STTSAtomPayload&		mSTTSAtomPayload;
	const	SMPEG4::STSCAtomPayload&		mSTSCAtomPayload;
	const	SMPEG4::STSZAtomPayload&		mSTSZAtomPayload;
			SMPEG4::STCOAtomPayload* 		mSTCOAtomPayload;
			SMPEG4::CO64AtomPayload*		mCO64AtomPayload;
};

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
I<SMediaSource::ImportResult> CMPEG4MediaFile::import(const SMediaSource::ImportSetup& importSetup)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CAtomReader	atomReader(importSetup.getRandomAccessDataSource());
	OV<SError>	error;

	// Read ftyp atom
	TVResult<CAtomReader::Atom>	atom = atomReader.readAtom();
	ReturnValueIfResultError(atom, I<SMediaSource::ImportResult>(new SMediaSource::ImportResult()));
	if (atom->getType() != MAKE_OSTYPE('f', 't', 'y', 'p'))
		return I<SMediaSource::ImportResult>(new SMediaSource::ImportResult());

	// Find moov atom
	while (atom->getType() != MAKE_OSTYPE('m', 'o', 'o', 'v')) {
		// Go to next atom
		error = atomReader.seekToNextAtom(*atom);
		ReturnValueIfError(error, I<SMediaSource::ImportResult>(new SMediaSource::ImportResult(*error)));

		// Get atom
		atom = atomReader.readAtom();
		ReturnValueIfResultError(atom, I<SMediaSource::ImportResult>(new SMediaSource::ImportResult(atom.getError())));
	}

	TVResult<CAtomReader::ContainerAtom>	moovContainerAtom = atomReader.readContainerAtom(*atom);
	ReturnValueIfResultError(moovContainerAtom,
			I<SMediaSource::ImportResult>(new SMediaSource::ImportResult(moovContainerAtom.getError())));

	// Iterate moov atom
	SMediaSource::Tracks	mediaSourceTracks;
	for (TArray<CAtomReader::Atom>::Iterator moovIterator = moovContainerAtom->getIterator(); moovIterator;
			moovIterator++) {
		// Check type
		if (moovIterator->getType() == MAKE_OSTYPE('t', 'r', 'a', 'k')) {
			// Track
			TVResult<CAtomReader::ContainerAtom>	trakContainerAtom = atomReader.readContainerAtom(*moovIterator);
			if (trakContainerAtom.hasError()) continue;

			// Media
			TVResult<CAtomReader::ContainerAtom>	mdiaContainerAtom =
															atomReader.readContainerAtom(
																	trakContainerAtom->getAtom(
																			MAKE_OSTYPE('m', 'd', 'i', 'a')));
			if (mdiaContainerAtom.hasError()) continue;

			// Media header
			TVResult<CData>	mdhdAtomPayloadData =
									atomReader.readAtomPayload(
											mdiaContainerAtom->getAtom(MAKE_OSTYPE('m', 'd', 'h', 'd')));
			if (mdhdAtomPayloadData.hasError()) continue;
			const	SMPEG4::MDHDAtomPayload&	mdhdAtomPayload =
														*((SMPEG4::MDHDAtomPayload*) mdhdAtomPayloadData->getBytePtr());
					UInt32						timeScale = mdhdAtomPayload.getTimeScale();
					UniversalTimeInterval		duration =
														(UniversalTimeInterval) mdhdAtomPayload.getDuration() /
																(UniversalTimeInterval) timeScale;

			// Handler
			TVResult<CData>	hdlrAtomPayloadData =
									atomReader.readAtomPayload(
											mdiaContainerAtom->getAtom(MAKE_OSTYPE('h', 'd', 'l', 'r')));
			if (hdlrAtomPayloadData.hasError()) continue;
			const	SMPEG4::HDLRAtomPayload&	hdlrAtomPayload =
														*((SMPEG4::HDLRAtomPayload*) hdlrAtomPayloadData->getBytePtr());

			// Media Information
			TVResult<CAtomReader::ContainerAtom>	minfContainerAtom =
															atomReader.readContainerAtom(
																	mdiaContainerAtom->getAtom(
																			MAKE_OSTYPE('m', 'i', 'n', 'f')));
			if (minfContainerAtom.hasError()) continue;

			// Sample Table
			TVResult<CAtomReader::ContainerAtom>	stblContainerAtom =
															atomReader.readContainerAtom(
																	minfContainerAtom->getAtom(
																			MAKE_OSTYPE('s', 't', 'b', 'l')));
			if (stblContainerAtom.hasError()) continue;

			// Sample Table Sample Description
			OR<CAtomReader::Atom>	stsdAtom = stblContainerAtom->getAtom(MAKE_OSTYPE('s', 't', 's', 'd'));
			if (!stsdAtom.hasReference()) continue;
			TVResult<CData>	stsdAtomPayloadData = atomReader.readAtomPayload(*stsdAtom);
			if (error.hasValue()) continue;
			const	SMPEG4::STSDAtomPayload&		stsdAtomPayload =
															*((SMPEG4::STSDAtomPayload*)
																	stsdAtomPayloadData->getBytePtr());
			const	SMPEG4::STSDDescriptionHeader&	stsdDescriptionHeader = stsdAtomPayload.getFirstDescriptionHeader();

			// Sample Table Time-to-Sample
			TVResult<CData>	sttsAtomPayloadData =
									atomReader.readAtomPayload(
											stblContainerAtom->getAtom(MAKE_OSTYPE('s', 't', 't', 's')));
			if (sttsAtomPayloadData.hasError()) continue;
			const	SMPEG4::STTSAtomPayload&	sttsAtomPayload =
														*((SMPEG4::STTSAtomPayload*) sttsAtomPayloadData->getBytePtr());

			// Sample Table Sample Blocks
			TVResult<CData>	stscAtomPayloadData =
									atomReader.readAtomPayload(
											stblContainerAtom->getAtom(MAKE_OSTYPE('s', 't', 's', 'c')));
			if (stscAtomPayloadData.hasError()) continue;
			const	SMPEG4::STSCAtomPayload&	stscAtomPayload =
														*((SMPEG4::STSCAtomPayload*) stscAtomPayloadData->getBytePtr());

			// Sample Table Packet Sizes
			TVResult<CData>	stszAtomPayloadData =
									atomReader.readAtomPayload(
											stblContainerAtom->getAtom(MAKE_OSTYPE('s', 't', 's', 'z')));
			if (stszAtomPayloadData.hasError()) continue;
			const	SMPEG4::STSZAtomPayload&	stszAtomPayload =
														*((SMPEG4::STSZAtomPayload*) stszAtomPayloadData->getBytePtr());

			// Sample Table Block offsets
			TVResult<CData>	stcoAtomPayloadData =
									atomReader.readAtomPayload(
											stblContainerAtom->getAtom(MAKE_OSTYPE('s', 't', 'c', 'o')));
			TVResult<CData>	co64AtomPayloadData =
									atomReader.readAtomPayload(
											stblContainerAtom->getAtom(MAKE_OSTYPE('c', 'o', '6', '4')));
			if (!stcoAtomPayloadData.hasValue() && !co64AtomPayloadData.hasValue()) continue;
			SMPEG4::STCOAtomPayload*	stcoAtomPayload =
												stcoAtomPayloadData.hasValue() ?
														(SMPEG4::STCOAtomPayload*) stcoAtomPayloadData->getBytePtr() :
														nil;
			SMPEG4::CO64AtomPayload*	co64AtomPayload =
												co64AtomPayloadData.hasValue() ?
														(SMPEG4::CO64AtomPayload*) co64AtomPayloadData->getBytePtr() :
														nil;

			// Internals
			Internals	internals(atomReader, *stsdAtom, *stblContainerAtom, stsdDescriptionHeader, sttsAtomPayload,
								stscAtomPayload, stszAtomPayload, stcoAtomPayload, co64AtomPayload);

			// Check track type
			if (hdlrAtomPayload.getSubType() == SMPEG4::HDLRAtomPayload::kSubTypeSound) {
				// Audio track
				TVResult<SMediaSource::Tracks::AudioTrack>	audioTrack =
																	composeAudioTrack(importSetup,
																			stsdDescriptionHeader.getType(), duration,
																			internals);
				ReturnValueIfResultError(audioTrack,
						I<SMediaSource::ImportResult>(new SMediaSource::ImportResult(audioTrack.getError())));

				// Success
				mediaSourceTracks.add(*audioTrack);
			} else if (hdlrAtomPayload.getSubType() == SMPEG4::HDLRAtomPayload::kSubTypeVideo) {
				// Video track
				if (importSetup.isImportingVideoTracks()) {
					// Compose video track
					TVResult<SMediaSource::Tracks::VideoTrack>	videoTrack =
																		composeVideoTrack(importSetup,
																				stsdDescriptionHeader.getType(),
																				timeScale, duration, internals);
					ReturnValueIfResultError(videoTrack,
							I<SMediaSource::ImportResult>(new SMediaSource::ImportResult(videoTrack.getError())));

					// Success
					mediaSourceTracks.add(*videoTrack);
				}
			} else {
				// Import track
				error =
						importTrack(importSetup, hdlrAtomPayload.getSubType(), stsdDescriptionHeader, internals);
				if (error.hasValue())
					// Error
					return I<SMediaSource::ImportResult>(new SMediaSource::ImportResult(*error));
			}
		} else if (moovIterator->getType() == MAKE_OSTYPE('u', 'd', 't', 'a')) {
			// User Data
			TVResult<CAtomReader::ContainerAtom>	udtaContainerAtom = atomReader.readContainerAtom(*moovIterator);
			if (!udtaContainerAtom.hasValue())
				continue;

			OR<CAtomReader::Atom>	metaAtom = udtaContainerAtom->getAtom(MAKE_OSTYPE('m', 'e', 't', 'a'));
			TVResult<CData>			metaAtomPayloadData = atomReader.readAtomPayload(metaAtom);
			if (metaAtomPayloadData.hasValue())
				// Process file metadata
				processFileMetadata(*metaAtomPayloadData);
		}
	}

	return I<SMediaSource::ImportResult>(new SMediaSource::ImportResult(mID, mediaSourceTracks));
}

//----------------------------------------------------------------------------------------------------------------------
TArray<SMedia::PacketAndLocation> CMPEG4MediaFile::composePacketAndLocations(const Internals& internals) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	SMPEG4::STTSAtomPayload&	sttsAtomPayload = internals.mSTTSAtomPayload;
	const	SMPEG4::STSCAtomPayload&	stscAtomPayload = internals.mSTSCAtomPayload;
	const	SMPEG4::STSZAtomPayload&	stszAtomPayload = internals.mSTSZAtomPayload;

	// Construct info
	UInt32	stszPacketByteCountIndex = 0;

	UInt32	stscPacketGroupInfoIndex = 0;
	UInt32	stscPacketGroupInfoPacketCount =
					stscAtomPayload.getPacketGroupInfo(stscPacketGroupInfoIndex).getPacketCount();

	UInt32	stcoBlockOffsetIndex = 0;

	UInt32	currentBlockPacketIndex = 0;
	UInt64	currentByteOffset = internals.getPacketGroupOffset(stcoBlockOffsetIndex);

	// Iterate all stts entries
	UInt32								sttsChunkCount = sttsAtomPayload.getChunkCount();
	TNArray<SMedia::PacketAndLocation>	packetAndLocations;
	for (UInt32 sttsChunkIndex = 0; sttsChunkIndex < sttsChunkCount; sttsChunkIndex++) {
		// Get packet info
		const	SMPEG4::STTSAtomPayload::Chunk&	sttsChunk = sttsAtomPayload.getChunk(sttsChunkIndex);
				UInt32							sttsChunkPacketCount = sttsChunk.getPacketCount();
				UInt32							sttsChunkPacketDuration = sttsChunk.getPacketDuration();

		// Iterate packets
		for (UInt32 packetIndex = 0;
				(packetIndex < sttsChunkPacketCount) && (currentByteOffset < internals.mAtomReader.getByteCount());
				packetIndex++, stszPacketByteCountIndex++) {
			// Get info
			UInt32	packetByteCount = stszAtomPayload.getPacketByteCount(stszPacketByteCountIndex);

			// Add Packet Location Info
			packetAndLocations +=
					SMedia::PacketAndLocation(SMedia::Packet(sttsChunkPacketDuration, packetByteCount),
							currentByteOffset);

			// Update
			if (++currentBlockPacketIndex < stscPacketGroupInfoPacketCount)
				// Still more to go in this block
				currentByteOffset += packetByteCount;
			else {
				// Finished with this block
				UInt32	blockOffsetCount = internals.getPacketGroupOffsetCount();
				if (++stcoBlockOffsetIndex < blockOffsetCount) {
					// Update info
					currentBlockPacketIndex = 0;
					currentByteOffset = internals.getPacketGroupOffset(stcoBlockOffsetIndex);

					// Check if have more block groups
					if ((stscPacketGroupInfoIndex + 1) < stscAtomPayload.getPacketGroupInfoCount()) {
						// Check if next block group
						UInt32	nextBlockStartIndex =
										stscAtomPayload.getPacketGroupInfo(stscPacketGroupInfoIndex + 1)
												.getChunkStartIndex();
						if ((stcoBlockOffsetIndex + 1) == nextBlockStartIndex) {
							// Next block group
							stscPacketGroupInfoIndex++;
							stscPacketGroupInfoPacketCount =
									stscAtomPayload.getPacketGroupInfo(stscPacketGroupInfoIndex).getPacketCount();
						}
					}
				}
			}
		}
	}

	return packetAndLocations;
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<SMediaSource::Tracks::AudioTrack> CMPEG4MediaFile::composeAudioTrack(
		const SMediaSource::ImportSetup& importSetup, OSType type, UniversalTimeInterval duration,
		const Internals& internals)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check type
	switch (type) {
		case SMPEG4::STSDMP4ADescription::kType: {
			// MPEG4 (AAC) Audio
			const	SMPEG4::STSDMP4ADescription&	mp4ADescription =
															*((SMPEG4::STSDMP4ADescription*)
																	&internals.mSTSDDescriptionHeader);

			// Get configuration data
			TVResult<CData>	configurationData = internals.getDecompressionData(sizeof(SMPEG4::STSDMP4ADescription));
			ReturnValueIfResultError(configurationData,
					TVResult<SMediaSource::Tracks::AudioTrack>(configurationData.getError()))

			// Compose format
			OV<CAACAudioCodec::Info>	info =
												CAACAudioCodec::composeInfo(*configurationData,
														mp4ADescription.getChannelCount());
			if (!info.hasValue())
				return TVResult<SMediaSource::Tracks::AudioTrack>(
						CCodec::unsupportedConfigurationError(CString(type, true)));

			SAudio::Format						audioFormat = CAACAudioCodec::composeAudioFormat(*info);

			// Compose info
			TArray<SMedia::PacketAndLocation>	mediaPacketAndLocations = composePacketAndLocations(internals);
			UInt64								byteCount =
														SMedia::PacketAndLocation::getTotalByteCount(
																mediaPacketAndLocations);
			SMedia::SegmentInfo					mediaSegmentInfo(duration, byteCount);

			// Add audio track
			if (importSetup.isCreatingDecoders())
				// Add audio track with decode info
				return TVResult<SMediaSource::Tracks::AudioTrack>(
						SMediaSource::Tracks::AudioTrack(audioFormat, mediaSegmentInfo,
								CAACAudioCodec::create(*info, importSetup.getRandomAccessDataSource(),
										mediaPacketAndLocations)));
			else
				// Add audio track
				return TVResult<SMediaSource::Tracks::AudioTrack>(
						SMediaSource::Tracks::AudioTrack(audioFormat, mediaSegmentInfo));
			}

		default:
			// Unsupported audio codec
			return TVResult<SMediaSource::Tracks::AudioTrack>(CCodec::unsupportedError(CString(type, true)));
	}
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<SMediaSource::Tracks::VideoTrack> CMPEG4MediaFile::composeVideoTrack(
		const SMediaSource::ImportSetup& importSetup, OSType type, UInt32 timeScale, UniversalTimeInterval duration,
		const Internals& internals)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check type
	switch (type) {
		case SMPEG4::STSDH264Description::kType: {
			// h.264 Video
			const	SMPEG4::STSDH264Description&	h264Description =
															*((SMPEG4::STSDH264Description*)
																	&internals.mSTSDDescriptionHeader);


			// Get configuration data
			TVResult<CData>	stssAtomPayloadData =
									internals.mAtomReader.readAtomPayload(
											internals.mSTBLContainerAtom.getAtom(MAKE_OSTYPE('s', 't', 's', 's')));
			ReturnValueIfResultError(stssAtomPayloadData,
					TVResult<SMediaSource::Tracks::VideoTrack>(stssAtomPayloadData.getError()));

			// Get configuration data
			TVResult<CData>	configurationData = internals.getDecompressionData(sizeof(SMPEG4::STSDH264Description));
			ReturnValueIfResultError(configurationData,
					TVResult<SMediaSource::Tracks::VideoTrack>(configurationData.getError()))

			// Compose packet and locations
			TArray<SMedia::PacketAndLocation>	mediaPacketAndLocations = composePacketAndLocations(internals);
			Float32								frameRate =
														(Float32) mediaPacketAndLocations.getCount() /
																(Float32) duration;
			UInt64								byteCount =
														SMedia::PacketAndLocation::getTotalByteCount(
																mediaPacketAndLocations);

			// Compose info
			SVideo::Format		videoFormat =
										CH264VideoCodec::composeVideoTrackFormat(
												S2DSizeU16(h264Description.getWidth(), h264Description.getHeight()),
												frameRate);
			SMedia::SegmentInfo	mediaSegmentInfo(duration, byteCount);

			// Add video track
			if (importSetup.isCreatingDecoders()) {
				// Setup
				const	SMPEG4::STSSAtomPayload&	stssAtomPayload =
															*((SMPEG4::STSSAtomPayload*)
																	stssAtomPayloadData->getBytePtr());
						UInt32					keyframesCount = stssAtomPayload.getKeyframesCount();
						TNumberArray<UInt32>	keyframeIndexes;
				for (UInt32 i = 0; i < keyframesCount; i++)
					// Add keyframe index
					keyframeIndexes += stssAtomPayload.getKeyframeIndex(i);

				// Add video track with decode info
				return TVResult<SMediaSource::Tracks::VideoTrack>(
						SMediaSource::Tracks::VideoTrack(videoFormat, mediaSegmentInfo,
								CH264VideoCodec::create(importSetup.getRandomAccessDataSource(),
										mediaPacketAndLocations, *configurationData, timeScale, keyframeIndexes)));
			} else
				// Add video track
				return TVResult<SMediaSource::Tracks::VideoTrack>(
						SMediaSource::Tracks::VideoTrack(videoFormat, mediaSegmentInfo));
			}

		default:
			// Unsupported video codec
			return TVResult<SMediaSource::Tracks::VideoTrack>(CCodec::unsupportedError(CString(type, true)));
	}
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> CMPEG4MediaFile::getDecompressionData(const Internals& internals, SInt64 offset) const
//----------------------------------------------------------------------------------------------------------------------
{
	return internals.getDecompressionData(offset);
}
