//----------------------------------------------------------------------------------------------------------------------
//	CQuickTimeMediaFile.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CQuickTimeMediaFile.h"

#include "CAACAudioCodec.h"
#include "CH264VideoCodec.h"
#include "CPCMAudioCodec.h"

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

//struct SQTftypAtom {
//	OSType	mMajorBrand;
//	UInt32	mMajorBrandVersion;
//	OSType	mCompatibleBrands[];
//};

struct SQThdlrAtomPayload {
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

//struct SQTmvhdAtomV0 {
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
//struct SQTmvhdAtomV1 {
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

//struct SQTtkhdAtomPayload {
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

struct SQTmdhdAtomPayload {
	// Structs
	struct Payload {
		UInt8	mVersion;
		UInt8	mFlags[3];

		UInt32	mCreationTime;
		UInt32	mModificationTime;

		UInt32	mTimeScale;
		UInt32	mDuration;

		UInt16	mLanguage;
		UInt16	mQuality;
	};

			// Lifecycle methods
			SQTmdhdAtomPayload(const CData& data) : mData(data) {}

			// Instance Methods
	UInt32	getTimeScale() const
				{
					// Setup
					const	Payload&	payload = *((Payload*) mData.getBytePtr());

					return EndianU32_BtoN(payload.mTimeScale);
				}
	UInt32	getDuration() const
				{
					// Setup
					const	Payload&	payload = *((Payload*) mData.getBytePtr());

					return EndianU32_BtoN(payload.mDuration);
				}

	// Properties (in storage endian)
	private:
		const	CData&	mData;
};

//struct SQTsmhdAtom {
//	UInt8	mVersion;								// 0
//	UInt8	mFlags[3];
//
//	UInt16	mAudioBalance;
//	UInt8	mReserved[2];
//};
//
//struct SQTdrefAtom {
//	UInt8	mVersion;								// 0
//	UInt8	mFlags[3];
//
//	UInt32	mReferenceCount;
//};
//
//struct SQTurlAtom {
//	UInt8	mVersion;								// 0
//	UInt8	mFlags[3];
//};

// SQTAudioSampleDescription
struct SQTAudioSampleDescription {
						// Methods
	UInt16				getBits() const
							{
								// Check version
								switch (EndianU16_BtoN(mVersion)) {
									case 0:		return EndianU16_BtoN(_.mV0.mBits);
									case 1:		return EndianU16_BtoN(_.mV1.mBits);
									case 2:		return (UInt16) EndianU32_BtoN(_.mV2.mConstBitsPerChannel);
									default:	return 0;
								}
							}
	Float32				getSampleRate() const
							{
								// Check version
								switch (EndianU16_BtoN(mVersion)) {
									case 0:		return ((Float32) EndianU32_BtoN(_.mV0.mSampleRate)) / 65536.0f;
									case 1:		return ((Float32) EndianU32_BtoN(_.mV1.mSampleRate)) / 65536.0f;
									case 2:		return (Float32) EndianF64_BtoN(_.mV2.mSampleRate);
									default:	return 0;
								}
							}
	UInt16				getChannels() const
							{
								// Check version
								switch (EndianU16_BtoN(mVersion)) {
									case 0:		return EndianU16_BtoN(_.mV0.mChannels);
									case 1:		return EndianU16_BtoN(_.mV1.mChannels);
									case 2:		return (UInt16) EndianU32_BtoN(_.mV2.mChannels);
									default:	return 0;
								}
							}
	CData::ByteIndex	getCodecConfigurationDataByteOffset() const
							{
								// Check version
								switch (EndianU16_BtoN(mVersion)) {
									case 0:		return sizeof(mVersion) + sizeof(_.mV0);
									case 1:		return sizeof(mVersion) + sizeof(_.mV1);
									case 2:		return sizeof(mVersion) + sizeof(_.mV2);
									default:	return 0;
								}
							}

	// Properties (in storage endian)
	private:
		UInt16	mVersion;			// 0, 1, or 2
		union {
			struct V0 {	// Uncompressed
				UInt16	mRevisionLevel;		// 0
				OSType	mEncodingVendor;	// 0
				UInt16	mChannels;			// 1 or 2
				UInt16	mBits;				// 8 or 16
				UInt16	mCompressionID;		// 0
				UInt16	mPacketSize;		// 0
				UInt32	mSampleRate;		// Fixed point 16.16
			} mV0;

			struct V1 {	// Compressed
				UInt16	mRevisionLevel;		// 0
				OSType	mEncodingVendor;	// 0
				UInt16	mChannels;			// 1 or 2
				UInt16	mBits;				// 8 or 16
				UInt16	mCompressionID;		// may be -2
				UInt16	mPacketSize;		// 0
				UInt32	mSampleRate;		// Fixed point 16.16

