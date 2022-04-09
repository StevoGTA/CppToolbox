//----------------------------------------------------------------------------------------------------------------------
//	CMPEG4MediaSource.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAACAudioCodec.h"
#include "CAtomReader.h"
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

// MP4A Audio Format (Sample Description detail)
struct SMP4AAudioFormat {
			// Methods
	UInt16	getChannels() const
				{ return EndianU16_BtoN(mChannels); }

	// Properties (in storage endian)
	private:
		UInt16	mQuickTimeEncodingVersion;
		UInt16	mQuickTimeEncodingRevisionLevel;
		OSType	mQuickTimeEncodingVendor;
		UInt16	mChannels;
		UInt16	mBits;
		UInt16	mQuickTimeCompressionID;
		UInt16	mQuickTimePacketSize;
		UInt32	mSampleRate;
};

// h.264 Video Format (Sample Description detail)
struct SH264VideoFormat {
			// Methods
	UInt16	getWidth() const
				{ return EndianU16_BtoN(mWidth); }
	UInt16	getHeight() const
				{ return EndianU16_BtoN(mHeight); }

	// Properties (in storage endian)
	private:
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
			OSType				getType() const
									{ return EndianU32_BtoN(mType); }
			SInt64				getFormatOffset() const
									{ return 16; }
	const	SMP4AAudioFormat&	getMP4AAudioFormat() const
									{ return mFormat.mMP4AAudioFormat; }
	const	SH264VideoFormat&	getH264VideoFormat() const
									{ return mFormat.mH264VideoFormat; }

	// Properties (in storage endian)
	private:
		UInt32				mLength;
		OSType				mType;
		UInt8				mReserved[6];
		UInt16				mDataRefIndex;
		union {
			SMP4AAudioFormat	mMP4AAudioFormat;
			SH264VideoFormat	mH264VideoFormat;
		}					mFormat;
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

//struct SMP4ALACSpecificConfig {
//	UInt32	mFrameLength;
//	UInt8	mCompatibleVersion;
//	UInt8	mBitDepth;
//	UInt8	mPB;
//	UInt8	mMB;
//	UInt8	mKB;
//	UInt8	mNumChannels;
//	UInt16	mMaxRun;
//	UInt32	mMaxFrameBytes;
//	UInt32	mAvgBitrate;
//	UInt32	mSampleRate;
//};
//
//struct SMP4ALACChannelLayoutInfo {
//	UInt32	mChannelLayoutInfoSize;
//	UInt32	mChannelLayoutInfoID;
//	UInt32	mVersionFlags;
//	UInt32	mChannelLayoutTag;
//	UInt32	mReserved1;
//	UInt32	mReserved2;
//};
//
//struct SMP4alacAtom {
//	UInt32				mByteCount;
//	OSType				mType;
//	UInt32				mVersion;
//
//	SALACSpecificConfig	mALACSpecificConfig;
//};
//
//struct SMP4alacAtomWithChannelLayout {
//	UInt32					mByteCount;
//	OSType					mType;
//	UInt32					mVersion;
//
//	SALACSpecificConfig		mALACSpecificConfig;
//	SALACChannelLayoutInfo	mALACChannelLayoutInfo;
//};
//
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

static	CString	sErrorDomain(OSSTR("CMPEG4MediaSource"));
static	SInt32	kUnsupportedCodecCode = 1;
static	SInt32	kUnsupportedCodecConfiguration = 2;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

static	SMediaSource::QueryTracksResult	sQueryTracksProc(const I<CSeekableDataSource>& seekableDataSource,
												const OR<const CAppleResourceManager>& appleResourceManager,
												SMediaSource::Options options);
static	OI<SError>						sAddAACAudioTrack(CMediaTrackInfos& mediaTrackInfos,
												const I<CSeekableDataSource>& seekableDataSource,
												SMediaSource::Options options,
												const SMP4stsdDescription& stsdDescription, const CData& configurationData,
												const SMP4mdhdAtomPayload& mdhdAtomPayload,
												const TArray<SMediaPacketAndLocation>& packetAndLocations);
static	OI<SError>						sAddH264VideoTrack(CMediaTrackInfos& mediaTrackInfos,
												const I<CSeekableDataSource>& seekableDataSource,
												SMediaSource::Options options,
												const SMP4stsdDescription& stsdDescription,
												const CData& configurationData,
												const SMP4mdhdAtomPayload& mdhdAtomPayload,
												const TArray<SMediaPacketAndLocation>& packetAndLocations,
												const OI<CData>& stssAtomPayloadData);
static	TArray<SMediaPacketAndLocation>	sComposePacketAndLocations(const SMP4sttsAtomPayload& sttsAtomPayload,
													const SMP4stscAtomPayload& stscAtomPayload,
													const SMP4stszAtomPayload& stszAtomPayload,
													SMP4stcoAtomPayload* stcoAtomPayload,
													SMP4co64AtomPayload* co64AtomPayload);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Register media source

CString	sMPEG4Extensions[] = { CString(OSSTR("m4a")), CString(OSSTR("m4v")), CString(OSSTR("mp4")) };

REGISTER_MEDIA_SOURCE(mp4,
		SMediaSource(MAKE_OSTYPE('m', 'p', '4', '*'), CString(OSSTR("MPEG 4")),
				TSArray<CString>(sMPEG4Extensions, 3), sQueryTracksProc));

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
SMediaSource::QueryTracksResult sQueryTracksProc(const I<CSeekableDataSource>& seekableDataSource,
		const OR<const CAppleResourceManager>& appleResourceManager, SMediaSource::Options options)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CAtomReader	atomReader(seekableDataSource);
	OI<SError>	error;

