//----------------------------------------------------------------------------------------------------------------------
//	CMPEG4MediaFile.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMPEG4MediaFile.h"

#include "CAACAudioCodec.h"
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

//struct SMP4ftypAtom {
//	OSType	mMajorBrand;
//	UInt32	mMajorBrandVersion;
//	OSType	mCompatibleBrands[];
//};

struct SMP4hdlrAtomPayload {
			// Methods
	OSType	getSubType() const
				{ return EndianU32_BtoN(mSubType); }

	// Properties (in storage endian)
	private:
		UInt8	mVersion;						// 0
		UInt8	mFlags[3];
		OSType	mQuickTimeType;
		OSType	mSubType;
		OSType	mQuickTimeManufacturerType;
		UInt32	mQuickTimeComponentFlags;
		UInt32	mQuickTimeComponentFlagsMask;
		SInt8	mComponentTypeName[];
};

//struct SMP4mvhdAtomV0 {
//	UInt8	mVersion;		// 0
//	UInt8	mFlags[3];
//
//	UInt32	mCreatedDate;
//	UInt32	mModifiedDate;
//
//	UInt32	mTimeScale;
//
//	UInt32	mDuration;
//
//	UInt32	mPlaybackSpeed;
//	UInt16	mVolume;
//	UInt16	mReserved[5];	// 0
//	UInt32	mMatrixA;		// Width Scale
//	UInt32	mMatrixB;		// Width Rotate
//	UInt32	mMatrixU;		// Width Angle
//	UInt32	mMatrixC;		// Height Rotate
//	UInt32	mMatrixD;		// Height Scale
//	UInt32	mMatrixV;		// Height Angle
//	UInt32	mMatrixX;
//	UInt32	mMatrixY;
//	UInt32	mMatrixW;
//
//	UInt32	mQuickTimePreviewStart;
//	UInt32	mQuickTimePreviewDuration;
//	UInt32	mQuickTimePoster;
//	UInt32	mQuickTimeSelectionStart;
//	UInt32	mQuickTimeSelectionDuration;
//	UInt32	mQuickTimeCurrentTime;
//
//	UInt32	mNextTrackID;
//};
//
//struct SMP4mvhdAtomV1 {
//	UInt8	mVersion;	// 1
//	UInt8	mFlags[3];
//
//	UInt64	mCreatedDate;
//	UInt64	mModifiedDate;
//
//	UInt32	mTimeScale;
//
//	UInt64	mDuration;
//
//	UInt32	mPlaybackSpeed;
//	UInt16	mVolume;
//	UInt16	mReserved[5];	// 0
//	UInt32	mMatrixA;		// Width Scale
//	UInt32	mMatrixB;		// Width Rotate
//	UInt32	mMatrixU;		// Width Angle
//	UInt32	mMatrixC;		// Height Rotate
//	UInt32	mMatrixD;		// Height Scale
//	UInt32	mMatrixV;		// Height Angle
//	UInt32	mMatrixX;
//	UInt32	mMatrixY;
//	UInt32	mMatrixW;
//
//	UInt32	mQuickTimePreviewStart;
//	UInt32	mQuickTimePreviewDuration;
//	UInt32	mQuickTimePoster;
//	UInt32	mQuickTimeSelectionStart;
//	UInt32	mQuickTimeSelectionDuration;
//	UInt32	mQuickTimeCurrentTime;
//
//	UInt32	mNextTrackID;
//};
//

