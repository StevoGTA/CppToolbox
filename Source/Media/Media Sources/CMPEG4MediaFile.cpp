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

*/

#pragma pack(push, 1)

#if defined(TARGET_OS_WINDOWS)
	#pragma warning(disable:4200)
#endif

// SMP4AtomHeader
struct SMP4AtomHeader {
	// Methods
	public:
				// Lifecycle methods
				SMP4AtomHeader(UInt32 length, OSType type) :
					mLength(EndianU32_NtoB(length)), mType(EndianU32_NtoB(type))
					{}

				// Instance methods
		UInt32	getLength() const
					{ return EndianU32_BtoN(mLength); }
		OSType	getType() const
					{ return EndianU32_BtoN(mType); }
		CData	getData() const
					{ return CData(this, sizeof(SMP4AtomHeader), false); }

	// Properties (in storage endian)
	private:
		UInt32	mLength;
		OSType	mType;
};

// SMP4AudioFormatAtom
struct SMP4AudioFormatAtom {
	// Methods
	public:
				// Instance methods
		UInt32	getLength() const
					{ return EndianU32_BtoN(mLength); }
		OSType	getType() const
					{ return EndianU32_BtoN(mType); }
		OSType	getFormat() const
					{ return EndianU32_BtoN(mFormat); }

	// Properties (in storage endian)
	private:
		UInt32	mLength;
		OSType	mType;
		OSType	mFormat;
};

// SMP4MP4AESDSAtomPayloadHeader
struct SMP4MP4AESDSAtomPayloadHeader {
	// Methods
	public:
				// Lifecycle methods
				SMP4MP4AESDSAtomPayloadHeader() :
					mVersion(0)
					{
						// Setup
						mFlags[0] = 0;
						mFlags[1] = 0;
						mFlags[2] = 0;
					}

				// Instance methods
		CData	getData() const
					{ return CData(this, sizeof(SMP4MP4AESDSAtomPayloadHeader), false); }

	// Properties (in storage endian)
	private:
		UInt8	mVersion;							// 0
		UInt8	mFlags[3];
};

// h.264 Description
struct SMP4stsdH264Description {
	// Type
	enum { kType = MAKE_OSTYPE('a', 'v', 'c', '1') };

	// Methods
	public:
				// Instance methods
		UInt16	getWidth() const
					{ return EndianU16_BtoN(mWidth); }
		UInt16	getHeight() const
					{ return EndianU16_BtoN(mHeight); }

	// Properties (in storage endian)
	private:
		UInt32	mLength;
		OSType	mType;
		UInt8	mReserved[6];
		UInt16	mDataRefIndex;

		UInt16	mQuickTimeEncodingVersion;
		UInt16	mQuickTimeEncodingRevisionLevel;
		OSType	mQuickTimeEncodingVendor;
		UInt32	mTemporalQuality;
		UInt32	mSpatialQuality;
		UInt16	mWidth;
		UInt16	mHeight;
		UInt32	mHorizontalDPI;
		UInt32	mVerticalDPI;
		UInt32	mQuickTimeDataSize;
		UInt16	mFrameCount;
		UInt8	mEncoderNameLength;
		UInt8	mEncoderName[31];
		UInt16	mPixelDepth;
		SInt16	mQuickTimeColorTableID;
		UInt8	mQuickTimeColorTable[];
};

/*
	Blocks are groups of packets.
	In the stsc Atom, packet counts are associated with blocks.  Blocks that have the same
		number of packets are collected together.  Each group of blocks gets an entry.  The
		number of blocks in the group is indicated by the next block group start index, or for
		the last block, it's just 1.
	In the stco Atom, specify file offsets for the start of data for each block.

	How packets are grouped into blocks seems arbitrary at this point
*/

// Sample Table Sync Sample Atom Payload
struct SMP4stssAtomPayload {
	// Methods
	UInt32	getKeyframesCount() const
				{ return EndianU32_BtoN(mKeyframesCount); }
	UInt32	getKeyframeIndex(UInt32 index) const
				{ return EndianU32_BtoN(mKeyFrameIndexes[index]) - 1; }

	// Properties
	private:
		UInt8	mVersion;
		UInt8	mFlags[3];
		UInt32	mKeyframesCount;
		UInt32	mKeyFrameIndexes[];
};

