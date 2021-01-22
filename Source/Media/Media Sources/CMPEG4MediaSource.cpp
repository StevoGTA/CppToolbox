//----------------------------------------------------------------------------------------------------------------------
//	CMPEG4MediaSource.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMPEG4MediaSource.h"

#include "CAACAudioCodec.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

/*
	Info on MPEG 4 files can be found here:
		http://www.geocities.com/xhelmboyx/quicktime/formats/mp4-layout.txt (no longer active)
		http://atomicparsley.sourceforge.net/mpeg-4files.html
*/

#pragma pack(push,1)

#if TARGET_OS_WINDOWS
	#pragma warning(disable:4200)
#endif

//struct SGeneralAtomInfo {
//	UInt8	mVersion;		// 0
//	UInt8	mFlags[3];
//};
//
//struct SftypAtomInfo {
//	OSType	mMajorBrand;
//	UInt32	mMajorBrandVersion;
//	OSType	mCompatibleBrands[];
//};

struct ShdlrAtomPayload {
			// Methods
	OSType	getSubType() const { return EndianU32_BtoN(mSubType); }

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

//struct SmvhdAtomInfoV0 {
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
//struct SmvhdAtomInfoV1 {
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
//struct StkhdAtomInfoV0 {
//	UInt8	mVersion;	// 0
//	UInt8	mFlags[3];	// 0x000001:	Track Enabled
//						// 0x000002:	Track In Movie
//						// 0x000004:	Track In Preview
//						// 0x000008:	Track In Poster
//
//	UInt32	mCreatedDate;
//	UInt32	mModifiedDate;
//
//	UInt32	mTrackID;
//	UInt32	mReserved;
//	UInt32	mDuration;
//	UInt32	mReserved1[2];
//	UInt16	mVideoLayer;
//	UInt16	mQuickTimeAlternateID;
//	UInt16	mAudioVolume;
//	UInt16	mReserved4;
//	UInt32	mMatrixA;		// Width Scale
//	UInt32	mMatrixB;		// Width Rotate
//	UInt32	mMatrixU;		// Width Angle
//	UInt32	mMatrixC;		// Height Rotate
//	UInt32	mMatrixD;		// Height Scale
//	UInt32	mMatrixV;		// Height Angle
//	UInt32	mMatrixX;
//	UInt32	mMatrixY;
//	UInt32	mMatrixW;
//	UInt32	mFrameWidth;
//	UInt32	mFrameHeight;
//};
//
//struct StkhdAtomInfoV1 {
//	UInt8	mVersion;	// 0
//	UInt8	mFlags[3];	// 0x000001:	Track Enabled
//						// 0x000002:	Track In Movie
//						// 0x000004:	Track In Preview
//						// 0x000008:	Track In Poster
//
//	UInt64	mCreatedDate;
//	UInt64	mModifiedDate;
//
//	UInt32	mTrackID;
//	UInt32	mReserved;
//	UInt64	mDuration;
//	UInt32	mReserved1[2];
//	UInt16	mVideoLayer;
//	UInt16	mQuickTimeAlternateID;
//	UInt16	mAudioVolume;
//	UInt16	mReserved4;
//	UInt32	mMatrixA;		// Width Scale
//	UInt32	mMatrixB;		// Width Rotate
//	UInt32	mMatrixU;		// Width Angle
//	UInt32	mMatrixC;		// Height Rotate
//	UInt32	mMatrixD;		// Height Scale
//	UInt32	mMatrixV;		// Height Angle
//	UInt32	mMatrixX;
//	UInt32	mMatrixY;
//	UInt32	mMatrixW;
//	UInt32	mFrameWidth;
//	UInt32	mFrameHeight;
//};
//
//struct SmdhdAtomInfoV0 {
//	UInt8	mVersion;			// 0
//	UInt8	mFlags[3];
//
//	UInt32	mCreatedDate;
//	UInt32	mModifiedDate;
//
//	UInt32	mTimeScale;
//	UInt32	mDuration;
//
//	UInt16	mLanguageCode;
//
//	UInt16	mQuickTimeQuality;
//};
//
//struct SmdhdAtomInfoV1 {
//	UInt8	mVersion;			// 1
//	UInt8	mFlags[3];
//
//	UInt64	mCreatedDate;
//	UInt64	mModifiedDate;
//
//	UInt32	mTimeScale;
//	UInt64	mDuration;
//
//	UInt16	mLanguageCode;
//
//	UInt16	mQuickTimeQuality;
//};
//
//struct SsmhdAtomInfo {
//	UInt8	mVersion;								// 0
//	UInt8	mFlags[3];
//
//	UInt16	mAudioBalance;
//	UInt8	mReserved[2];
//};
//
//struct SdrefAtomInfo {
//	UInt8	mVersion;								// 0
//	UInt8	mFlags[3];
//
//	UInt32	mReferenceCount;
//};
//
//struct SurlAtomInfo {
//	UInt8	mVersion;								// 0
//	UInt8	mFlags[3];
//};

//enum EESDSDescriptorType {
//	kESDSDescriptorTypeES 					= 0x03,
//	kESDSDescriptorTypeDecoderConfig		= 0x04,
//	kESDSDescriptorTypeDecoderSpecificInfo	= 0x05,
//	kESDSDescriptorTypeSyncLayerConfig		= 0x06,
//};

struct SesdsDecoderSpecificDescriptor {
			// Methods
	CData	getStartCodes() const
				{
					// Check for minimal/extended
					if (_.mExtendedInfo.mExtendedDescriptorType[0] != 0x80)
						// Minimal
						return CData(_.mMinimalInfo.mStartCodes, _.mMinimalInfo.mDescriptorLength);
					else
						// Extended
						return CData(_.mExtendedInfo.mStartCodes, _.mExtendedInfo.mDescriptorLength);
				}

