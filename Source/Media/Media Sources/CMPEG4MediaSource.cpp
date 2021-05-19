//----------------------------------------------------------------------------------------------------------------------
//	CMPEG4MediaSource.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMPEG4MediaSource.h"

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

#pragma pack(push,1)

#if TARGET_OS_WINDOWS
	#pragma warning(disable:4200)
#endif

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

struct SmdhdAtomPayload {
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
			SmdhdAtomPayload(const CData& data) : mData(data) {}

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
							EndianU32_BtoN(payload._.mInfoV0.mDuration) :
							EndianU32_BtoN(payload._.mInfoV1.mDuration);
				}

	// Properties (in storage endian)
	private:
		const	CData&	mData;
};

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

struct SesdsAtomPayload {
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
		UInt8	mVersion;							// 0
		UInt8	mFlags[3];
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

// MP4A Audio Format (Sample Description detail)
struct SMP4AAudioFormat {
			// Methods
	UInt16	getBits() const { return EndianU16_BtoN(mBits); }
	UInt16	getChannels() const { return EndianU16_BtoN(mChannels); }

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
	UInt16	getWidth() const { return EndianU16_BtoN(mWidth); }
	UInt16	getHeight() const { return EndianU16_BtoN(mHeight); }

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
struct SstsdDescription {
			// Methods
			OSType				getType() const { return EndianU32_BtoN(mType); }
			SInt64				getFormatOffset() const { return 16; }
	const	SMP4AAudioFormat&	getMP4AAudioFormat() const { return mFormat.mMP4AAudioFormat; }
	const	SH264VideoFormat&	getH264VideoFormat() const { return mFormat.mH264VideoFormat; }

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
struct SstsdAtomPayload {
								// Methods
	const	SstsdDescription&	getFirstDescription() const { return *((SstsdDescription*) mDescriptions); }

	// Properties (in storage endian)
	private:
		UInt8	mVersion;				// 0
		UInt8	mFlags[3];
		UInt32	mDescriptionCount;
		UInt8	mDescriptions[];
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

// Sample Table Time-to-Sample Atom Payload
struct SsttsAtomPayload {
	// Structs
	struct Chunk {
				// Methods
		UInt32	getPacketCount() const { return EndianU32_BtoN(mPacketCount); }
		UInt32	getPacketDuration() const { return EndianU32_BtoN(mPacketDuration); }

		// Properties (in storage endian)
		private:
			UInt32	mPacketCount;
			UInt32	mPacketDuration;
	};

					// Methods
			UInt32	getChunkCount() const { return EndianU32_BtoN(mChunkCount); }
	const	Chunk&	getChunk(UInt32 index) const { return mChunks[index]; }

	// Properties (in storage endian)
	private:
		UInt8	mVersion;
		UInt8	mFlags[3];
		UInt32	mChunkCount;
		Chunk	mChunks[];
};

// Sample Table Sync Sample Atom Payload
struct SstssAtomPayload {
	//

	// Properties
//	private:
		UInt8	mVersion;
		UInt8	mFlags[3];
		UInt32	mKeyframesCount;
		UInt32	mKeyFrameIndexes[];
};

// Sample Table Sample-to-Chunk Atom Payload
struct SstscAtomPayload {
	// Structs
	struct PacketGroupInfo {
		// Methods
		UInt32	getChunkStartIndex() const { return EndianU32_BtoN(mChunkStartIndex); }
		UInt32	getPacketCount() const { return EndianU32_BtoN(mPacketCount); }

		// Properties (in storage endian)
		private:
			UInt32	mChunkStartIndex;
			UInt32	mPacketCount;
			UInt32	mSampleDescriptionIndex;
	};

								// Methods
			UInt32				getPacketGroupInfoCount() const { return EndianU32_BtoN(mPacketGroupInfoCount); }
	const	PacketGroupInfo&	getPacketGroupInfo(UInt32 index) const { return mPacketGroupInfos[index]; }

	// Properties (in storage endian)
	private:
		UInt8			mVersion;
		UInt8			mFlags[3];
		UInt32			mPacketGroupInfoCount;
		PacketGroupInfo	mPacketGroupInfos[];
};

// Sample Table Sample siZe Atom Payload
struct SstszAtomPayload {
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
struct SstcoAtomPayload {
			// Methods
	UInt32	getPacketGroupOffsetCount() const { return EndianU32_BtoN(mPacketGroupOffsetCount); }
	UInt64	getPacketGroupOffset(UInt32 index) const { return EndianU32_BtoN(mPacketGroupOffsets[index]); }