				UInt32	mSamplesPerPacket;
				UInt32	mBytesPerPacket;
				UInt32	mBytesPerFrame;
				UInt32	mBytesPerSample;

				UInt8	mAdditionalAtoms[];
			} mV1;

			struct V2 {	// Compressed
				UInt16			mRevisionLevel;			// 0
				OSType			mEncodingVendor;		// 0
				UInt16			mAlways3;				// 3
				UInt16			mAlways16;				// 16
				UInt16			mAlwaysFFFE;			// 0xFFFE
				UInt16			mAlways0;				// 0
				UInt32			mAlways00010000;		// 00010000
				UInt32			mSizeOfStructOnly;		// Offset to Atoms
				StoredFloat64	mSampleRate;
				UInt32			mChannels;
				UInt32			mAlways7F000000;		// 0x7F000000
				UInt32			mConstBitsPerChannel;	// Set for uncompressed, 0 otherwise
				UInt32			mFormatSpecificFlags;	// Set for uncompressed (See CoreAudio flags)
				UInt32			mConstBytesPerPacket;	// Set for CBR, 0 otherwise
				UInt32			mConstLPCMFramesPerPacket;	// Set for uncompressed, 0 otherwise

				UInt8			mAdditionalAtoms[];
			} mV2;
		} _;
};

// SQTVideoSampleDescription
struct SQTVideoSampleDescription {
			// Methods
	UInt16	getWidth() const
				{ return EndianU16_BtoN(mWidth); }
	UInt16	getHeight() const
				{ return EndianU16_BtoN(mHeight); }

	// Properties (in storage endian)
	private:
		UInt16	mVersion;			// 0 unless the vendor has changed its data format
		UInt16	mRevisionLevel;		// 0
		OSType	mEncodingVendor;
		UInt32	mTemporalQuality;	// 0 to 1023
		UInt32	mSpatialQuality;	// 0 to 1024
		UInt16	mWidth;
		UInt16	mHeight;
		UInt32	mHorizontalDPI;
		UInt32	mVerticalDPI;
		UInt32	mDataSize;			// 0
		UInt16	mFrameCount;		// Usually 1
		UInt8	mEncoderNameLength;
		UInt8	mEncoderName[31];
		UInt16	mPixelDepth;
		SInt16	mColorTableID;
		UInt8	mColorTable[];
};

// Sample Table Sample Description (general)
struct SQTstsdDescription {
			// Methods
			OSType						getType() const
											{ return EndianU32_BtoN(mType); }
	const	SQTAudioSampleDescription&	getAudioSampleDescription() const
											{ return mSampleDescription.mAudioSampleDescription; }
	const	SQTVideoSampleDescription&	getVideoSampleDescription() const
											{ return mSampleDescription.mVideoSampleDescription; }

	// Properties (in storage endian)
	public:
		static	CData::ByteCount	mByteCountWithoutSampleDescriptions;

	private:
		UInt32				mLength;
		OSType				mType;
		UInt8				mReserved[6];
		UInt16				mDataRefIndex;
		union {
			SQTAudioSampleDescription	mAudioSampleDescription;
			SQTVideoSampleDescription	mVideoSampleDescription;
		}					mSampleDescription;
};
CData::ByteCount	SQTstsdDescription::mByteCountWithoutSampleDescriptions = 16;


// Sample Table Sample Description Atom Payload
struct SQTstsdAtomPayload {
								// Methods
	const	SQTstsdDescription&	getFirstDescription() const
									{ return *((SQTstsdDescription*) mDescriptions); }