	// Properties (in storage endian)
	private:
		UInt8	mDescriptorType;					// 5
		union {
			struct MinimalInfo {
				UInt8	mDescriptorLength;			// size of 5

				UInt8	mStartCodes[];
			} mMinimalInfo;
			struct ExtendedInfo {
				UInt8	mExtendedDescriptorType[3];
				UInt8	mDescriptorLength;			// size of 5

				UInt8	mStartCodes[];
			} mExtendedInfo;
		} _;
};

struct SesdsDecoderConfigDescriptor {
											// Methods
			UInt32							getLength() const
												{
													// Check for minimal/extended
													if (_.mExtendedInfo.mExtendedDescriptorType[0] != 0x80)
														// Minimal
														return _.mMinimalInfo.mDescriptorLength;
													else
														// Extended
														return _.mExtendedInfo.mDescriptorLength;
												}
	const	SesdsDecoderSpecificDescriptor&	getDecoderSpecificDescriptor() const
												{
													// Check for minimal/extended
													if (_.mExtendedInfo.mExtendedDescriptorType[0] != 0x80)
														// Minimal
														return _.mMinimalInfo.mDecoderSpecificDescriptor;
													else
														// Extended
														return _.mExtendedInfo.mDecoderSpecificDescriptor;
												}

	// Properties (in storage endian)
	private:
		UInt8							mDescriptorType;					// 4
		union {
			struct MinimalInfo {
				UInt8							mDescriptorLength;			// size of 4 + 5

				UInt8							mObjectTypeID;
				UInt8							mStreamTypeAndFlags;
				UInt8							mBufferSize[3];
				UInt32							mMaximumBitrate;
				UInt32							mAverageBitrate;
				SesdsDecoderSpecificDescriptor	mDecoderSpecificDescriptor;
			} mMinimalInfo;
			struct ExtendedInfo {
				UInt8							mExtendedDescriptorType[3];
				UInt8							mDescriptorLength;			// size of 4 + 5