	// Properties (in storage endian)
	private:
		UInt8	mVersion;
		UInt8	mFlags[3];

		// For each packet group specified in the stsc Atom, the file offset is specified
		UInt32	mPacketGroupOffsetCount;
		UInt32	mPacketGroupOffsets[];		// Offset to start of packet data from start of file
};

// sample table Chunk Offset 64 Atom Payload
struct Sco64AtomPayload {
			// Methods
	UInt32	getPacketGroupOffsetCount() const { return EndianU32_BtoN(mPacketGroupOffsetCount); }
	UInt64	getPacketGroupOffset(UInt32 index) const { return EndianU64_NtoB(mPacketGroupOffsets[index]); }

	// Properties (in storage endian)
	private:
		UInt8	mVersion;
		UInt8	mFlags[3];

		// For each packet group specified in the stsc Atom, the file offset is specified
		UInt32	mPacketGroupOffsetCount;
		UInt64	mPacketGroupOffsets[];		// Offset to start of packet data from start of file
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

				void								addMP4AAudioTrack(const SstsdDescription& stsdDescription,
															const CData& esdsAtomPayloadData,
															const TArray<CCodec::PacketAndLocation>&
																	packetAndLocations);
				void								addH264VideoTrack(const SstsdDescription& stsdDescription,
															const CData& configurationData,
															const SmdhdAtomPayload& mdhdAtomPayload,
															const TArray<CCodec::PacketAndLocation>&
																	packetAndLocations,
															const OI<CData>& stssAtomPayloadData);

		static	TArray<CCodec::PacketAndLocation>	composePacketAndLocations(const SsttsAtomPayload& sttsAtomPayload,
															const SstscAtomPayload& stscAtomPayload,
															const SstszAtomPayload& stszAtomPayload,
															SstcoAtomPayload* stcoAtomPayload,
															Sco64AtomPayload* co64AtomPayload);

		TNArray<CAudioTrack>	mAudioTracks;
		TNArray<CVideoTrack>	mVideoTracks;
};

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CMPEG4MediaSourceInternals::addMP4AAudioTrack(const SstsdDescription& stsdDescription,
		const CData& esdsAtomPayloadData, const TArray<CCodec::PacketAndLocation>& packetAndLocations)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	SMP4AAudioFormat&				audioFormat = stsdDescription.getMP4AAudioFormat();
	const	SesdsAtomPayload&				esdsAtomPayload = *((SesdsAtomPayload*) esdsAtomPayloadData.getBytePtr());
	const	SesdsDecoderConfigDescriptor&	esdsDecoderConfigDescriptor = esdsAtomPayload.getDecoderConfigDescriptor();
	const	SesdsDecoderSpecificDescriptor&	esdsDecoderSpecificDescriptor =
													esdsDecoderConfigDescriptor.getDecoderSpecificDescriptor();
			CData							startCodesData = esdsDecoderSpecificDescriptor.getStartCodes();
			UInt16							startCodes = EndianU16_BtoN(*((UInt16*) startCodesData.getBytePtr()));

	// Compose storage format
	OI<SAudioStorageFormat>	audioStorageFormat =
									CAACAudioCodec::composeStorageFormat(startCodes, audioFormat.getChannels());
	if (!audioStorageFormat.hasInstance()) return;