// sample table Chunk Offset 64 Atom Payload
struct SMP4co64AtomPayload {
			// Methods
	UInt32	getPacketGroupOffsetCount() const
				{ return EndianU32_BtoN(mPacketGroupOffsetCount); }
	UInt64	getPacketGroupOffset(UInt32 index) const
				{ return EndianU64_NtoB(mPacketGroupOffsets[index]); }

	// Properties (in storage endian)
	private:
		UInt8	mVersion;
		UInt8	mFlags[3];

		// For each packet group specified in the stsc Atom, the file offset is specified
		UInt32	mPacketGroupOffsetCount;
		UInt64	mPacketGroupOffsets[];		// Offset to start of packet data from start of file
};

#if defined(TARGET_OS_WINDOWS)
	#pragma warning(default:4200)
#endif

#pragma pack(pop)

static	CString	sErrorDomain(OSSTR("CMPEG4MediaFile"));
static	SError	sInvalidMagicCookieError(sErrorDomain, 1, CString(OSSTR("Invalid Magic Cookie")));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMPEG4MediaFile::SstsdAtomPayload

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CMPEG4MediaFile::SstsdAtomPayload::SstsdAtomPayload(UInt32 descriptionCount) :
		mVersion(0), mDescriptionCount(EndianU32_NtoB(descriptionCount))
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mFlags[0] = 0;
	mFlags[1] = 0;
	mFlags[2] = 0;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CData CMPEG4MediaFile::SstsdAtomPayload::getData(const TArray<CData>& descriptions)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SstsdAtomPayload	stsdAtomPayload(descriptions.getCount());
	CData				data(&stsdAtomPayload, sizeof(SstsdAtomPayload), false);

	// Add descriptions
	for (TIteratorD<CData> iterator = descriptions.getIterator(); iterator.hasValue(); iterator.advance())
		// Add description
		data += *iterator;

	return data;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMPEG4MediaFile::SstsdMP4ADescription

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CMPEG4MediaFile::SstsdMP4ADescription::SstsdMP4ADescription(UInt32 length, Float32 sampleRate,
		const SAudio::ChannelMap& audioChannelMap, UInt16 dataRefIndex) :
		mLength(EndianU32_NtoB(length)), mType(EndianU32_NtoB(kType)), mDataRefIndex(EndianU16_NtoB(dataRefIndex)),
		mQuickTimeEncodingVersion(EndianU16_NtoB(0)), mQuickTimeEncodingRevisionLevel(EndianU16_NtoB(0)),
		mQuickTimeEncodingVendor(EndianU32_NtoB(0)), mChannelCounts(EndianU16_NtoB(audioChannelMap.getChannelCount())),
		mBits(EndianU16_NtoB(16)), mQuickTimeCompressionID(EndianU16_NtoB(0)), mQuickTimePacketSize(EndianU16_NtoB(0)),
		mSampleRate(EndianU32_NtoB((UInt32) sampleRate << 16))
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mReserved[0] = 0;
	mReserved[1] = 0;
	mReserved[2] = 0;
	mReserved[3] = 0;
	mReserved[4] = 0;
	mReserved[5] = 0;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> CMPEG4MediaFile::SstsdMP4ADescription::getData(Float32 sampleRate,
		const SAudio::ChannelMap& audioChannelMap, UInt16 dataRefIndex, const CData& magicCookie)