				UInt8							mObjectTypeID;
				UInt8							mStreamTypeAndFlags;
				UInt8							mBufferSize[3];
				UInt32							mMaximumBitrate;
				UInt32							mAverageBitrate;
				SesdsDecoderSpecificDescriptor	mDecoderSpecificDescriptor;
			} mExtendedInfo;
		} _;
};

struct SesdsSyncLayerDescriptor {
			// Methods

	// Properties (in storage endian)
	private:
		UInt8	mDescriptorType;					// 6
		union {
			struct MinimalInfo {
				UInt8	mDescriptorLength;			// size of 6

				UInt8	mSyncLayerValue;
			} mMinimalInfo;
			struct ExtendedInfo {
				UInt8	mExtendedDescriptorType[3];
				UInt8	mDescriptorLength;			// size of 6

				UInt8	mSyncLayerValue;
			} mExtendedInfo;
		} _;
};

struct SesdsDescriptor {
											// Methods
	const	SesdsDecoderConfigDescriptor&	getDecoderConfigDescriptor() const
												{
													// Check for minimal/extended
													if (_.mExtendedInfo.mExtendedDescriptorType[0] != 0x80)
														// Minimal
														return *((SesdsDecoderConfigDescriptor*)
																&_.mMinimalInfo.mChildDescriptorData);
													else
														// Extended
														return *((SesdsDecoderConfigDescriptor*)
																&_.mExtendedInfo.mChildDescriptorData);
												}
	const	SesdsSyncLayerDescriptor&		getSyncLayerDescriptor() const
												{
													// Setup
													UInt32	offset = getDecoderConfigDescriptor().getLength();

													// Check for minimal/extended
													if (_.mExtendedInfo.mExtendedDescriptorType[0] != 0x80)
														// Minimal
														return *((SesdsSyncLayerDescriptor*)
																&_.mMinimalInfo.mChildDescriptorData[offset]);
													else
														// Extended
														return *((SesdsSyncLayerDescriptor*)
																&_.mExtendedInfo.mChildDescriptorData[offset]);
												}

	// Properties (in storage endian)
	private:
		UInt8	mDescriptorType;					// 3
		union {
			struct MinimalInfo {
				UInt8	mDescriptorLength;			// size of 3 + 4 + 5 + 6

				UInt16	mESID;
				UInt8	mStreamPriority;
				UInt8	mChildDescriptorData[];
			} mMinimalInfo;

			struct ExtendedInfo {
				UInt8	mExtendedDescriptorType[3];
				UInt8	mDescriptorLength;			// size of 3 + 4 + 5 + 6

				UInt16	mESID;
				UInt8	mStreamPriority;
				UInt8	mChildDescriptorData[];
			} mExtendedInfo;
		} _;
};

struct SesdsInfo {
			// Methods
			UInt32				getSize() const { return EndianU32_BtoN(mHeader.mSize); }

	const	SesdsDescriptor&	getDescriptor() const { return mESDSDescriptor; }
			UInt32				getDescriptorSize() const { return getSize() - sizeof(Header); }

	// Properties (in storage endian)
	private:
		struct Header {
			UInt32	mSize;
			OSType	mType;	// 'esds'

			UInt8	mVersion;
			UInt8	mFlags[3];
		} mHeader;
		SesdsDescriptor	mESDSDescriptor;
};

struct SMP4AAudioFormat {
						// Methods
			UInt16		getBits() const { return EndianU16_BtoN(mBits); }
			UInt16		getChannels() const { return EndianU16_BtoN(mChannels); }
	const	SesdsInfo&	getESDSInfo() const { return mESDSInfo; }

	// Properties (in storage endian)
	private:
		UInt8		mReserved[6];
		UInt16		mDataRefIndex;
		UInt16		mQuickTimeEncodingVersion;
		UInt16		mQuickTimeEncodingRevisionLevel;
		OSType		mQuickTimeEncodingVendor;
		UInt16		mChannels;
		UInt16		mBits;
		UInt16		mQuickTimeCompressionID;
		UInt16		mQuickTimePacketSize;
		UInt32		mSampleRate;
		SesdsInfo	mESDSInfo;
};

struct SstsdDescription {
			// Methods
			OSType				getType() const { return EndianU32_BtoN(mType); }
	const	SMP4AAudioFormat&	getMP4AAudioFormat() const { return mFormat.mMP4AAudioFormat; }