//struct SMP4tkhdAtomPayload {
//	// Structs
//	struct Payload {
//		UInt8	mVersion;
//		UInt8	mFlags[3];	// 0x000001:	Track Enabled
//							// 0x000002:	Track In Movie
//							// 0x000004:	Track In Preview
//							// 0x000008:	Track In Poster
//		union {
//			struct InfoV0 {
//				UInt32	mCreatedDate;
//				UInt32	mModifiedDate;
//
//				UInt32	mTrackID;
//				UInt32	mReserved;
//				UInt32	mDuration;
//				UInt32	mReserved1[2];
//				UInt16	mVideoLayer;
//				UInt16	mQuickTimeAlternateID;
//				UInt16	mAudioVolume;
//				UInt16	mReserved4;
//				UInt32	mMatrixA;		// Width Scale
//				UInt32	mMatrixB;		// Width Rotate
//				UInt32	mMatrixU;		// Width Angle
//				UInt32	mMatrixC;		// Height Rotate
//				UInt32	mMatrixD;		// Height Scale
//				UInt32	mMatrixV;		// Height Angle
//				UInt32	mMatrixX;
//				UInt32	mMatrixY;
//				UInt32	mMatrixW;
//				UInt32	mFrameWidth;
//				UInt32	mFrameHeight;
//			} mInfoV0;
//
//			struct InfoV1 {
//				UInt64	mCreatedDate;
//				UInt64	mModifiedDate;
//
//				UInt32	mTrackID;
//				UInt32	mReserved;
//				UInt64	mDuration;
//				UInt32	mReserved1[2];
//				UInt16	mVideoLayer;
//				UInt16	mQuickTimeAlternateID;
//				UInt16	mAudioVolume;
//				UInt16	mReserved4;
//				UInt32	mMatrixA;		// Width Scale
//				UInt32	mMatrixB;		// Width Rotate
//				UInt32	mMatrixU;		// Width Angle
//				UInt32	mMatrixC;		// Height Rotate
//				UInt32	mMatrixD;		// Height Scale
//				UInt32	mMatrixV;		// Height Angle
//				UInt32	mMatrixX;
//				UInt32	mMatrixY;
//				UInt32	mMatrixW;
//				UInt32	mFrameWidth;
//				UInt32	mFrameHeight;
//			} mInfoV1;
//		} _;
//	};
//
//			// Lifecycle methods
//			StkhdAtomPayload(const CData& data) : mData(data) {}
//
//			// Instance Methods
//	UInt64	getDuration() const
//				{
//					// Setup
//					const	Payload&	payload = *((Payload*) mData.getBytePtr());
//
//					return (payload.mVersion == 0) ?
//							EndianU32_BtoN(payload._.mInfoV0.mDuration) :
//							EndianU64_BtoN(payload._.mInfoV1.mDuration);
//				}
//
//	// Properties (in storage endian)
//	private:
//		const	CData&	mData;
//};

struct SMP4mdhdAtomPayload {
	// Structs
	struct Payload {
		UInt8	mVersion;
		UInt8	mFlags[3];
		union {
			struct InfoV0 {
				UInt32	mCreationDate;
				UInt32	mModificationDate;

				UInt32	mTimeScale;
				UInt32	mDuration;

				UInt16	mLanguageCode;
				UInt16	mQuickTimeQuality;
			} mInfoV0;

			struct InfoV1 {
				UInt64	mCreationDate;
				UInt64	mModificationDate;

				UInt32	mTimeScale;
				UInt64	mDuration;

				UInt16	mLanguageCode;
				UInt16	mQuickTimeQuality;
			} mInfoV1;
		} _;
	};

			// Lifecycle methods
			SMP4mdhdAtomPayload(const CData& data) : mData(data) {}

			// Instance Methods
	UInt32	getTimeScale() const
				{
					// Setup
					const	Payload&	payload = *((Payload*) mData.getBytePtr());

					return (payload.mVersion == 0) ?
							EndianU32_BtoN(payload._.mInfoV0.mTimeScale) :
							EndianU32_BtoN(payload._.mInfoV1.mTimeScale);
				}
	UInt64	getDuration() const
				{
					// Setup
					const	Payload&	payload = *((Payload*) mData.getBytePtr());

					return (payload.mVersion == 0) ?
							(UInt64) EndianU32_BtoN(payload._.mInfoV0.mDuration) :
							EndianU64_BtoN(payload._.mInfoV1.mDuration);
				}

	// Properties (in storage endian)
	private:
		const	CData&	mData;
};

//struct SMP4smhdAtom {
//	UInt8	mVersion;								// 0
//	UInt8	mFlags[3];
//
//	UInt16	mAudioBalance;
//	UInt8	mReserved[2];
//};
//
//struct SMP4drefAtom {
//	UInt8	mVersion;								// 0
//	UInt8	mFlags[3];
//
//	UInt32	mReferenceCount;
//};
//
//struct SMP4urlAtom {
//	UInt8	mVersion;								// 0
//	UInt8	mFlags[3];
//};