	// Read root atom
	TIResult<CAtomReader::Atom>	atom = atomReader.readAtom();
	ReturnValueIfResultError(atom, SMediaSource::QueryTracksResult());
	if (atom->mType != MAKE_OSTYPE('f', 't', 'y', 'p'))
		return SMediaSource::QueryTracksResult();

	// Find moov atom
	while (atom->mType != MAKE_OSTYPE('m', 'o', 'o', 'v')) {
		// Go to next atom
		error = atomReader.seekToNextAtom(*atom);
		ReturnValueIfError(error, SMediaSource::QueryTracksResult(*error));

		// Get atom
		atom = atomReader.readAtom();
		ReturnValueIfResultError(atom, SMediaSource::QueryTracksResult(atom.getError()));
	}

	TIResult<CAtomReader::ContainerAtom>	moovContainerAtom = atomReader.readContainerAtom(*atom);
	ReturnValueIfResultError(moovContainerAtom, SMediaSource::QueryTracksResult(moovContainerAtom.getError()));

	// Iterate moov atom
	CMediaTrackInfos	mediaTrackInfos;
	for (TIteratorD<CAtomReader::Atom> moovIterator = moovContainerAtom->getIterator();
			moovIterator.hasValue(); moovIterator.advance()) {
		// Check type
		if (moovIterator->mType == MAKE_OSTYPE('t', 'r', 'a', 'k')) {
			// Track
			TIResult<CAtomReader::ContainerAtom>	trakContainerAtom = atomReader.readContainerAtom(*moovIterator);
			if (trakContainerAtom.hasError()) continue;

//			//
//			OR<CAtomReader::Atom>	tkhdAtom = trakContainerAtom->getAtom(MAKE_OSTYPE('t', 'k', 'h', 'd'));
//			if (!tkhdAtom.hasReference()) continue;
//			TIResult<CData>	tkhdAtomPayloadData = atomReader.readAtomPayload(*tkhdAtom);
//			if (error.hasInstance()) continue;
//
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
			if (!stcoAtomPayloadData.hasValue() && !co64AtomPayloadData.hasValue()) continue;
			SMP4stcoAtomPayload*	stcoAtomPayload =
											stcoAtomPayloadData.hasValue() ?
													(SMP4stcoAtomPayload*) stcoAtomPayloadData->getBytePtr() :
													nil;
			SMP4co64AtomPayload*	co64AtomPayload =
											co64AtomPayloadData.hasValue() ?
													(SMP4co64AtomPayload*) co64AtomPayloadData->getBytePtr() :
													nil;

			// Check track type
			if (hdlrAtomPayload.getSubType() == MAKE_OSTYPE('s', 'o', 'u', 'n')) {
				// Audio track
				if (stsdDescription.getType() == MAKE_OSTYPE('m', 'p', '4', 'a')) {
					// MPEG4 (AAC) Audio
					TIResult<CData>	configurationData =
											atomReader.readAtomPayload(*stsdAtom,
													sizeof(SMP4stsdAtomPayload) + stsdDescription.getFormatOffset() +
															sizeof(SMP4AAudioFormat));
  					if (configurationData.hasError()) continue;

					// Add audio track
  					error =
							sAddAACAudioTrack(mediaTrackInfos, seekableDataSource, options, stsdDescription,
									*configurationData, SMP4mdhdAtomPayload(*mdhdAtomPayloadData),
									sComposePacketAndLocations(sttsAtomPayload, stscAtomPayload, stszAtomPayload,
											stcoAtomPayload, co64AtomPayload));
					ReturnValueIfError(error, SMediaSource::QueryTracksResult(*error));
				} else
					// Unsupported audio codec
					return SMediaSource::QueryTracksResult(
							SError(sErrorDomain, kUnsupportedCodecCode,
									CString(OSSTR("Unsupported audio codec: ")) +
											CString(stsdDescription.getType(), true, true)));
			} else if (hdlrAtomPayload.getSubType() == MAKE_OSTYPE('v', 'i', 'd', 'e')) {
				// Video track
				TIResult<CData>	stssAtomPayloadData =
										atomReader.readAtomPayload(
												stblContainerAtom->getAtom(MAKE_OSTYPE('s', 't', 's', 's')));
				if (stssAtomPayloadData.hasError()) continue;

				// Check type
				if (stsdDescription.getType() == MAKE_OSTYPE('a', 'v', 'c', '1')) {
					// h.264 Video
					TIResult<CData>	h264ConfigurationAtomPayloadData =
										atomReader.readAtomPayload(*stsdAtom,
												sizeof(SMP4stsdAtomPayload) + stsdDescription.getFormatOffset() +
														sizeof(SH264VideoFormat));
					if (h264ConfigurationAtomPayloadData.hasError()) continue;

					// Add video track
					error =
							sAddH264VideoTrack(mediaTrackInfos, seekableDataSource, options, stsdDescription,
									*h264ConfigurationAtomPayloadData, SMP4mdhdAtomPayload(*mdhdAtomPayloadData),
									sComposePacketAndLocations(sttsAtomPayload, stscAtomPayload, stszAtomPayload,
											stcoAtomPayload, co64AtomPayload),
									*stssAtomPayloadData);
					ReturnValueIfError(error, SMediaSource::QueryTracksResult(*error));
				} else
					// Unsupported video codec
					return SMediaSource::QueryTracksResult(
							SError(sErrorDomain, kUnsupportedCodecCode,
									CString(OSSTR("Unsupported video codec: ")) +
											CString(stsdDescription.getType(), true, true)));
			}
		}
	}