//----------------------------------------------------------------------------------------------------------------------
{
	// Decode Magic Cookie
	const	UInt8*					constBytePtr = (const UInt8*) magicCookie.getBytePtr();
			CData::ByteCount		byteCount = magicCookie.getByteCount();
	const	SMP4AudioFormatAtom&	audioFormatAtom = *((const SMP4AudioFormatAtom*) constBytePtr);
			CData					magicCookieData;
	if ((byteCount >= sizeof(SMP4AudioFormatAtom)) &&
			(audioFormatAtom.getLength() == 12) &&
			(audioFormatAtom.getType() == MAKE_OSTYPE('f', 'r', 'm', 'a')) &&
			(audioFormatAtom.getFormat() == MAKE_OSTYPE('m', 'p', '4', 'a'))) {
		// We have an old-style magic cookie in full atom form
		// Skip the atom header
		constBytePtr += audioFormatAtom.getLength();
		byteCount -= audioFormatAtom.getLength();

		// Extract just the part we need
		const	SMP4AtomHeader&	mp4aAtomHeader = *((const SMP4AtomHeader*) constBytePtr);
		if ((byteCount < sizeof(SMP4AtomHeader)) || (mp4aAtomHeader.getType() != MAKE_OSTYPE('m', 'p', '4', 'a')))
			return TVResult<CData>(sInvalidMagicCookieError);

		// Almost there...
		constBytePtr += mp4aAtomHeader.getLength();

		// Sanity check
		const	SMP4AtomHeader&	esdsAtomHeader = *((const SMP4AtomHeader*) constBytePtr);
		if (esdsAtomHeader.getType() != MAKE_OSTYPE('e', 's', 'd', 's'))
			return TVResult<CData>(sInvalidMagicCookieError);

		// Copy magic cookie
		magicCookieData = CData(constBytePtr, esdsAtomHeader.getLength());
	} else
		// New style magic cookie
		magicCookieData +=
				SMP4AtomHeader(sizeof(SMP4AtomHeader) + sizeof(SMP4MP4AESDSAtomPayloadHeader) + (UInt32) byteCount,
								MAKE_OSTYPE('e', 's', 'd', 's'))
						.getData() +
				SMP4MP4AESDSAtomPayloadHeader().getData() +
				magicCookie;

	return SstsdMP4ADescription(sizeof(SstsdMP4ADescription) + (UInt32) magicCookieData.getByteCount(), sampleRate,
					audioChannelMap, dataRefIndex).getData() +
			magicCookieData;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMPEG4MediaFile

// MARK: Properties

OSType	CMPEG4MediaFile::mID = MAKE_OSTYPE('M', 'P', '4', ' ');

// MARK: Internals

struct CMPEG4MediaFile::Internals {

								Internals(const CAtomReader& atomReader, const CAtomReader::Atom& stsdAtom,
										const CAtomReader::ContainerAtom& stblContainerAtom,
										const SstsdDescriptionHeader& stsdDescriptionHeader,
										const SsttsAtomPayload& sttsAtomPayload,
										const SstscAtomPayload& stscAtomPayload,
										const SstszAtomPayload& stszAtomPayload,
										SstcoAtomPayload* stcoAtomPayload, SMP4co64AtomPayload* co64AtomPayload) :
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
																					sizeof(SstsdAtomPayload) + offset);
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

	const	CAtomReader&				mAtomReader;
	const	CAtomReader::Atom&			mSTSDAtom;
	const	CAtomReader::ContainerAtom&	mSTBLContainerAtom;
	const	SstsdDescriptionHeader&		mSTSDDescriptionHeader;
	const	SsttsAtomPayload&			mSTTSAtomPayload;
	const	SstscAtomPayload&			mSTSCAtomPayload;
	const	SstszAtomPayload&			mSTSZAtomPayload;
			SstcoAtomPayload*	 		mSTCOAtomPayload;
			SMP4co64AtomPayload*		mCO64AtomPayload;
};

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
I<SMediaSource::ImportResult> CMPEG4MediaFile::import(const SMediaSource::ImportSetup& importSetup)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CAtomReader	atomReader(importSetup.getRandomAccessDataSource());
	OV<SError>	error;

	// Read root atom
	TVResult<CAtomReader::Atom>	atom = atomReader.readAtom();
	ReturnValueIfResultError(atom, I<SMediaSource::ImportResult>(new SMediaSource::ImportResult()));
	if (atom->mType != MAKE_OSTYPE('f', 't', 'y', 'p'))
		return I<SMediaSource::ImportResult>(new SMediaSource::ImportResult());

	// Find moov atom
	while (atom->mType != MAKE_OSTYPE('m', 'o', 'o', 'v')) {
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
	for (TIteratorD<CAtomReader::Atom> moovIterator = moovContainerAtom->getIterator();
			moovIterator.hasValue(); moovIterator.advance()) {
		// Check type
		if (moovIterator->mType == MAKE_OSTYPE('t', 'r', 'a', 'k')) {
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
			const	SmdhdAtomPayload&		mdhdAtomPayload = *((SmdhdAtomPayload*) mdhdAtomPayloadData->getBytePtr());
					UInt32					timeScale = mdhdAtomPayload.getTimeScale();
					UniversalTimeInterval	duration =
													(UniversalTimeInterval) mdhdAtomPayload.getDuration() /
															(UniversalTimeInterval) timeScale;

			// Handler
			TVResult<CData>	hdlrAtomPayloadData =
									atomReader.readAtomPayload(
											mdiaContainerAtom->getAtom(MAKE_OSTYPE('h', 'd', 'l', 'r')));
			if (hdlrAtomPayloadData.hasError()) continue;
			const	ShdlrAtomPayload&	hdlrAtomPayload = *((ShdlrAtomPayload*) hdlrAtomPayloadData->getBytePtr());

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
			const	SstsdAtomPayload&		stsdAtomPayload =
													*((SstsdAtomPayload*) stsdAtomPayloadData->getBytePtr());
			const	SstsdDescriptionHeader&	stsdDescriptionHeader = stsdAtomPayload.getFirstDescriptionHeader();

			// Sample Table Time-to-Sample
			TVResult<CData>	sttsAtomPayloadData =
									atomReader.readAtomPayload(
											stblContainerAtom->getAtom(MAKE_OSTYPE('s', 't', 't', 's')));
			if (sttsAtomPayloadData.hasError()) continue;
			const	SsttsAtomPayload&	sttsAtomPayload = *((SsttsAtomPayload*) sttsAtomPayloadData->getBytePtr());

			// Sample Table Sample Blocks
			TVResult<CData>	stscAtomPayloadData =
									atomReader.readAtomPayload(
											stblContainerAtom->getAtom(MAKE_OSTYPE('s', 't', 's', 'c')));
			if (stscAtomPayloadData.hasError()) continue;
			const	SstscAtomPayload&	stscAtomPayload = *((SstscAtomPayload*) stscAtomPayloadData->getBytePtr());

			// Sample Table Packet Sizes
			TVResult<CData>	stszAtomPayloadData =
									atomReader.readAtomPayload(
											stblContainerAtom->getAtom(MAKE_OSTYPE('s', 't', 's', 'z')));
			if (stszAtomPayloadData.hasError()) continue;
			const	SstszAtomPayload&	stszAtomPayload = *((SstszAtomPayload*) stszAtomPayloadData->getBytePtr());

			// Sample Table Block offsets
			TVResult<CData>	stcoAtomPayloadData =
									atomReader.readAtomPayload(
											stblContainerAtom->getAtom(MAKE_OSTYPE('s', 't', 'c', 'o')));
			TVResult<CData>	co64AtomPayloadData =
									atomReader.readAtomPayload(
											stblContainerAtom->getAtom(MAKE_OSTYPE('c', 'o', '6', '4')));
			if (!stcoAtomPayloadData.hasValue() && !co64AtomPayloadData.hasValue()) continue;
			SstcoAtomPayload*		stcoAtomPayload =
											stcoAtomPayloadData.hasValue() ?
													(SstcoAtomPayload*) stcoAtomPayloadData->getBytePtr() : nil;
			SMP4co64AtomPayload*	co64AtomPayload =
											co64AtomPayloadData.hasValue() ?
													(SMP4co64AtomPayload*) co64AtomPayloadData->getBytePtr() : nil;

			// Internals
			Internals	internals(atomReader, *stsdAtom, *stblContainerAtom, stsdDescriptionHeader, sttsAtomPayload,
								stscAtomPayload, stszAtomPayload, stcoAtomPayload, co64AtomPayload);

			// Check track type
			if (hdlrAtomPayload.getSubType() == ShdlrAtomPayload::kSubTypeSound) {
				// Audio track
				TVResult<SMediaSource::Tracks::AudioTrack>	audioTrack =
																	composeAudioTrack(
																			importSetup.getRandomAccessDataSource(),
																			importSetup.getOptions(),
																			stsdDescriptionHeader.getType(), duration,
																			internals);
				ReturnValueIfResultError(audioTrack,
						I<SMediaSource::ImportResult>(new SMediaSource::ImportResult(audioTrack.getError())));

				// Success
				mediaSourceTracks.add(*audioTrack);
			} else if (hdlrAtomPayload.getSubType() == ShdlrAtomPayload::kSubTypeVideo) {
				// Video track
				TVResult<SMediaSource::Tracks::VideoTrack>	videoTrack =
																	composeVideoTrack(
																			importSetup.getRandomAccessDataSource(),
																			importSetup.getOptions(),
																			stsdDescriptionHeader.getType(), timeScale,
																			duration, internals);
				ReturnValueIfResultError(videoTrack,
						I<SMediaSource::ImportResult>(new SMediaSource::ImportResult(videoTrack.getError())));

				// Success
				mediaSourceTracks.add(*videoTrack);
			} else {
				// Import track
				error =
						importTrack(importSetup.getRandomAccessDataSource(), hdlrAtomPayload.getSubType(),
								stsdDescriptionHeader, internals);
				if (error.hasValue())
					// Error
					return I<SMediaSource::ImportResult>(new SMediaSource::ImportResult(*error));
			}
		} else if (moovIterator->mType == MAKE_OSTYPE('u', 'd', 't', 'a')) {
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
	const	SsttsAtomPayload&	sttsAtomPayload = internals.mSTTSAtomPayload;
	const	SstscAtomPayload&	stscAtomPayload = internals.mSTSCAtomPayload;
	const	SstszAtomPayload&	stszAtomPayload = internals.mSTSZAtomPayload;

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
		const	SsttsAtomPayload::Chunk&	sttsChunk = sttsAtomPayload.getChunk(sttsChunkIndex);
				UInt32						sttsChunkPacketCount = sttsChunk.getPacketCount();
				UInt32						sttsChunkPacketDuration = sttsChunk.getPacketDuration();

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
		const I<CRandomAccessDataSource>& randomAccessDataSource, UInt32 options, OSType type,
		UniversalTimeInterval duration, const Internals& internals)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check type
	switch (type) {
		case SstsdMP4ADescription::kType: {
			// MPEG4 (AAC) Audio
			const	SstsdMP4ADescription&	mp4ADescription =
													*((SstsdMP4ADescription*) &internals.mSTSDDescriptionHeader);

			// Get configuration data
			TVResult<CData>	configurationData = internals.getDecompressionData(sizeof(SstsdMP4ADescription));
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
			if (options & SMediaSource::kOptionsCreateDecoders)
				// Add audio track with decode info
				return TVResult<SMediaSource::Tracks::AudioTrack>(
						SMediaSource::Tracks::AudioTrack(audioFormat, mediaSegmentInfo,
								CAACAudioCodec::create(*info, randomAccessDataSource, mediaPacketAndLocations)));
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
		const I<CRandomAccessDataSource>& randomAccessDataSource, UInt32 options, OSType type, UInt32 timeScale,
		UniversalTimeInterval duration, const Internals& internals)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check type
	switch (type) {
		case SMP4stsdH264Description::kType: {
			// h.264 Video
			const	SMP4stsdH264Description&	h264Description =
														*((SMP4stsdH264Description*) &internals.mSTSDDescriptionHeader);


			// Get configuration data
			TVResult<CData>	stssAtomPayloadData =
									internals.mAtomReader.readAtomPayload(
											internals.mSTBLContainerAtom.getAtom(MAKE_OSTYPE('s', 't', 's', 's')));
			ReturnValueIfResultError(stssAtomPayloadData,
					TVResult<SMediaSource::Tracks::VideoTrack>(stssAtomPayloadData.getError()));

			// Get configuration data
			TVResult<CData>	configurationData = internals.getDecompressionData(sizeof(SMP4stsdH264Description));
			ReturnValueIfResultError(configurationData,
					TVResult<SMediaSource::Tracks::VideoTrack>(configurationData.getError()))

			// Compose packet and locations
			TArray<SMedia::PacketAndLocation>	mediaPacketAndLocations = composePacketAndLocations(internals);
			Float32								framerate =
														(Float32) mediaPacketAndLocations.getCount() /
																(Float32) duration;
			UInt64								byteCount =
														SMedia::PacketAndLocation::getTotalByteCount(
																mediaPacketAndLocations);

			// Compose info
			SVideo::Format		videoFormat =
										CH264VideoCodec::composeVideoTrackFormat(
												S2DSizeU16(h264Description.getWidth(), h264Description.getHeight()),
												framerate);
			SMedia::SegmentInfo	mediaSegmentInfo(duration, byteCount);

			// Add video track
			if (options & SMediaSource::kOptionsCreateDecoders) {
				// Setup
				const	SMP4stssAtomPayload&	stssAtomPayload =
														*((SMP4stssAtomPayload*) stssAtomPayloadData->getBytePtr());
						UInt32					keyframesCount = stssAtomPayload.getKeyframesCount();
						TNumberArray<UInt32>	keyframeIndexes;
				for (UInt32 i = 0; i < keyframesCount; i++)
					// Add keyframe index
					keyframeIndexes += stssAtomPayload.getKeyframeIndex(i);

				// Add video track with decode info
				return TVResult<SMediaSource::Tracks::VideoTrack>(
						SMediaSource::Tracks::VideoTrack(videoFormat, mediaSegmentInfo,
								CH264VideoCodec::create(randomAccessDataSource, mediaPacketAndLocations,
										*configurationData, timeScale, keyframeIndexes)));
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