	// Properties (in storage endian)
	private:
		UInt8	mVersion;				// 0
		UInt8	mFlags[3];
		UInt32	mDescriptionCount;
		UInt8	mDescriptions[];
};

//struct SQTALACSpecificConfig {
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
//struct SQTALACChannelLayoutInfo {
//	UInt32	mChannelLayoutInfoSize;
//	UInt32	mChannelLayoutInfoID;
//	UInt32	mVersionFlags;
//	UInt32	mChannelLayoutTag;
//	UInt32	mReserved1;
//	UInt32	mReserved2;
//};
//
//struct SQTalacAtom {
//	UInt32				mByteCount;
//	OSType				mType;
//	UInt32				mVersion;
//
//	SALACSpecificConfig	mALACSpecificConfig;
//};
//
//struct SQTalacAtomWithChannelLayout {
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
struct SQTsttsAtomPayload {
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
struct SQTstssAtomPayload {
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
struct SQTstscAtomPayload {
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
struct SQTstszAtomPayload {
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
struct SQTstcoAtomPayload {
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

//// sample table Chunk Offset 64 Atom Payload
//struct SQTco64AtomPayload {
//			// Methods
//	UInt32	getPacketGroupOffsetCount() const
//				{ return EndianU32_BtoN(mPacketGroupOffsetCount); }
//	UInt64	getPacketGroupOffset(UInt32 index) const
//				{ return EndianU64_NtoB(mPacketGroupOffsets[index]); }
//
//	// Properties (in storage endian)
//	private:
//		UInt8	mVersion;
//		UInt8	mFlags[3];
//
//		// For each packet group specified in the stsc Atom, the file offset is specified
//		UInt32	mPacketGroupOffsetCount;
//		UInt64	mPacketGroupOffsets[];		// Offset to start of packet data from start of file
//};

//struct SQTmetaAtom {
//	UInt8	mVersion;
//	UInt8	mFlags[3];
//};
//
//struct SQTdataAtom {
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
// MARK: - Local proc declarations

static	CMediaTrackInfos::AudioTrackInfo	sComposePCMAudioTrackInfo(const CQuickTimeMediaFile& quickTimeMediaFile,
													const I<CRandomAccessDataSource>& randomAccessDataSource,
													UInt32 options, bool isFloat, UInt8 bits,
													CPCMAudioCodec::Format format, UniversalTimeInterval duration,
													const CQuickTimeMediaFile::Internals& internals);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CQuickTimeMediaFile

// MARK: Internals

struct CQuickTimeMediaFile::Internals {

										Internals(const CAtomReader& atomReader, const CAtomReader::Atom& stsdAtom,
												const CAtomReader::ContainerAtom& stblContainerAtom,
												const SQTstsdDescription& stsdDescription,
												const SQTsttsAtomPayload& sttsAtomPayload,
												const SQTstscAtomPayload& stscAtomPayload,
												const SQTstszAtomPayload& stszAtomPayload,
												SQTstcoAtomPayload* stcoAtomPayload) :
											mAtomReader(atomReader), mSTSDAtom(stsdAtom),
													mSTBLContainerAtom(stblContainerAtom),
													mSTSDDescription(stsdDescription),
													mSTTSAtomPayload(sttsAtomPayload),
													mSTSCAtomPayload(stscAtomPayload),
													mSTSZAtomPayload(stszAtomPayload),
													mSTCOAtomPayload(stcoAtomPayload)
											{}

	const	SQTAudioSampleDescription&	getAudioSampleDescription() const
											{ return mSTSDDescription.getAudioSampleDescription(); }
	const	SQTVideoSampleDescription&	getVideoSampleDescription() const
											{ return mSTSDDescription.getVideoSampleDescription(); }

			TVResult<CData>				getAudioDecompressionData() const
											{
												// Setup
												CData::ByteIndex			codecConfigurationDataByteOffset =
																					sizeof(SQTstsdAtomPayload) +
																							SQTstsdDescription::
																									mByteCountWithoutSampleDescriptions +
																							getAudioSampleDescription()
																									.getCodecConfigurationDataByteOffset();
												TVResult<CAtomReader::Atom>	decompressionParamAtom =
																					mAtomReader.readAtom(mSTSDAtom,
																							codecConfigurationDataByteOffset);
												ReturnValueIfResultError(decompressionParamAtom,
														TVResult<CData>(decompressionParamAtom.getError()));

												return mAtomReader.readAtomPayload(*decompressionParamAtom);
											}
			TVResult<CData>				getVideoDecompressionData() const
											{
												// Setup
												CData::ByteIndex	codecConfigurationDataByteOffset =
																			sizeof(SQTstsdAtomPayload) +
																					SQTstsdDescription::
																							mByteCountWithoutSampleDescriptions +
																					sizeof(SQTVideoSampleDescription);

												// Read atom payload
												TVResult<CData>	payload = mAtomReader.readAtomPayload(mSTSDAtom);
												ReturnResultIfResultError(payload);

												return TVResult<CData>(
														payload->subData(codecConfigurationDataByteOffset));
											}

			UInt32						getPacketGroupOffsetCount() const
											{
//												return (mSTCOAtomPayload != nil) ?
//														mSTCOAtomPayload->getPacketGroupOffsetCount() :
//														mCO64AtomPayload->getPacketGroupOffsetCount();
												return mSTCOAtomPayload->getPacketGroupOffsetCount();
											}
			UInt64						getPacketGroupOffset(UInt32 stcoBlockOffsetIndex) const
											{
//												return (mSTCOAtomPayload != nil) ?
//														mSTCOAtomPayload->getPacketGroupOffset(stcoBlockOffsetIndex) :
//														mCO64AtomPayload->getPacketGroupOffset(stcoBlockOffsetIndex);
												return mSTCOAtomPayload->getPacketGroupOffset(stcoBlockOffsetIndex);
											}