	// Properties (in storage endian)
	private:
		UInt32				mLength;
		OSType				mType;
		union {
			SMP4AAudioFormat	mMP4AAudioFormat;
		}					mFormat;
};

struct SstsdAtomPayload {
									// Methods
	const	SstsdDescription&	getDescription() const { return mDescriptions[0]; }

	// Properties (in storage endian)
	private:
		UInt8				mVersion;				// 0
		UInt8				mFlags[3];
		UInt32				mDescriptionCount;
		SstsdDescription	mDescriptions[];
};

//struct SALACSpecificConfig {
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
//struct SALACChannelLayoutInfo {
//	UInt32	mChannelLayoutInfoSize;
//	UInt32	mChannelLayoutInfoID;
//	UInt32	mVersionFlags;
//	UInt32	mChannelLayoutTag;
//	UInt32	mReserved1;
//	UInt32	mReserved2;
//};
//
//struct SalacAtom {
//	UInt32				mSize;
//	OSType				mType;
//	UInt32				mVersion;
//
//	SALACSpecificConfig	mALACSpecificConfig;
//};
//
//struct SalacAtomWithChannelLayout {
//	UInt32					mSize;
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

struct SsttsAtomPayload {
	// Structs
	struct PacketInfo {
				// Methods
		UInt32	getPacketCount() const { return EndianU32_BtoN(mPacketCount); }
		UInt32	getSamplesPerPacket() const { return EndianU32_BtoN(mSamplesPerPacket); }

		// Properties (in storage endian)
		private:
			UInt32	mPacketCount;
			UInt32	mSamplesPerPacket;
	};

						// Methods
			UInt32		getPacketInfoCount() const { return EndianU32_BtoN(mPacketInfoCount); }
	const	PacketInfo&	getPacketInfo(UInt32 index) const { return mPacketInfos[index]; }

	// Properties (in storage endian)
	private:
		UInt8		mVersion;
		UInt8		mFlags[3];

		// In each entry, the number of samples per packet must be the same
		// Total packets = sum of packet counts
		UInt32		mPacketInfoCount;
		PacketInfo	mPacketInfos[];
};

struct SstscAtomPayload {
	// Structs
	struct BlockGroupInfo {
		// Methods
		UInt32	getStartIndex() const { return EndianU32_BtoN(mStartIndex); }
		UInt32	getPacketsPerBlock() const { return EndianU32_BtoN(mPacketsPerBlock); }
		UInt32	getSampleDescriptionIndex() const { return EndianU32_BtoN(mSampleDescriptionIndex); }

		// Properties (in storage endian)
		private:
			UInt32	mStartIndex;
			UInt32	mPacketsPerBlock;
			UInt32	mSampleDescriptionIndex;
	};

							// Methods
			UInt32			getBlockGroupCount() const { return EndianU32_BtoN(mBlockGroupCount); }
	const	BlockGroupInfo&	getBlockGroupInfo(UInt32 index) const { return mBlockGroupInfos[index]; }

	// Properties (in storage endian)
	private:
		UInt8			mVersion;
		UInt8			mFlags[3];

		// In each group, the number of packets must be the same
		// Total blocks = highest block start index + (1 if cumulative packet count is less than total packet count)
		UInt32			mBlockGroupCount;
		BlockGroupInfo	mBlockGroupInfos[];
};

struct SstszAtomPayload {
			// Methods
	UInt32	getPacketSize(UInt32 index) const
					{ return (mGlobalPacketSize != 0) ?
							EndianU32_BtoN(mGlobalPacketSize) : EndianU32_BtoN(mPacketSizes[index]); }

	// Properties (in storage endian)
	private:
		UInt8	mVersion;
		UInt8	mFlags[3];