	return SMediaSource::QueryTracksResult(mediaTrackInfos);
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> sAddAACAudioTrack(CMediaTrackInfos& mediaTrackInfos, const I<CSeekableDataSource>& seekableDataSource,
		SMediaSource::Options options, const SMP4stsdDescription& stsdDescription, const CData& configurationData,
		const SMP4mdhdAtomPayload& mdhdAtomPayload, const TArray<SMediaPacketAndLocation>& packetAndLocations)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	SMP4AAudioFormat&	audioFormat = stsdDescription.getMP4AAudioFormat();

	// Compose storage format
	OI<SAudioStorageFormat>	audioStorageFormat =
									CAACAudioCodec::composeAudioStorageFormat(configurationData,
											audioFormat.getChannels());
	if (!audioStorageFormat.hasInstance())
		// Unsupported configuration
		return OI<SError>(
				SError(sErrorDomain, kUnsupportedCodecConfiguration,
						CString(OSSTR("Unsupported ")) + CAACAudioCodec::mAACLCName +
								CString(OSSTR(" codec configuration"))));

	// Compose info
	UniversalTimeInterval	duration =
									(UniversalTimeInterval) mdhdAtomPayload.getDuration() /
											(UniversalTimeInterval) mdhdAtomPayload.getTimeScale();
	UInt64					byteCount = SMediaPacketAndLocation::getTotalByteCount(packetAndLocations);