	const	CAtomReader&				mAtomReader;
	const	CAtomReader::Atom&			mSTSDAtom;
	const	CAtomReader::ContainerAtom&	mSTBLContainerAtom;
	const	SQTstsdDescription&			mSTSDDescription;
	const	SQTsttsAtomPayload&			mSTTSAtomPayload;
	const	SQTstscAtomPayload&			mSTSCAtomPayload;
	const	SQTstszAtomPayload&			mSTSZAtomPayload;
			SQTstcoAtomPayload* 		mSTCOAtomPayload;
};

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
I<SMediaSource::ImportResult> CQuickTimeMediaFile::import(const I<CRandomAccessDataSource>& randomAccessDataSource,
		const OI<CAppleResourceManager>& appleResourceManager, UInt32 options)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CAtomReader	atomReader(randomAccessDataSource);
	OV<SError>	error;

	// Find core atoms
	OV<CAtomReader::Atom>	moovAtom;
	OV<CAtomReader::Atom>	mdatAtom;

	while (true) {
		// Read next atom
		TVResult<CAtomReader::Atom>	atom = atomReader.readAtom();
		if (atom.hasError())
			break;

		// Check atom type
		if (atom->mType == MAKE_OSTYPE('m', 'o', 'o', 'v'))
			// moov
			moovAtom = OV<CAtomReader::Atom>(*atom);
		else if (atom->mType == MAKE_OSTYPE('m', 'd', 'a', 't'))
			// mdat
			mdatAtom = OV<CAtomReader::Atom>(*atom);

		// Go to next atom
		error = atomReader.seekToNextAtom(*atom);
		if (error.hasValue())
			break;
	}

	if (!moovAtom.hasValue() || !mdatAtom.hasValue()) {
		// Didn't find core atoms
		if (error.hasValue())
			return I<SMediaSource::ImportResult>(new SMediaSource::ImportResult(*error));
		else
			return I<SMediaSource::ImportResult>(new SMediaSource::ImportResult());
	}

	TVResult<CAtomReader::ContainerAtom>	moovContainerAtom = atomReader.readContainerAtom(*moovAtom);
	ReturnValueIfResultError(moovContainerAtom,
			I<SMediaSource::ImportResult>(new SMediaSource::ImportResult(moovContainerAtom.getError())));

	// Iterate moov atom
	CMediaTrackInfos	mediaTrackInfos;
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

			SQTmdhdAtomPayload		mdhdAtomPayload(*mdhdAtomPayloadData);
			UInt32					timeScale = mdhdAtomPayload.getTimeScale();
			UniversalTimeInterval	duration =
											(UniversalTimeInterval) mdhdAtomPayload.getDuration() /
													(UniversalTimeInterval) timeScale;

			// Handler
			TVResult<CData>	hdlrAtomPayloadData =
									atomReader.readAtomPayload(
											mdiaContainerAtom->getAtom(MAKE_OSTYPE('h', 'd', 'l', 'r')));
			if (hdlrAtomPayloadData.hasError()) continue;
			const	SQThdlrAtomPayload&	hdlrAtomPayload =
												*((SQThdlrAtomPayload*) hdlrAtomPayloadData->getBytePtr());

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
			const	SQTstsdAtomPayload&	stsdAtomPayload =
												*((SQTstsdAtomPayload*) stsdAtomPayloadData->getBytePtr());
			const	SQTstsdDescription&	stsdDescription = stsdAtomPayload.getFirstDescription();

			// Sample Table Time-to-Sample
			TVResult<CData>	sttsAtomPayloadData =
									atomReader.readAtomPayload(
											stblContainerAtom->getAtom(MAKE_OSTYPE('s', 't', 't', 's')));
			if (sttsAtomPayloadData.hasError()) continue;
			const	SQTsttsAtomPayload&	sttsAtomPayload =
												*((SQTsttsAtomPayload*) sttsAtomPayloadData->getBytePtr());

			// Sample Table Sample Blocks
			TVResult<CData>	stscAtomPayloadData =
									atomReader.readAtomPayload(
											stblContainerAtom->getAtom(MAKE_OSTYPE('s', 't', 's', 'c')));
			if (stscAtomPayloadData.hasError()) continue;
			const	SQTstscAtomPayload&	stscAtomPayload =
												*((SQTstscAtomPayload*) stscAtomPayloadData->getBytePtr());