		// If every packet has the same size, specify it here
		UInt32	mGlobalPacketSize;	// 0 means use packet sizes below

		// For every individual packet, specify the packet size
		UInt32	mPacketSizeCount;
		UInt32	mPacketSizes[];			// Packet size in bytes
};

struct SstcoAtomPayload {
			// Methods
	UInt32	getBlockOffsetCount() const { return EndianU32_BtoN(mBlockOffsetCount); }
	UInt64	getBlockOffset(UInt32 index) const { return EndianU32_BtoN(mBlockOffsets[index]); }

	// Properties (in storage endian)
	private:
		UInt8	mVersion;
		UInt8	mFlags[3];

		// For each block specified in the stsc Atom, the file offset is specified
		UInt32	mBlockOffsetCount;
		UInt32	mBlockOffsets[];		// Offset to start of block data from start of file
};

struct Sco64AtomPayload {
			// Methods
	UInt32	getBlockOffsetCount() const { return EndianU32_BtoN(mBlockOffsetCount); }
	UInt64	getBlockOffset(UInt32 index) const { return EndianU64_NtoB(mBlockOffsets[index]); }

	// Properties (in storage endian)
	private:
		UInt8	mVersion;
		UInt8	mFlags[3];

		// For each block specified in the stsc Atom, the file offset is specified
		UInt32	mBlockOffsetCount;
		UInt64	mBlockOffsets[];		// Offset to start of block data from start of file
};

//struct SmetaAtomInfo {
//	UInt8	mVersion;
//	UInt8	mFlags[3];
//};
//
//struct SdataAtomInfo {
//	UInt8	mVersion;
//	UInt8	mFlags[3];
//
//	UInt32	mReserved;
//
//	UInt8	mData[];
//};

#if TARGET_OS_WINDOWS
	#pragma warning(default:4200)
#endif

#pragma pack(pop)

static	CString	sErrorDomain(OSSTR("CMPEG4MediaSource"));
static	SError	sNotAnMPEG4FileError(sErrorDomain, 1, CString(OSSTR("Not an MPEG-4 file")));
static	SError	sUnsupportedCodecError(sErrorDomain, 2, CString(OSSTR("Unsupported codec")));

#define ReturnNotAnMPEG4FileIf(check)	{ if (check) return OI<SError>(sNotAnMPEG4FileError); }

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMPEG4MediaSourceInternals

class CMPEG4MediaSourceInternals {
	public:
													CMPEG4MediaSourceInternals() {}

		void										addMP4AAudioTrack(const SstsdDescription& stsdDescription,
															const TArray<CAudioCodec::PacketLocation>& packetLocations);

		static	TArray<CAudioCodec::PacketLocation>	composePacketLocations(const SsttsAtomPayload& sttsAtomPayload,
															const SstscAtomPayload& stscAtomPayload,
															const SstszAtomPayload& stszAtomPayload,
															SstcoAtomPayload* stcoAtomPayload,
															Sco64AtomPayload* co64AtomPayload);