	// Add audio track
	mAudioTracks +=
			CAudioTrack(*audioStorageFormat,
					I<CCodec::DecodeInfo>(
							new CAACAudioCodec::DecodeInfo(packetAndLocations,
									CData((UInt8*) esdsAtomPayloadData.getBytePtr() + 4,
											esdsAtomPayloadData.getSize() - 4),
									startCodes)));
}

//#include "CLogServices.h"

//----------------------------------------------------------------------------------------------------------------------
void CMPEG4MediaSourceInternals::addH264VideoTrack(const SstsdDescription& stsdDescription,
		const CData& configurationData, const SmdhdAtomPayload& mdhdAtomPayload,
		const TArray<CCodec::PacketAndLocation>& packetAndLocations, const OI<CData>& stssAtomPayloadData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	SH264VideoFormat&	videoFormat = stsdDescription.getH264VideoFormat();
			UInt32				timeScale = mdhdAtomPayload.getTimeScale();
//			UInt64				duration = mdhdAtomPayload.getDuration();
//			Float32				framerate = (Float32) timeScale / 100.0;
// TODO
//	const	SstssAtomPayload&	stssAtomPayload = *((SstssAtomPayload*) stssAtomPayloadData->getBytePtr());
//UInt32	keyframesCount = EndianU32_BtoN(stssAtomPayload.mKeyframesCount);
//CLogServices::logMessage(CString("Keyframes count: ") + CString(keyframesCount));
//for (UInt32 i = 0; i < keyframesCount; i++)
//	CLogServices::logMessage(CString("    ") + CString(EndianU32_BtoN(stssAtomPayload.mKeyFrameIndexes[i])));

	// Compose storage format
	OI<SVideoStorageFormat>	videoStorageFormat =
									CH264VideoCodec::composeStorageFormat(
											S2DSizeU16(videoFormat.getWidth(), videoFormat.getHeight()));
	if (!videoStorageFormat.hasInstance()) return;

	// Add video track
	mVideoTracks +=
			CVideoTrack(*videoStorageFormat,
					I<CCodec::DecodeInfo>(
							new CH264VideoCodec::DecodeInfo(configurationData, timeScale, packetAndLocations)));
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TArray<CCodec::PacketAndLocation> CMPEG4MediaSourceInternals::composePacketAndLocations(
		const SsttsAtomPayload& sttsAtomPayload, const SstscAtomPayload& stscAtomPayload,
		const SstszAtomPayload& stszAtomPayload, SstcoAtomPayload* stcoAtomPayload, Sco64AtomPayload* co64AtomPayload)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNArray<CCodec::PacketAndLocation>	packetAndLocations;

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
		const	SsttsAtomPayload::Chunk&	sttsChunk = sttsAtomPayload.getChunk(sttsChunkIndex);
				UInt32						sttsChunkPacketCount = sttsChunk.getPacketCount();
				UInt32						sttsChunkPacketDuration = sttsChunk.getPacketDuration();

		// Iterate packets
		for (UInt32 packetIndex = 0; packetIndex < sttsChunkPacketCount; packetIndex++, stszPacketByteCountIndex++) {
			// Get info
			UInt32	packetByteCount = stszAtomPayload.getPacketByteCount(stszPacketByteCountIndex);

			// Add Packet Location Info
			packetAndLocations +=
					CCodec::PacketAndLocation(CCodec::Packet(sttsChunkPacketDuration, packetByteCount),
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

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMPEG4MediaSource

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CMPEG4MediaSource::CMPEG4MediaSource(const I<CDataSource>& dataSource) :
		CAtomMediaSource(CByteParceller(dataSource, true))
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
			// Track
			OI<AtomGroup>	trakAtomGroup = getAtomGroup(*moovIterator, error);
			if (error.hasInstance()) continue;

			// Media
			OI<AtomGroup>	mdiaAtomGroup =
									getAtomGroup(trakAtomGroup->getAtomInfo(MAKE_OSTYPE('m', 'd', 'i', 'a')), error);
			if (error.hasInstance()) continue;

			// Handler
			OI<CData>	hdlrAtomPayloadData =
								getAtomPayload(mdiaAtomGroup->getAtomInfo(MAKE_OSTYPE('h', 'd', 'l', 'r')), error);
			if (error.hasInstance()) continue;
			const	ShdlrAtomPayload&	hdlrAtomPayload = *((ShdlrAtomPayload*) hdlrAtomPayloadData->getBytePtr());

			// Media Information
			OI<AtomGroup>	minfAtomGroup =
									getAtomGroup(mdiaAtomGroup->getAtomInfo(MAKE_OSTYPE('m', 'i', 'n', 'f')), error);
			if (error.hasInstance()) continue;

			// Sample Table
			OI<AtomGroup>	stblAtomGroup =
									getAtomGroup(minfAtomGroup->getAtomInfo(MAKE_OSTYPE('s', 't', 'b', 'l')), error);
			if (error.hasInstance()) continue;

			// Sample Table Sample Description
			OR<AtomInfo>	stsdAtomInfo = stblAtomGroup->getAtomInfo(MAKE_OSTYPE('s', 't', 's', 'd'));
			if (!stsdAtomInfo.hasReference()) continue;
			OI<CData>	stsdAtomPayloadData = getAtomPayload(*stsdAtomInfo, error);
			if (error.hasInstance()) continue;
			const	SstsdAtomPayload&	stsdAtomPayload = *((SstsdAtomPayload*) stsdAtomPayloadData->getBytePtr());
			const	SstsdDescription&	stsdDescription = stsdAtomPayload.getFirstDescription();

			// Sample Table Time-to-Sample
			OI<CData>	sttsAtomPayloadData =
								getAtomPayload(stblAtomGroup->getAtomInfo(MAKE_OSTYPE('s', 't', 't', 's')), error);
			if (error.hasInstance()) continue;
			const	SsttsAtomPayload&	sttsAtomPayload = *((SsttsAtomPayload*) sttsAtomPayloadData->getBytePtr());

			// Sample Table Sample Blocks
			OI<CData>	stscAtomPayloadData =
								getAtomPayload(stblAtomGroup->getAtomInfo(MAKE_OSTYPE('s', 't', 's', 'c')), error);
			if (error.hasInstance()) continue;
			const	SstscAtomPayload&	stscAtomPayload = *((SstscAtomPayload*) stscAtomPayloadData->getBytePtr());

			// Sample Table Packet Sizes
			OI<CData>	stszAtomPayloadData =
								getAtomPayload(stblAtomGroup->getAtomInfo(MAKE_OSTYPE('s', 't', 's', 'z')), error);
			if (error.hasInstance()) continue;
			const	SstszAtomPayload&	stszAtomPayload = *((SstszAtomPayload*) stszAtomPayloadData->getBytePtr());

			// Sample Table Block offsets
			OI<CData>	stcoAtomPayloadData =
								getAtomPayload(stblAtomGroup->getAtomInfo(MAKE_OSTYPE('s', 't', 'c', 'o')), error);
			OI<CData>	co64AtomPayloadData =
								getAtomPayload(stblAtomGroup->getAtomInfo(MAKE_OSTYPE('c', 'o', '6', '4')), error);
			if (!stcoAtomPayloadData.hasInstance() && !co64AtomPayloadData.hasInstance()) continue;
			SstcoAtomPayload*	stcoAtomPayload =
										stcoAtomPayloadData.hasInstance() ?
												(SstcoAtomPayload*) stcoAtomPayloadData->getBytePtr() : nil;
			Sco64AtomPayload*	co64AtomPayload =
										co64AtomPayloadData.hasInstance() ?
												(Sco64AtomPayload*) co64AtomPayloadData->getBytePtr() : nil;

			// Check track type
			if (hdlrAtomPayload.getSubType() == MAKE_OSTYPE('s', 'o', 'u', 'n')) {
				// Audio track
				if (stsdDescription.getType() == MAKE_OSTYPE('m', 'p', '4', 'a')) {
					// MPEG4 Audio
					OI<CData>	esdsAtomPayloadData =
										getAtomPayload(*stsdAtomInfo,
												sizeof(SstsdAtomPayload) + stsdDescription.getFormatOffset() +
														sizeof(SMP4AAudioFormat),
												error);
  					if (error.hasInstance()) continue;

					mInternals->addMP4AAudioTrack(stsdDescription, *esdsAtomPayloadData,
							CMPEG4MediaSourceInternals::composePacketAndLocations(sttsAtomPayload, stscAtomPayload,
									stszAtomPayload, stcoAtomPayload, co64AtomPayload));
				} else
					// Unsupported codec
					return OI<SError>(sUnsupportedCodecError);
			} else if (hdlrAtomPayload.getSubType() == MAKE_OSTYPE('v', 'i', 'd', 'e')) {
				// Video track
				OI<CData>	mdhdAtomPayloadData =
									getAtomPayload(mdiaAtomGroup->getAtomInfo(MAKE_OSTYPE('m', 'd', 'h', 'd')), error);
				if (error.hasInstance()) continue;

				OI<CData>	stssAtomPayloadData =
									getAtomPayload(stblAtomGroup->getAtomInfo(MAKE_OSTYPE('s', 't', 's', 's')), error);
				if (error.hasInstance()) continue;

				// Check type
				if (stsdDescription.getType() == MAKE_OSTYPE('a', 'v', 'c', '1')) {
					// h.264 Video
					OI<CData>	h264ConfigurationAtomPayloadData =
										getAtomPayload(*stsdAtomInfo,
												sizeof(SstsdAtomPayload) + stsdDescription.getFormatOffset() +
														sizeof(SH264VideoFormat),
												error);
					if (error.hasInstance()) continue;

					mInternals->addH264VideoTrack(stsdDescription, *h264ConfigurationAtomPayloadData,
							SmdhdAtomPayload(*mdhdAtomPayloadData),
							CMPEG4MediaSourceInternals::composePacketAndLocations(sttsAtomPayload, stscAtomPayload,
									stszAtomPayload, stcoAtomPayload, co64AtomPayload),
							stssAtomPayloadData);
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

//----------------------------------------------------------------------------------------------------------------------
TArray<CVideoTrack> CMPEG4MediaSource::getVideoTracks()
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mVideoTracks;
}