			// Sample Table Packet Sizes
			TVResult<CData>	stszAtomPayloadData =
									atomReader.readAtomPayload(
											stblContainerAtom->getAtom(MAKE_OSTYPE('s', 't', 's', 'z')));
			if (stszAtomPayloadData.hasError()) continue;
			const	SQTstszAtomPayload&	stszAtomPayload =
												*((SQTstszAtomPayload*) stszAtomPayloadData->getBytePtr());

			// Sample Table Block offsets
			TVResult<CData>	stcoAtomPayloadData =
									atomReader.readAtomPayload(
											stblContainerAtom->getAtom(MAKE_OSTYPE('s', 't', 'c', 'o')));
			TVResult<CData>	co64AtomPayloadData =
									atomReader.readAtomPayload(
											stblContainerAtom->getAtom(MAKE_OSTYPE('c', 'o', '6', '4')));
			if (!stcoAtomPayloadData.hasValue() && !co64AtomPayloadData.hasValue()) continue;
			SQTstcoAtomPayload*	stcoAtomPayload =
										stcoAtomPayloadData.hasValue() ?
												(SQTstcoAtomPayload*) stcoAtomPayloadData->getBytePtr() : nil;
//			Sco64AtomPayload*	co64AtomPayload =
//										co64AtomPayloadData.hasValue() ?
//												(Sco64AtomPayload*) co64AtomPayloadData->getBytePtr() : nil;

			// Internals
			Internals	internals(atomReader, *stsdAtom, *stblContainerAtom, stsdDescription, sttsAtomPayload,
								stscAtomPayload, stszAtomPayload, stcoAtomPayload);

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
					return I<SMediaSource::ImportResult>(new SMediaSource::ImportResult(audioTrackInfo.getError()));
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
					return I<SMediaSource::ImportResult>(new SMediaSource::ImportResult(videoTrackInfo.getError()));
			}
		}
	}