// MP4A Description
struct SMP4stsdMP4ADescription {
			// Methods
	UInt16	getChannels() const
				{ return EndianU16_BtoN(mChannels); }

	// Properties (in storage endian)
	private:
		UInt32	mLength;
		OSType	mType;
		UInt8	mReserved[6];
		UInt16	mDataRefIndex;

		UInt16	mQuickTimeEncodingVersion;
		UInt16	mQuickTimeEncodingRevisionLevel;
		OSType	mQuickTimeEncodingVendor;
		UInt16	mChannels;
		UInt16	mBits;
		UInt16	mQuickTimeCompressionID;
		UInt16	mQuickTimePacketSize;
		UInt32	mSampleRate;
};

// h.264 Description
struct SMP4stsdH264Description {
			// Methods
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

// Sample Table Sample Description (general)
struct SMP4stsdDescription {
	// Methods
	OSType	getType() const
				{ return EndianU32_BtoN(mType); }

	// Properties (in storage endian)
	private:
		UInt32	mLength;
		OSType	mType;
		UInt8	mReserved[6];
		UInt16	mDataRefIndex;
};

// Sample Table Sample Description Atom Payload
struct SMP4stsdAtomPayload {
									// Methods
	const	SMP4stsdDescription&	getFirstDescription() const
										{ return *((SMP4stsdDescription*) mDescriptions); }

	// Properties (in storage endian)
	private:
		UInt8	mVersion;				// 0
		UInt8	mFlags[3];
		UInt32	mDescriptionCount;
		UInt8	mDescriptions[];
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

// Sample Table Time-to-Sample Atom Payload
struct SMP4sttsAtomPayload {
	// Structs
	struct Chunk {
				// Methods
		UInt32	getPacketCount() const
					{ return EndianU32_BtoN(mPacketCount); }
		UInt32	getPacketDuration() const
					{ return EndianU32_BtoN(mPacketDuration); }

		// Properties (in storage endian)
		private:
			UInt32	mPacketCount;
			UInt32	mPacketDuration;
	};

					// Methods
			UInt32	getChunkCount() const
						{ return EndianU32_BtoN(mChunkCount); }
	const	Chunk&	getChunk(UInt32 index) const
						{ return mChunks[index]; }

	// Properties (in storage endian)
	private:
		UInt8	mVersion;
		UInt8	mFlags[3];
		UInt32	mChunkCount;
		Chunk	mChunks[];
};

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

// Sample Table Sample-to-Chunk Atom Payload
struct SMP4stscAtomPayload {
	// Structs
	struct PacketGroupInfo {
		// Methods
		UInt32	getChunkStartIndex() const
					{ return EndianU32_BtoN(mChunkStartIndex); }
		UInt32	getPacketCount() const
					{ return EndianU32_BtoN(mPacketCount); }

		// Properties (in storage endian)
		private:
			UInt32	mChunkStartIndex;
			UInt32	mPacketCount;
			UInt32	mSampleDescriptionIndex;
	};

								// Methods
			UInt32				getPacketGroupInfoCount() const
									{ return EndianU32_BtoN(mPacketGroupInfoCount); }
	const	PacketGroupInfo&	getPacketGroupInfo(UInt32 index) const
									{ return mPacketGroupInfos[index]; }

	// Properties (in storage endian)
	private:
		UInt8			mVersion;
		UInt8			mFlags[3];
		UInt32			mPacketGroupInfoCount;
		PacketGroupInfo	mPacketGroupInfos[];
};

// Sample Table Sample siZe Atom Payload
struct SMP4stszAtomPayload {
			// Methods
	UInt32	getPacketByteCount(UInt32 index) const
					{ return (mGlobalPacketByteCount != 0) ?
							EndianU32_BtoN(mGlobalPacketByteCount) : EndianU32_BtoN(mPacketByteCounts[index]); }

	// Properties (in storage endian)
	private:
		UInt8	mVersion;
		UInt8	mFlags[3];