		TNArray<CAudioTrack>	mAudioTracks;
};

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CMPEG4MediaSourceInternals::addMP4AAudioTrack(const SstsdDescription& stsdDescription,
		const TArray<CAudioCodec::PacketLocation>& packetLocations)
//----------------------------------------------------------------------------------------------------------------------
{
	// MPEG 4 Audio
	const	SMP4AAudioFormat&				audioFormat = stsdDescription.getMP4AAudioFormat();
	const	SesdsInfo&						esdsInfo = audioFormat.getESDSInfo();
	const	SesdsDescriptor&				descriptor = esdsInfo.getDescriptor();
	const	SesdsDecoderConfigDescriptor&	decoderConfigDescriptor = descriptor.getDecoderConfigDescriptor();
	const	SesdsDecoderSpecificDescriptor&	decoderSpecificDescriptor =
													decoderConfigDescriptor.getDecoderSpecificDescriptor();
			CData							startCodesData = decoderSpecificDescriptor.getStartCodes();
			UInt16							startCodes = EndianU16_BtoN(*((UInt16*) startCodesData.getBytePtr()));

	// Compose audio storage format
	OI<SAudioStorageFormat>	audioStorageFormat =
									CAACAudioCodec::composeAudioStorageFormat(startCodes, audioFormat.getChannels());
	if (!audioStorageFormat.hasInstance())
		// Unknown
		return;

	// Add audio track
	mAudioTracks +=
			CAudioTrack(*audioStorageFormat,
					I<CAudioCodec::DecodeInfo>(
							new CAACAudioCodec::DecodeInfo(packetLocations,
									CData(&descriptor, esdsInfo.getDescriptorSize()), startCodes)));
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TArray<CAudioCodec::PacketLocation> CMPEG4MediaSourceInternals::composePacketLocations(
		const SsttsAtomPayload& sttsAtomPayload, const SstscAtomPayload& stscAtomPayload,
		const SstszAtomPayload& stszAtomPayload, SstcoAtomPayload* stcoAtomPayload, Sco64AtomPayload* co64AtomPayload)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNArray<CAudioCodec::PacketLocation>	packetLocations;

	// Construct info
	UInt32	stszPacketSizeIndex = 0;

	UInt32	stscBlockGroupIndex = 0;
	UInt32	stscPacketsPerBlock = stscAtomPayload.getBlockGroupInfo(stscBlockGroupIndex).getPacketsPerBlock();

	UInt32	stcoBlockOffsetIndex = 0;

	UInt32	currentBlockPacketIndex = 0;
	UInt64	currentByteOffset =
					(stcoAtomPayload != nil) ?
							stcoAtomPayload->getBlockOffset(stcoBlockOffsetIndex) :
							co64AtomPayload->getBlockOffset(stcoBlockOffsetIndex);

	// Iterate all stts packet infos
	UInt32	sttsPacketInfoCount = sttsAtomPayload.getPacketInfoCount();
	for (UInt32 sttsPacketInfoIndex = 0; sttsPacketInfoIndex < sttsPacketInfoCount; sttsPacketInfoIndex++) {
		// Get packet info
		const	SsttsAtomPayload::PacketInfo&	packetInfo = sttsAtomPayload.getPacketInfo(sttsPacketInfoIndex);
				UInt32							packetCount = packetInfo.getPacketCount();
				UInt32							samplesPerPacket = packetInfo.getSamplesPerPacket();

		// Iterate packets
		for (UInt32 packetIndex = 0; packetIndex < packetCount; packetIndex++, stszPacketSizeIndex++) {
			// Get info
			UInt32	packetSize = stszAtomPayload.getPacketSize(stszPacketSizeIndex);

			// Add Audio Packet Location Info
			packetLocations +=
					CAudioCodec::PacketLocation(CAudioCodec::Packet(samplesPerPacket, packetSize),
							currentByteOffset);

			// Update
			if (++currentBlockPacketIndex < stscPacketsPerBlock)
				// Still more to go in this block
				currentByteOffset += packetSize;
			else {
				// Finished with this block
				UInt32	blockOffsetCount =
								(stcoAtomPayload != nil) ?
										stcoAtomPayload->getBlockOffsetCount() :
										co64AtomPayload->getBlockOffsetCount();
				if (++stcoBlockOffsetIndex < blockOffsetCount) {
					// Update info
					currentBlockPacketIndex = 0;
					currentByteOffset =
							(stcoAtomPayload != nil) ?
									stcoAtomPayload->getBlockOffset(stcoBlockOffsetIndex) :
									co64AtomPayload->getBlockOffset(stcoBlockOffsetIndex);

					// Check if have more block groups
					if ((stscBlockGroupIndex + 1) < stscAtomPayload.getBlockGroupCount()) {
						// Check if next block group
						UInt32	nextBlockStartIndex =
										stscAtomPayload.getBlockGroupInfo(stscBlockGroupIndex + 1).getStartIndex();
						if ((stcoBlockOffsetIndex + 1) ==
								nextBlockStartIndex) {
							// Next block group
							stscBlockGroupIndex++;
							stscPacketsPerBlock =
									stscAtomPayload.getBlockGroupInfo(stscBlockGroupIndex).getPacketsPerBlock();
						}
					}
				}
			}
		}
	}

	return packetLocations;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMPEG4MediaSource

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CMPEG4MediaSource::CMPEG4MediaSource(const CByteParceller& byteParceller) : CAtomMediaSource(byteParceller)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CMPEG4MediaSourceInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CMPEG4MediaSource::~CMPEG4MediaSource()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CMediaSource methods

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CMPEG4MediaSource::loadTracks()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	OI<SError>	error;

	// Reset to beginning
	error = reset();
	ReturnErrorIfError(error);

	// Read root atom
	OI<AtomInfo>	atomInfo = getAtomInfo(error);
	ReturnErrorIfError(error);
	ReturnNotAnMPEG4FileIf(atomInfo->mType != MAKE_OSTYPE('f', 't', 'y', 'p'));

	// Find moov atom
	while (atomInfo->mType != MAKE_OSTYPE('m', 'o', 'o', 'v')) {
		// Go to next atom
		error = seekToNextAtom(*atomInfo);
		ReturnErrorIfError(error);

		// Get atom
		atomInfo = getAtomInfo(error);
		ReturnErrorIfError(error);
	}

	OI<AtomGroup>	moovAtomGroup = getAtomGroup(*atomInfo, error);
	ReturnErrorIfError(error);

	for (TIteratorD<AtomInfo> moovIterator = moovAtomGroup->getIterator(); moovIterator.hasValue();
			moovIterator.advance()) {
		// Check type
		if (moovIterator->mType == MAKE_OSTYPE('t', 'r', 'a', 'k')) {
			// trak
			OI<AtomGroup>	trakAtomGroup = getAtomGroup(*moovIterator, error);
			if (error.hasInstance()) continue;

			// Media
			OR<AtomInfo>	mdiaAtomInfo = trakAtomGroup->getAtomInfo(MAKE_OSTYPE('m', 'd', 'i', 'a'));
			if (!mdiaAtomInfo.hasReference()) continue;
			OI<AtomGroup>	mdiaAtomGroup = getAtomGroup(*mdiaAtomInfo, error);
			if (error.hasInstance()) continue;

			// Handler
			OR<AtomInfo>	hdlrAtomInfo = mdiaAtomGroup->getAtomInfo(MAKE_OSTYPE('h', 'd', 'l', 'r'));
			if (!hdlrAtomInfo.hasReference()) continue;
			OI<CData>	hdlrAtomPayloadData = getAtomPayload(*hdlrAtomInfo, error);
			if (error.hasInstance()) continue;
			const	ShdlrAtomPayload&	hdlrAtomPayload = *((ShdlrAtomPayload*) hdlrAtomPayloadData->getBytePtr());

			// Media Information
			OR<AtomInfo>	minfAtomInfo = mdiaAtomGroup->getAtomInfo(MAKE_OSTYPE('m', 'i', 'n', 'f'));
			if (!minfAtomInfo.hasReference()) continue;
			OI<AtomGroup>	minfAtomGroup = getAtomGroup(*minfAtomInfo, error);
			if (error.hasInstance()) continue;

			// Sample Table
			OR<AtomInfo>	stblAtomInfo = minfAtomGroup->getAtomInfo(MAKE_OSTYPE('s', 't', 'b', 'l'));
			if (!stblAtomInfo.hasReference()) continue;
			OI<AtomGroup>	stblAtomGroup = getAtomGroup(*stblAtomInfo, error);
			if (error.hasInstance()) continue;

			// Sample Table Sample Description
			OR<AtomInfo>	stsdAtomInfo = stblAtomGroup->getAtomInfo(MAKE_OSTYPE('s', 't', 's', 'd'));
			if (!stsdAtomInfo.hasReference()) continue;
			OI<CData>	stsdAtomPayloadData = getAtomPayload(*stsdAtomInfo, error);
			if (error.hasInstance()) continue;
			const	SstsdAtomPayload&	stsdAtomPayload = *((SstsdAtomPayload*) stsdAtomPayloadData->getBytePtr());
			const	SstsdDescription&	stsdDescription = stsdAtomPayload.getDescription();	// Only processing first

			// Sample Table Time to Sample
			OR<AtomInfo>	sttsAtomInfo = stblAtomGroup->getAtomInfo(MAKE_OSTYPE('s', 't', 't', 's'));
			if (!sttsAtomInfo.hasReference()) continue;
			OI<CData>	sttsAtomPayloadData = getAtomPayload(*sttsAtomInfo, error);
			if (error.hasInstance()) continue;
			const	SsttsAtomPayload&	sttsAtomPayload = *((SsttsAtomPayload*) sttsAtomPayloadData->getBytePtr());

			// Sample Table Sample Blocks
			OR<AtomInfo>	stscAtomInfo = stblAtomGroup->getAtomInfo(MAKE_OSTYPE('s', 't', 's', 'c'));
			if (!stscAtomInfo.hasReference()) continue;
			OI<CData>	stscAtomPayloadData = getAtomPayload(*stscAtomInfo, error);
			if (error.hasInstance()) continue;
			const	SstscAtomPayload&	stscAtomPayload = *((SstscAtomPayload*) stscAtomPayloadData->getBytePtr());

			// Sample Table Packet Sizes
			OR<AtomInfo>	stszAtomInfo = stblAtomGroup->getAtomInfo(MAKE_OSTYPE('s', 't', 's', 'z'));
			if (!stszAtomInfo.hasReference()) continue;
			OI<CData>	stszAtomPayloadData = getAtomPayload(*stszAtomInfo, error);
			if (error.hasInstance()) continue;
			const	SstszAtomPayload&	stszAtomPayload = *((SstszAtomPayload*) stszAtomPayloadData->getBytePtr());

			// Sample Table Block offsets
			OR<AtomInfo>	stcoAtomInfo = stblAtomGroup->getAtomInfo(MAKE_OSTYPE('s', 't', 'c', 'o'));
			OI<CData>		stcoAtomPayloadData =
									stcoAtomInfo.hasReference() ? getAtomPayload(*stcoAtomInfo, error) : OI<CData>();
			OR<AtomInfo>	co64AtomInfo = stblAtomGroup->getAtomInfo(MAKE_OSTYPE('c', 'o', '6', '4'));
			OI<CData>		co64AtomPayloadData =
									co64AtomInfo.hasReference() ? getAtomPayload(*co64AtomInfo, error) : OI<CData>();
			if (error.hasInstance()) continue;
			SstcoAtomPayload*	stcoAtomPayload =
										stcoAtomPayloadData.hasInstance() ?
												(SstcoAtomPayload*) stcoAtomPayloadData->getBytePtr() : nil;
			Sco64AtomPayload*	co64AtomPayload =
										co64AtomPayloadData.hasInstance() ?
												(Sco64AtomPayload*) co64AtomPayloadData->getBytePtr() : nil;

			if (hdlrAtomPayload.getSubType() == MAKE_OSTYPE('s', 'o', 'u', 'n')) {
				// Audio track
				if (stsdDescription.getType() == MAKE_OSTYPE('m', 'p', '4', 'a')) {
					// MPEG4 Audio
					mInternals->addMP4AAudioTrack(stsdDescription,
							CMPEG4MediaSourceInternals::composePacketLocations(sttsAtomPayload, stscAtomPayload,
									stszAtomPayload, stcoAtomPayload, co64AtomPayload));
				} else
					// Unsupported codec
					return OI<SError>(sUnsupportedCodecError);
			}
		}
	}

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CAudioTrack> CMPEG4MediaSource::getAudioTracks()
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mAudioTracks;
}