	return I<SMediaSource::ImportResult>(new SMediaSource::ImportResult(mediaTrackInfos));
}

//----------------------------------------------------------------------------------------------------------------------
TArray<SMediaPacketAndLocation> CQuickTimeMediaFile::composePacketAndLocations(const Internals& internals,
		const OV<UInt32>& framesPerPacket, const OV<UInt32>& bytesPerPacket) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	SQTsttsAtomPayload&	sttsAtomPayload = internals.mSTTSAtomPayload;
	const	SQTstscAtomPayload&	stscAtomPayload = internals.mSTSCAtomPayload;
	const	SQTstszAtomPayload&	stszAtomPayload = internals.mSTSZAtomPayload;

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
	UInt32								currentFrameIndex = 0;
	TNArray<SMediaPacketAndLocation>	packetAndLocations;
	for (UInt32 sttsChunkIndex = 0; sttsChunkIndex < sttsChunkCount; sttsChunkIndex++) {
		// Get packet info
		const	SQTsttsAtomPayload::Chunk&	sttsChunk = sttsAtomPayload.getChunk(sttsChunkIndex);
				UInt32						sttsChunkPacketCount = sttsChunk.getPacketCount();
				UInt32						sttsChunkPacketDuration = sttsChunk.getPacketDuration();

		// Iterate packets
		for (UInt32 packetIndex = 0; packetIndex < sttsChunkPacketCount; packetIndex++, stszPacketByteCountIndex++) {
			// Get info
			UInt32	packetByteCount = stszAtomPayload.getPacketByteCount(stszPacketByteCountIndex);

			// Check if packet details are specified
			if (framesPerPacket.hasValue()) {
				// Check if have completed this packet
				if (++currentFrameIndex == *framesPerPacket) {
					// Add Packet Location Info
					packetAndLocations +=
							SMediaPacketAndLocation(SMediaPacket(*framesPerPacket, *bytesPerPacket), currentByteOffset);
					currentByteOffset += *bytesPerPacket;
					currentFrameIndex = 0;
				}
			} else {
				// Add Packet Location Info
				packetAndLocations +=
						SMediaPacketAndLocation(SMediaPacket(sttsChunkPacketDuration, packetByteCount),
								currentByteOffset);
			}

			// Update
			if (++currentBlockPacketIndex < stscPacketGroupInfoPacketCount) {
				// Still more to go in this block
				if (!bytesPerPacket.hasValue())
					// Update byte offset
					currentByteOffset += packetByteCount;
			} else {
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
TVResult<CMediaTrackInfos::AudioTrackInfo> CQuickTimeMediaFile::composeAudioTrackInfo(
		const I<CRandomAccessDataSource>& randomAccessDataSource, UInt32 options, OSType type,
		UniversalTimeInterval duration, const Internals& internals)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	SQTAudioSampleDescription&	audioSampleDescription = internals.getAudioSampleDescription();

	// Check type
	switch (type) {
		case MAKE_OSTYPE('N', 'O', 'N', 'E'):
		case MAKE_OSTYPE('t', 'w', 'o', 's'):
			// None / Integer, Big Endian
			if (audioSampleDescription.getBits() <= 32) {
				// Good to go
				UInt8	bits = (UInt8) audioSampleDescription.getBits();

				return TVResult<CMediaTrackInfos::AudioTrackInfo>(
						sComposePCMAudioTrackInfo(*this, randomAccessDataSource, options, false, bits,
								(bits > 8) ? CPCMAudioCodec::kFormatBigEndian : CPCMAudioCodec::kFormat8BitSigned,
								duration, internals));
			} else
				// Don't know what to do with sample size greater than 32 bits
				return TVResult<CMediaTrackInfos::AudioTrackInfo>(
						CCodec::unsupportedConfigurationError(CString(type, true)));

		case MAKE_OSTYPE('s', 'o', 'w', 't'):
			// None / Integer, Little Endian
			if (audioSampleDescription.getBits() <= 32) {
				// Good to go
				UInt8	bits = (UInt8) audioSampleDescription.getBits();

				return TVResult<CMediaTrackInfos::AudioTrackInfo>(
						sComposePCMAudioTrackInfo(*this, randomAccessDataSource, options, false, bits,
								(bits > 8) ? CPCMAudioCodec::kFormatLittleEndian : CPCMAudioCodec::kFormat8BitSigned,
								duration, internals));
			} else
				// Don't know what to do with sample size greater than 32 bits
				return TVResult<CMediaTrackInfos::AudioTrackInfo>(
						CCodec::unsupportedConfigurationError(CString(type, true)));

		case MAKE_OSTYPE('r', 'a', 'w', ' '):
			// None / Integer
			if (audioSampleDescription.getBits() <= 32) {
				// Good to go
				UInt8	bits = (UInt8) audioSampleDescription.getBits();

				return TVResult<CMediaTrackInfos::AudioTrackInfo>(
						sComposePCMAudioTrackInfo(*this, randomAccessDataSource, options, false, bits,
								(bits > 8) ? CPCMAudioCodec::kFormatBigEndian : CPCMAudioCodec::kFormat8BitUnsigned,
								duration, internals));
			} else
				// Don't know what to do with sample size greater than 32 bits
				return TVResult<CMediaTrackInfos::AudioTrackInfo>(
						CCodec::unsupportedConfigurationError(CString(type, true)));

		case MAKE_OSTYPE('i', 'n', '2', '4'):
			// 24-bit Integer
			return TVResult<CMediaTrackInfos::AudioTrackInfo>(
					sComposePCMAudioTrackInfo(*this, randomAccessDataSource, options, false, 24,
							CPCMAudioCodec::kFormatBigEndian, duration, internals));

		case MAKE_OSTYPE('i', 'n', '3', '2'):
			// 32-bit Integer
			return TVResult<CMediaTrackInfos::AudioTrackInfo>(
					sComposePCMAudioTrackInfo(*this, randomAccessDataSource, options, false, 32,
							CPCMAudioCodec::kFormatBigEndian, duration, internals));

		case MAKE_OSTYPE('f', 'l', '3', '2'):
		case MAKE_OSTYPE('F', 'L', '3', '2'):
			// None / Floating Point
			return TVResult<CMediaTrackInfos::AudioTrackInfo>(
					sComposePCMAudioTrackInfo(*this, randomAccessDataSource, options, true, 32,
							CPCMAudioCodec::kFormatBigEndian, duration, internals));

//		case MAKE_OSTYPE('f', 'l', '6', '4'):

		case MAKE_OSTYPE('m', 'p', '4', 'a'): {
			// MPEG4 (AAC) Audio
			TVResult<CData>	decompressionData = internals.getAudioDecompressionData();
			ReturnValueIfResultError(decompressionData,
					TVResult<CMediaTrackInfos::AudioTrackInfo>(
							CCodec::unsupportedConfigurationError(CString(type, true))));

			CAtomReader	decompressionAtomReader(I<CRandomAccessDataSource>(new CDataDataSource(*decompressionData)));

			TVResult<CAtomReader::ContainerAtom>	decompressionParamContainerAtom =
															decompressionAtomReader.readContainerAtom();
			ReturnValueIfResultError(decompressionParamContainerAtom,
					TVResult<CMediaTrackInfos::AudioTrackInfo>(
							CCodec::unsupportedConfigurationError(CString(type, true))));

			OR<CAtomReader::Atom>	esdsAtom = decompressionParamContainerAtom->getAtom(
															MAKE_OSTYPE('e', 's', 'd', 's'));
			if (!esdsAtom.hasReference())
				return TVResult<CMediaTrackInfos::AudioTrackInfo>(
						CCodec::unsupportedConfigurationError(CString(type, true)));

			TVResult<CData>	esdsAtomPayload = decompressionAtomReader.readAtomPayload(*esdsAtom);
			ReturnValueIfResultError(esdsAtomPayload,
					TVResult<CMediaTrackInfos::AudioTrackInfo>(
							CCodec::unsupportedConfigurationError(CString(type, true))));

			// Compose storage format
			OV<CAACAudioCodec::Info>	info =
												CAACAudioCodec::composeInfo(*esdsAtomPayload,
														audioSampleDescription.getChannels());
			if (!info.hasValue())
				return TVResult<CMediaTrackInfos::AudioTrackInfo>(
						CCodec::unsupportedConfigurationError(CString(type, true)));

			SAudioStorageFormat	audioStorageFormat = CAACAudioCodec::composeAudioStorageFormat(*info);

			// Compose info
			TArray<SMediaPacketAndLocation>	mediaPacketAndLocations = composePacketAndLocations(internals);
			UInt64							byteCount =
													SMediaPacketAndLocation::getTotalByteCount(mediaPacketAndLocations);

			// Add audio track
			CAudioTrack	audioTrack(CMediaTrack::composeInfo(duration, byteCount), audioStorageFormat);
			if (options & SMediaSource::kOptionsCreateDecoders)
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
TVResult<CMediaTrackInfos::VideoTrackInfo> CQuickTimeMediaFile::composeVideoTrackInfo(
		const I<CRandomAccessDataSource>& randomAccessDataSource, UInt32 options, OSType type, UInt32 timeScale,
		UniversalTimeInterval duration, const Internals& internals)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	SQTVideoSampleDescription&	videoSampleDescription = internals.getVideoSampleDescription();

	// Check type
	switch (type) {
		case MAKE_OSTYPE('a', 'v', 'c', '1'): {
			// h.264 Video
			TVResult<CData>	decompressionData = internals.getVideoDecompressionData();
			ReturnValueIfResultError(decompressionData,
					TVResult<CMediaTrackInfos::VideoTrackInfo>(decompressionData.getError()));

			CAtomReader					atomReader(I<CRandomAccessDataSource>(new CDataDataSource(*decompressionData)));
			TVResult<CAtomReader::Atom>	avcCAtom(SError::mEndOfData);
			do {
				// Read
				avcCAtom = atomReader.readAtom();

				// Check if have instance
				if (avcCAtom.hasValue())
					// Seek to next atom
					atomReader.seekToNextAtom(*avcCAtom);
			} while (!avcCAtom.hasError() && (avcCAtom->mType != MAKE_OSTYPE('a', 'v', 'c', 'C')));
			ReturnValueIfResultError(avcCAtom,
					TVResult<CMediaTrackInfos::VideoTrackInfo>(
							CCodec::unsupportedConfigurationError(CString(type, true))));

			TVResult<CData>	avcCAtomPayload = atomReader.readAtomPayload(*avcCAtom);
			ReturnValueIfResultError(avcCAtomPayload,
					TVResult<CMediaTrackInfos::VideoTrackInfo>(avcCAtomPayload.getError()));

			// Setup
			TArray<SMediaPacketAndLocation>	mediaPacketAndLocations = composePacketAndLocations(internals);
			Float32							framerate =
													(Float32) mediaPacketAndLocations.getCount() / (Float32) duration;
			UInt64							byteCount =
													SMediaPacketAndLocation::getTotalByteCount(mediaPacketAndLocations);

			// Compose storage format
			OV<SVideoStorageFormat>	videoStorageFormat =
											CH264VideoCodec::composeVideoStorageFormat(
													S2DSizeU16(videoSampleDescription.getWidth(),
															videoSampleDescription.getHeight()), framerate);
			if (!videoStorageFormat.hasValue())
				// Unsupported configuration
				return TVResult<CMediaTrackInfos::VideoTrackInfo>(
						CCodec::unsupportedConfigurationError(CString(type, true)));

			// Add video track
			CVideoTrack	videoTrack(CMediaTrack::composeInfo(duration, byteCount), *videoStorageFormat);
			if (options & SMediaSource::kOptionsCreateDecoders) {
				// Setup
				TVResult<CData>	stssAtomPayloadData =
										internals.mAtomReader.readAtomPayload(
												internals.mSTBLContainerAtom.getAtom(MAKE_OSTYPE('s', 't', 's', 's')));
				ReturnValueIfResultError(stssAtomPayloadData,
						TVResult<CMediaTrackInfos::VideoTrackInfo>(stssAtomPayloadData.getError()));

				const	SQTstssAtomPayload&		stssAtomPayload =
														*((SQTstssAtomPayload*) stssAtomPayloadData->getBytePtr());
						UInt32					keyframesCount = stssAtomPayload.getKeyframesCount();
						TNumberArray<UInt32>	keyframeIndexes;
				for (UInt32 i = 0; i < keyframesCount; i++)
					// Add keyframe index
					keyframeIndexes += stssAtomPayload.getKeyframeIndex(i);

				// Add video track with decode info
				return TVResult<CMediaTrackInfos::VideoTrackInfo>(
						CMediaTrackInfos::VideoTrackInfo(videoTrack,
								CH264VideoCodec::create(randomAccessDataSource, mediaPacketAndLocations,
										*avcCAtomPayload, timeScale, keyframeIndexes)));
			} else
				// Add video track
				return TVResult<CMediaTrackInfos::VideoTrackInfo>(CMediaTrackInfos::VideoTrackInfo(videoTrack));
			}

		default:
			// Unsupported video codec
			return TVResult<CMediaTrackInfos::VideoTrackInfo>(CCodec::unsupportedError(CString(type, true)));
	}
}

// MARK: Subclass methods

//----------------------------------------------------------------------------------------------------------------------
Float32 CQuickTimeMediaFile::getSampleRate(const Internals& internals) const
//----------------------------------------------------------------------------------------------------------------------
{
	return internals.getAudioSampleDescription().getSampleRate();
}

//----------------------------------------------------------------------------------------------------------------------
UInt8 CQuickTimeMediaFile::getChannels(const Internals& internals) const
//----------------------------------------------------------------------------------------------------------------------
{
	return (UInt8) internals.getAudioSampleDescription().getChannels();
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> CQuickTimeMediaFile::getAudioDecompressionData(const Internals& internals) const
//----------------------------------------------------------------------------------------------------------------------
{
	return internals.getAudioDecompressionData();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
static I<SMediaSource::ImportResult> sImport(const I<CRandomAccessDataSource>& randomAccessDataSource,
		const OI<CAppleResourceManager>& appleResourceManager, UInt32 options)
//----------------------------------------------------------------------------------------------------------------------
{
	return CQuickTimeMediaFile::create()->import(randomAccessDataSource, appleResourceManager, options);
}

//----------------------------------------------------------------------------------------------------------------------
CMediaTrackInfos::AudioTrackInfo sComposePCMAudioTrackInfo(const CQuickTimeMediaFile& quickTimeMediaFile,
		const I<CRandomAccessDataSource>& randomAccessDataSource, UInt32 options, bool isFloat, UInt8 bits,
		CPCMAudioCodec::Format format, UniversalTimeInterval duration, const CQuickTimeMediaFile::Internals& internals)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	SQTAudioSampleDescription&		audioSampleDescription = internals.getAudioSampleDescription();
			Float32							sampleRate = audioSampleDescription.getSampleRate();
			UInt8							channels = (UInt8) audioSampleDescription.getChannels();
			UInt32							bytesPerFrame = bits / 8 * channels;
			TArray<SMediaPacketAndLocation>	mediaPacketAndLocations =
													quickTimeMediaFile.composePacketAndLocations(internals,
															OV<UInt32>(1), OV<UInt32>(bytesPerFrame));

	OV<SAudioStorageFormat>	audioStorageFormat =
									CPCMAudioCodec::composeAudioStorageFormat(isFloat, bits, sampleRate, channels);
	UInt64					frameCount = mediaPacketAndLocations.getCount();

	// Compose audio track
	CAudioTrack	audioTrack(CAudioTrack::composeInfo(duration, *audioStorageFormat, bytesPerFrame),
						*audioStorageFormat);
	if (options & SMediaSource::kOptionsCreateDecoders)
		// Add audio track with decode info
		return CMediaTrackInfos::AudioTrackInfo(audioTrack,
						CPCMAudioCodec::create(*audioStorageFormat, randomAccessDataSource,
								mediaPacketAndLocations.getFirst().mByteOffset,
								frameCount * bytesPerFrame, format));
	else
		// Add audio track
		return CMediaTrackInfos::AudioTrackInfo(audioTrack);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Register media source

static	CString	sExtensions[] = { CString(OSSTR("mov")) };

REGISTER_MEDIA_SOURCE(quicktime,
		SMediaSource(MAKE_OSTYPE('M', 'o', 'o', 'V'), CString(OSSTR("QuickTime")), TSArray<CString>(sExtensions, 1),
				sImport));