	// Add audio track
	CAudioTrack	audioTrack(CMediaTrack::Info(duration, (UInt32) (((UniversalTimeInterval) byteCount * 8) / duration)),
						*audioStorageFormat);
	if (options & SMediaSource::kCreateDecoders)
		// Add audio track with decode info
		mediaTrackInfos.add(
				CMediaTrackInfos::AudioTrackInfo(audioTrack,
						CAACAudioCodec::create(*audioStorageFormat, seekableDataSource, packetAndLocations,
								configurationData)));
	else
		// Add audio track
		mediaTrackInfos.add(CMediaTrackInfos::AudioTrackInfo(audioTrack));

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> sAddH264VideoTrack(CMediaTrackInfos& mediaTrackInfos, const I<CSeekableDataSource>& seekableDataSource,
		SMediaSource::Options options, const SMP4stsdDescription& stsdDescription, const CData& configurationData,
		const SMP4mdhdAtomPayload& mdhdAtomPayload, const TArray<SMediaPacketAndLocation>& packetAndLocations,
		const OI<CData>& stssAtomPayloadData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	SH264VideoFormat&		videoFormat = stsdDescription.getH264VideoFormat();
			UInt32					timeScale = mdhdAtomPayload.getTimeScale();
			UniversalTimeInterval	duration =
											(UniversalTimeInterval) mdhdAtomPayload.getDuration() /
													(UniversalTimeInterval) timeScale;
			Float32					framerate =
											(Float32) ((UniversalTimeInterval) packetAndLocations.getCount() /
													duration);
			UInt64					byteCount = SMediaPacketAndLocation::getTotalByteCount(packetAndLocations);

	// Compose storage format
	OI<SVideoStorageFormat>	videoStorageFormat =
									CH264VideoCodec::composeVideoStorageFormat(
											S2DSizeU16(videoFormat.getWidth(), videoFormat.getHeight()), framerate);
	if (!videoStorageFormat.hasInstance())
		// Unsupported configuration
		return OI<SError>(
				SError(sErrorDomain, kUnsupportedCodecConfiguration,
						CString(OSSTR("Unsupported ")) + CH264VideoCodec::mName +
								CString(OSSTR(" codec configuration"))));

	// Add video track
	CVideoTrack	videoTrack(CMediaTrack::Info(duration, (UInt32) (((UniversalTimeInterval) byteCount * 8) / duration)),
						*videoStorageFormat);
	if (options & SMediaSource::kCreateDecoders) {
		// Setup
		const	SMP4stssAtomPayload&	stssAtomPayload = *((SMP4stssAtomPayload*) stssAtomPayloadData->getBytePtr());
				UInt32					keyframesCount = stssAtomPayload.getKeyframesCount();
				TNumericArray<UInt32>	keyframeIndexes;
		for (UInt32 i = 0; i < keyframesCount; i++)
			// Add keyframe index
			keyframeIndexes += stssAtomPayload.getKeyframeIndex(i);

		// Add video track with decode info
		mediaTrackInfos.add(
				CMediaTrackInfos::VideoTrackInfo(videoTrack,
						CH264VideoCodec::create(seekableDataSource, packetAndLocations, configurationData, timeScale,
								keyframeIndexes)));
	} else
		// Add video track
		mediaTrackInfos.add(CMediaTrackInfos::VideoTrackInfo(videoTrack));

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
TArray<SMediaPacketAndLocation> sComposePacketAndLocations(const SMP4sttsAtomPayload& sttsAtomPayload,
		const SMP4stscAtomPayload& stscAtomPayload, const SMP4stszAtomPayload& stszAtomPayload,
		SMP4stcoAtomPayload* stcoAtomPayload, SMP4co64AtomPayload* co64AtomPayload)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNArray<SMediaPacketAndLocation>	packetAndLocations;

	// Construct info
	UInt32	stszPacketByteCountIndex = 0;

	UInt32	stscPacketGroupInfoIndex = 0;
	UInt32	stscPacketGroupInfoPacketCount =
					stscAtomPayload.getPacketGroupInfo(stscPacketGroupInfoIndex).getPacketCount();

	UInt32	stcoBlockOffsetIndex = 0;

	UInt32	currentBlockPacketIndex = 0;
	UInt64	currentByteOffset =
					(stcoAtomPayload != nil) ?
							stcoAtomPayload->getPacketGroupOffset(stcoBlockOffsetIndex) :
							co64AtomPayload->getPacketGroupOffset(stcoBlockOffsetIndex);

	// Iterate all stts entries
	UInt32	sttsChunkCount = sttsAtomPayload.getChunkCount();
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
				UInt32	blockOffsetCount =
								(stcoAtomPayload != nil) ?
										stcoAtomPayload->getPacketGroupOffsetCount() :
										co64AtomPayload->getPacketGroupOffsetCount();
				if (++stcoBlockOffsetIndex < blockOffsetCount) {
					// Update info
					currentBlockPacketIndex = 0;
					currentByteOffset =
							(stcoAtomPayload != nil) ?
									stcoAtomPayload->getPacketGroupOffset(stcoBlockOffsetIndex) :
									co64AtomPayload->getPacketGroupOffset(stcoBlockOffsetIndex);

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