		// If every packet has the same size, specify it here
		UInt32	mGlobalPacketByteCount;	// 0 means use packet sizes below

		// For every individual packet, specify the packet size
		UInt32	mPacketByteCountCount;
		UInt32	mPacketByteCounts[];			// Packet size in bytes
};

// Sample Table Chunk Offset Atom Payload
struct SMP4stcoAtomPayload {
			// Methods
	UInt32	getPacketGroupOffsetCount() const
				{ return EndianU32_BtoN(mPacketGroupOffsetCount); }
	UInt64	getPacketGroupOffset(UInt32 index) const
				{ return EndianU32_BtoN(mPacketGroupOffsets[index]); }

	// Properties (in storage endian)
	private:
		UInt8	mVersion;
		UInt8	mFlags[3];

		// For each packet group specified in the stsc Atom, the file offset is specified
		UInt32	mPacketGroupOffsetCount;
		UInt32	mPacketGroupOffsets[];		// Offset to start of packet data from start of file
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

//struct SMP4metaAtom {
//	UInt8	mVersion;
//	UInt8	mFlags[3];
//};
//
//struct SMP4dataAtom {
//	UInt8	mVersion;
//	UInt8	mFlags[3];
//
//	UInt32	mReserved;
//
//	UInt8	mData[];
//};

#if defined(TARGET_OS_WINDOWS)
	#pragma warning(default:4200)
#endif

#pragma pack(pop)

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMPEG4MediaFile

// MARK: Internals

struct CMPEG4MediaFile::Internals {

								Internals(const CAtomReader& atomReader, const CAtomReader::Atom& stsdAtom,
										const CAtomReader::ContainerAtom& stblContainerAtom,
										const SMP4stsdDescription& stsdDescription,
										const SMP4sttsAtomPayload& sttsAtomPayload,
										const SMP4stscAtomPayload& stscAtomPayload,
										const SMP4stszAtomPayload& stszAtomPayload,
										SMP4stcoAtomPayload* stcoAtomPayload, SMP4co64AtomPayload* co64AtomPayload) :
									mAtomReader(atomReader), mSTSDAtom(stsdAtom), mSTBLContainerAtom(stblContainerAtom),
											mSTSDDescription(stsdDescription), mSTTSAtomPayload(sttsAtomPayload),
											mSTSCAtomPayload(stscAtomPayload), mSTSZAtomPayload(stszAtomPayload),
											mSTCOAtomPayload(stcoAtomPayload), mCO64AtomPayload(co64AtomPayload)
									{}

			TIResult<CData>		getDecompressionData(SInt64 offset) const
									{
										// Read atom
										TIResult<CAtomReader::Atom>	atom =
																			mAtomReader.readAtom(mSTSDAtom,
																					sizeof(SMP4stsdAtomPayload) +
																							offset);
										ReturnValueIfResultError(atom, TIResult<CData>(atom.getError()));

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
	const	SMP4stsdDescription&		mSTSDDescription;
	const	SMP4sttsAtomPayload&		mSTTSAtomPayload;
	const	SMP4stscAtomPayload&		mSTSCAtomPayload;
	const	SMP4stszAtomPayload&		mSTSZAtomPayload;
			SMP4stcoAtomPayload* 		mSTCOAtomPayload;
			SMP4co64AtomPayload*		mCO64AtomPayload;
};

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
SMediaSource::ImportResult CMPEG4MediaFile::import(const I<CRandomAccessDataSource>& randomAccessDataSource,
		const OI<CAppleResourceManager>& appleResourceManager, SMediaSource::Options options)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CAtomReader	atomReader(randomAccessDataSource);
	OI<SError>	error;

	// Read root atom
	TIResult<CAtomReader::Atom>	atom = atomReader.readAtom();
	ReturnValueIfResultError(atom, SMediaSource::ImportResult());
	if (atom->mType != MAKE_OSTYPE('f', 't', 'y', 'p'))
		return SMediaSource::ImportResult();

	// Find moov atom
	while (atom->mType != MAKE_OSTYPE('m', 'o', 'o', 'v')) {
		// Go to next atom
		error = atomReader.seekToNextAtom(*atom);
		ReturnValueIfError(error, SMediaSource::ImportResult(*error));

		// Get atom
		atom = atomReader.readAtom();
		ReturnValueIfResultError(atom, SMediaSource::ImportResult(atom.getError()));
	}

	TIResult<CAtomReader::ContainerAtom>	moovContainerAtom = atomReader.readContainerAtom(*atom);
	ReturnValueIfResultError(moovContainerAtom, SMediaSource::ImportResult(moovContainerAtom.getError()));

	// Iterate moov atom
	CMediaTrackInfos	mediaTrackInfos;
	for (TIteratorD<CAtomReader::Atom> moovIterator = moovContainerAtom->getIterator();
			moovIterator.hasValue(); moovIterator.advance()) {
		// Check type
		if (moovIterator->mType == MAKE_OSTYPE('t', 'r', 'a', 'k')) {
			// Track
			TIResult<CAtomReader::ContainerAtom>	trakContainerAtom = atomReader.readContainerAtom(*moovIterator);
			if (trakContainerAtom.hasError()) continue;

			// Media
			TIResult<CAtomReader::ContainerAtom>	mdiaContainerAtom =
															atomReader.readContainerAtom(
																	trakContainerAtom->getAtom(
																			MAKE_OSTYPE('m', 'd', 'i', 'a')));
			if (mdiaContainerAtom.hasError()) continue;

			// Media header
			TIResult<CData>	mdhdAtomPayloadData =
									atomReader.readAtomPayload(
											mdiaContainerAtom->getAtom(MAKE_OSTYPE('m', 'd', 'h', 'd')));
			if (mdhdAtomPayloadData.hasError()) continue;

			SMP4mdhdAtomPayload		mdhdAtomPayload(*mdhdAtomPayloadData);
			UInt32					timeScale = mdhdAtomPayload.getTimeScale();
			UniversalTimeInterval	duration =
											(UniversalTimeInterval) mdhdAtomPayload.getDuration() /
													(UniversalTimeInterval) timeScale;

			// Handler
			TIResult<CData>	hdlrAtomPayloadData =
									atomReader.readAtomPayload(
											mdiaContainerAtom->getAtom(MAKE_OSTYPE('h', 'd', 'l', 'r')));
			if (hdlrAtomPayloadData.hasError()) continue;
			const	SMP4hdlrAtomPayload&	hdlrAtomPayload =
													*((SMP4hdlrAtomPayload*) hdlrAtomPayloadData->getBytePtr());

			// Media Information
			TIResult<CAtomReader::ContainerAtom>	minfContainerAtom =
															atomReader.readContainerAtom(
																	mdiaContainerAtom->getAtom(
																			MAKE_OSTYPE('m', 'i', 'n', 'f')));
			if (minfContainerAtom.hasError()) continue;

			// Sample Table
			TIResult<CAtomReader::ContainerAtom>	stblContainerAtom =
															atomReader.readContainerAtom(
																	minfContainerAtom->getAtom(
																			MAKE_OSTYPE('s', 't', 'b', 'l')));
			if (stblContainerAtom.hasError()) continue;

			// Sample Table Sample Description
			OR<CAtomReader::Atom>	stsdAtom = stblContainerAtom->getAtom(MAKE_OSTYPE('s', 't', 's', 'd'));
			if (!stsdAtom.hasReference()) continue;
			TIResult<CData>	stsdAtomPayloadData = atomReader.readAtomPayload(*stsdAtom);
			if (error.hasInstance()) continue;
			const	SMP4stsdAtomPayload&	stsdAtomPayload =
													*((SMP4stsdAtomPayload*) stsdAtomPayloadData->getBytePtr());
			const	SMP4stsdDescription&	stsdDescription = stsdAtomPayload.getFirstDescription();

			// Sample Table Time-to-Sample
			TIResult<CData>	sttsAtomPayloadData =
									atomReader.readAtomPayload(
											stblContainerAtom->getAtom(MAKE_OSTYPE('s', 't', 't', 's')));
			if (sttsAtomPayloadData.hasError()) continue;
			const	SMP4sttsAtomPayload&	sttsAtomPayload =
													*((SMP4sttsAtomPayload*)
															sttsAtomPayloadData->getBytePtr());

			// Sample Table Sample Blocks
			TIResult<CData>	stscAtomPayloadData =
									atomReader.readAtomPayload(
											stblContainerAtom->getAtom(MAKE_OSTYPE('s', 't', 's', 'c')));
			if (stscAtomPayloadData.hasError()) continue;
			const	SMP4stscAtomPayload&	stscAtomPayload =
													*((SMP4stscAtomPayload*)
															stscAtomPayloadData->getBytePtr());

			// Sample Table Packet Sizes
			TIResult<CData>	stszAtomPayloadData =
									atomReader.readAtomPayload(
											stblContainerAtom->getAtom(MAKE_OSTYPE('s', 't', 's', 'z')));
			if (stszAtomPayloadData.hasError()) continue;
			const	SMP4stszAtomPayload&	stszAtomPayload =
													*((SMP4stszAtomPayload*) stszAtomPayloadData->getBytePtr());

			// Sample Table Block offsets
			TIResult<CData>	stcoAtomPayloadData =
									atomReader.readAtomPayload(
											stblContainerAtom->getAtom(MAKE_OSTYPE('s', 't', 'c', 'o')));
			TIResult<CData>	co64AtomPayloadData =
									atomReader.readAtomPayload(
											stblContainerAtom->getAtom(MAKE_OSTYPE('c', 'o', '6', '4')));
			if (!stcoAtomPayloadData.hasInstance() && !co64AtomPayloadData.hasInstance()) continue;
			SMP4stcoAtomPayload*	stcoAtomPayload =
											stcoAtomPayloadData.hasInstance() ?
													(SMP4stcoAtomPayload*) stcoAtomPayloadData->getBytePtr() :
													nil;
			SMP4co64AtomPayload*	co64AtomPayload =
											co64AtomPayloadData.hasInstance() ?
													(SMP4co64AtomPayload*) co64AtomPayloadData->getBytePtr() :
													nil;

			// Internals
			Internals	internals(atomReader, *stsdAtom, *stblContainerAtom, stsdDescription, sttsAtomPayload,
								stscAtomPayload, stszAtomPayload, stcoAtomPayload, co64AtomPayload);

			// Check track type
			if (hdlrAtomPayload.getSubType() == MAKE_OSTYPE('s', 'o', 'u', 'n')) {
				// Audio track
				TVResult<CMediaTrackInfos::AudioTrackInfo>	audioTrackInfo =
																	composeAudioTrackInfo(randomAccessDataSource,
																			options, stsdDescription.getType(),
																			duration, internals);
				if (audioTrackInfo.hasValue())
					// Success
					mediaTrackInfos.add(*audioTrackInfo);
				else
					// Error
					return SMediaSource::ImportResult(audioTrackInfo.getError());
			} else if (hdlrAtomPayload.getSubType() == MAKE_OSTYPE('v', 'i', 'd', 'e')) {
				// Video track
				TVResult<CMediaTrackInfos::VideoTrackInfo>	videoTrackInfo =
																	composeVideoTrackInfo(randomAccessDataSource,
																			options, stsdDescription.getType(),
																			timeScale, duration, internals);
				if (videoTrackInfo.hasValue())
					// Success
					mediaTrackInfos.add(*videoTrackInfo);
				else
					// Error
					return SMediaSource::ImportResult(videoTrackInfo.getError());
			}
		}
	}

	return SMediaSource::ImportResult(mediaTrackInfos);
}

//----------------------------------------------------------------------------------------------------------------------
TArray<SMediaPacketAndLocation> CMPEG4MediaFile::composePacketAndLocations(const Internals& internals) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	SMP4sttsAtomPayload&	sttsAtomPayload = internals.mSTTSAtomPayload;
	const	SMP4stscAtomPayload&	stscAtomPayload = internals.mSTSCAtomPayload;
	const	SMP4stszAtomPayload&	stszAtomPayload = internals.mSTSZAtomPayload;

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
	TNArray<SMediaPacketAndLocation>	packetAndLocations;
	for (UInt32 sttsChunkIndex = 0; sttsChunkIndex < sttsChunkCount; sttsChunkIndex++) {
		// Get packet info
		const	SMP4sttsAtomPayload::Chunk&	sttsChunk = sttsAtomPayload.getChunk(sttsChunkIndex);
				UInt32						sttsChunkPacketCount = sttsChunk.getPacketCount();
				UInt32						sttsChunkPacketDuration = sttsChunk.getPacketDuration();

		// Iterate packets
		for (UInt32 packetIndex = 0; packetIndex < sttsChunkPacketCount; packetIndex++, stszPacketByteCountIndex++) {
			// Get info
			UInt32	packetByteCount = stszAtomPayload.getPacketByteCount(stszPacketByteCountIndex);

			// Add Packet Location Info
			packetAndLocations +=
					SMediaPacketAndLocation(SMediaPacket(sttsChunkPacketDuration, packetByteCount),
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
TVResult<CMediaTrackInfos::AudioTrackInfo> CMPEG4MediaFile::composeAudioTrackInfo(
		const I<CRandomAccessDataSource>& randomAccessDataSource, SMediaSource::Options options, OSType type,
		UniversalTimeInterval duration, const Internals& internals)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check type
	switch (type) {
		case MAKE_OSTYPE('m', 'p', '4', 'a'): {
			// MPEG4 (AAC) Audio
			const	SMP4stsdMP4ADescription&	mp4ADescription =
														*((SMP4stsdMP4ADescription*) &internals.mSTSDDescription);

			// Get configuration data
			TIResult<CData>	configurationData = internals.getDecompressionData(sizeof(SMP4stsdMP4ADescription));
			ReturnValueIfResultError(configurationData,
					TVResult<CMediaTrackInfos::AudioTrackInfo>(configurationData.getError()))

			// Compose storage format
			OV<CAACAudioCodec::Info>	info =
												CAACAudioCodec::composeInfo(*configurationData,
														mp4ADescription.getChannels());
			if (!info.hasValue())
				return TVResult<CMediaTrackInfos::AudioTrackInfo>(
						CCodec::unsupportedConfigurationError(CString(type, true)));

			OI<SAudioStorageFormat>	audioStorageFormat = CAACAudioCodec::composeAudioStorageFormat(*info);
			if (!audioStorageFormat.hasInstance())
				// Unsupported configuration
				return TVResult<CMediaTrackInfos::AudioTrackInfo>(
						CCodec::unsupportedConfigurationError(CString(type, true)));

			// Compose info
			TArray<SMediaPacketAndLocation>	mediaPacketAndLocations = composePacketAndLocations(internals);
			UInt64							byteCount =
													SMediaPacketAndLocation::getTotalByteCount(mediaPacketAndLocations);

			// Add audio track
			CAudioTrack	audioTrack(CMediaTrack::composeInfo(duration, byteCount), *audioStorageFormat);
			if (options & SMediaSource::kCreateDecoders)
				// Add audio track with decode info
				return TVResult<CMediaTrackInfos::AudioTrackInfo>(
						CMediaTrackInfos::AudioTrackInfo(audioTrack,
								CAACAudioCodec::create(*info, randomAccessDataSource, mediaPacketAndLocations)));
			else
				// Add audio track
				return TVResult<CMediaTrackInfos::AudioTrackInfo>(CMediaTrackInfos::AudioTrackInfo(audioTrack));
			}

		default:
			// Unsupported audio codec
			return TVResult<CMediaTrackInfos::AudioTrackInfo>(CCodec::unsupportedError(CString(type, true)));
	}
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CMediaTrackInfos::VideoTrackInfo> CMPEG4MediaFile::composeVideoTrackInfo(
		const I<CRandomAccessDataSource>& randomAccessDataSource, SMediaSource::Options options, OSType type,
		UInt32 timeScale, UniversalTimeInterval duration, const Internals& internals)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check type
	switch (type) {
		case MAKE_OSTYPE('a', 'v', 'c', '1'): {
			// h.264 Video
			const	SMP4stsdH264Description&	h264Description =
														*((SMP4stsdH264Description*) &internals.mSTSDDescription);


			// Get configuration data
			TIResult<CData>	stssAtomPayloadData =
									internals.mAtomReader.readAtomPayload(
											internals.mSTBLContainerAtom.getAtom(MAKE_OSTYPE('s', 't', 's', 's')));
			ReturnValueIfResultError(stssAtomPayloadData,
					TVResult<CMediaTrackInfos::VideoTrackInfo>(stssAtomPayloadData.getError()));

			// Get configuration data
			TIResult<CData>	configurationData = internals.getDecompressionData(sizeof(SMP4stsdH264Description));
			ReturnValueIfResultError(configurationData,
					TVResult<CMediaTrackInfos::VideoTrackInfo>(configurationData.getError()))

			// Compose packet and locations
			TArray<SMediaPacketAndLocation>	mediaPacketAndLocations = composePacketAndLocations(internals);
			Float32							framerate =
													(Float32) mediaPacketAndLocations.getCount() / (Float32) duration;
			UInt64							byteCount =
													SMediaPacketAndLocation::getTotalByteCount(mediaPacketAndLocations);

			// Compose storage format
			OI<SVideoStorageFormat>	videoStorageFormat =
											CH264VideoCodec::composeVideoStorageFormat(
													S2DSizeU16(h264Description.getWidth(), h264Description.getHeight()),
													framerate);
			if (!videoStorageFormat.hasInstance())
				// Unsupported configuration
				return TVResult<CMediaTrackInfos::VideoTrackInfo>(
						CCodec::unsupportedConfigurationError(CString(type, true)));

			// Add video track
			CVideoTrack	videoTrack(CMediaTrack::composeInfo(duration, byteCount), *videoStorageFormat);
			if (options & SMediaSource::kCreateDecoders) {
				// Setup
				const	SMP4stssAtomPayload&	stssAtomPayload =
														*((SMP4stssAtomPayload*) stssAtomPayloadData->getBytePtr());
						UInt32					keyframesCount = stssAtomPayload.getKeyframesCount();
						TNumericArray<UInt32>	keyframeIndexes;
				for (UInt32 i = 0; i < keyframesCount; i++)
					// Add keyframe index
					keyframeIndexes += stssAtomPayload.getKeyframeIndex(i);

				// Add video track with decode info
				return TVResult<CMediaTrackInfos::VideoTrackInfo>(
						CMediaTrackInfos::VideoTrackInfo(videoTrack,
								CH264VideoCodec::create(randomAccessDataSource, mediaPacketAndLocations,
										*configurationData, timeScale, keyframeIndexes)));
			} else
				// Add video track
				return TVResult<CMediaTrackInfos::VideoTrackInfo>(CMediaTrackInfos::VideoTrackInfo(videoTrack));
			}

		default:
			// Unsupported video codec
			return TVResult<CMediaTrackInfos::VideoTrackInfo>(CCodec::unsupportedError(CString(type, true)));
	}
}

//----------------------------------------------------------------------------------------------------------------------
const void* CMPEG4MediaFile::getSampleDescription(const Internals& internals) const
//----------------------------------------------------------------------------------------------------------------------
{
	return &internals.mSTSDDescription;
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CData> CMPEG4MediaFile::getDecompressionData(const Internals& internals, SInt64 offset) const
//----------------------------------------------------------------------------------------------------------------------
{
	return internals.getDecompressionData(offset);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
static SMediaSource::ImportResult sImport(const I<CRandomAccessDataSource>& randomAccessDataSource,
		const OI<CAppleResourceManager>& appleResourceManager, SMediaSource::Options options)
//----------------------------------------------------------------------------------------------------------------------
{
	return CMPEG4MediaFile::create()->import(randomAccessDataSource, appleResourceManager, options);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Register media source

static	CString	sExtensions[] = { CString(OSSTR("m4a")), CString(OSSTR("m4v")), CString(OSSTR("mp4")) };

REGISTER_MEDIA_SOURCE(mp4,
		SMediaSource(MAKE_OSTYPE('m', 'p', '4', '*'), CString(OSSTR("MPEG 4")),
				TSArray<CString>(sExtensions, 3), sImport));
