//----------------------------------------------------------------------------------------------------------------------
//	SMPEG4.h			©2025 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CData.h"
#include "SAudio.h"
#include "TResult.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SMPEG4

struct SMPEG4 {
#pragma pack(push, 1)

#if defined(TARGET_OS_WINDOWS)
	#pragma warning(disable:4200)
#endif

	// CO64AtomPayload (Sample Table Chunk Offset Atom Payload for large files)
	public:
		struct CO64AtomPayload {
			// Methods
			public:
						// Instance methods
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

	// HDLRAtomPayload
	public:
		struct HDLRAtomPayload {
			// SubType
			enum SubType {
				kSubTypeSound		= MAKE_OSTYPE('s', 'o', 'u', 'n'),
				kSubTypeVideo		= MAKE_OSTYPE('v', 'i', 'd', 'e'),
				kSubTypeMetadata	= MAKE_OSTYPE('m', 'd', 'i', 'r'),
			};

					// Methods
					HDLRAtomPayload(SubType subType) :
						mVersion(0),
								mQuickTimeType(EndianU32_NtoB(0)), mSubType(EndianU32_NtoB(subType)),
								mQuickTimeManufacturerType(EndianU32_NtoB(0)),
								mQuickTimeComponentFlags(EndianU32_NtoB(0)),
								mQuickTimeComponentFlagsMask(EndianU32_NtoB(0))
						{
							// Finish setup
							mFlags[0] = 0;
							mFlags[1] = 0;
							mFlags[2] = 0;
						}

			SubType	getSubType() const
						{ return (SubType) EndianU32_BtoN(mSubType); }

			CData	getData() const
						{ return CData(this, sizeof(HDLRAtomPayload), false) + CData::mZeroByte + CData::mZeroByte; }

			// Properties (in storage endian)
			private:
				UInt8	mVersion;
				UInt8	mFlags[3];
				OSType	mQuickTimeType;
				OSType	mSubType;
				OSType	mQuickTimeManufacturerType;
				UInt32	mQuickTimeComponentFlags;
				UInt32	mQuickTimeComponentFlagsMask;
				SInt8	mComponentTypeName[];
		};

	// MDHDAtomPayload
	public:
		struct MDHDAtomPayload {
			// Methods
			public:
					// Lifecycle methods
					MDHDAtomPayload(UniversalTime utcUniversalTime, Float32 sampleRate, UInt64 duration)
						{
							// Setup
							mFlags[0] = 0;
							mFlags[1] = 0;
							mFlags[2] = 0;

							// Check duration
							if (duration < 0x0100000000) {
								// Version 0
								mVersion = 0;

								_.mInfoV0.mCreatedDate = EndianU32_NtoB((UInt32) utcUniversalTime);
								_.mInfoV0.mModifiedDate = EndianU32_NtoB((UInt32) utcUniversalTime);

								_.mInfoV0.mTimeScale = EndianU32_NtoB((UInt32) sampleRate);
								_.mInfoV0.mDuration = EndianU32_NtoB((UInt32) duration);

								_.mInfoV0.mLanguageCode = EndianU16_NtoB(0x55c4);
								_.mInfoV0.mQuickTimeQuality = EndianU16_NtoB(0);
							} else {
								// Version 1
								mVersion = 1;

								_.mInfoV1.mCreatedDate = EndianU64_NtoB((UInt64) utcUniversalTime);
								_.mInfoV1.mModifiedDate = EndianU64_NtoB((UInt64) utcUniversalTime);

								_.mInfoV1.mTimeScale = EndianU32_NtoB((UInt32) sampleRate);
								_.mInfoV1.mDuration = EndianU64_NtoB(duration);

								_.mInfoV1.mLanguageCode = EndianU16_NtoB(0x55c4);
								_.mInfoV1.mQuickTimeQuality = EndianU16_NtoB(0);
							}
						}

					// Instance Methods
			UInt32	getTimeScale() const
						{ return (mVersion == 0) ?
								EndianU32_BtoN(_.mInfoV0.mTimeScale) :
								EndianU32_BtoN(_.mInfoV1.mTimeScale); }
			UInt64	getDuration() const
						{ return (mVersion == 0) ?
								(UInt64) EndianU32_BtoN(_.mInfoV0.mDuration) :
								EndianU64_BtoN(_.mInfoV1.mDuration); }
			CData	getData() const
						{ return CData(this, (mVersion == 0) ? 4 + sizeof(Infos::InfoV0) : 4 + sizeof(Infos::InfoV1),
								false); }

			// Properties (in storage endian)
			private:
				UInt8	mVersion;
				UInt8	mFlags[3];

				union Infos {
					struct InfoV0 {
						UInt32	mCreatedDate;
						UInt32	mModifiedDate;

						UInt32	mTimeScale;
						UInt32	mDuration;

						UInt16	mLanguageCode;
						UInt16	mQuickTimeQuality;
					} mInfoV0;

					struct InfoV1 {
						UInt64	mCreatedDate;
						UInt64	mModifiedDate;

						UInt32	mTimeScale;
						UInt64	mDuration;

						UInt16	mLanguageCode;
						UInt16	mQuickTimeQuality;
					} mInfoV1;
				} _;
		};

	// STCOAtomPayload (Sample Table Chunk Offset Atom Payload)
	public:
		struct STCOAtomPayload {
			// Methods
			public:
								// Instance methods
						UInt32	getPacketGroupOffsetCount() const
									{ return EndianU32_BtoN(mPacketGroupOffsetCount); }
						UInt64	getPacketGroupOffset(UInt32 index) const
									{ return EndianU32_BtoN(mPacketGroupOffsets[index]); }

								// Class methods
				static	CData	getData(const TNumberArray<UInt32>& packetGroupOffsets)
									{
										// Setup
										CData	data;
										data += STCOAtomPayload(packetGroupOffsets.getCount()).getData();

										// Add packet group offsets
										for (TNumberArray<UInt32>::Iterator iterator = packetGroupOffsets.getIterator();
												iterator; iterator++) {
											// Add packet group offset
											UInt32	value = EndianU32_NtoB(*iterator);
											data.appendBytes(&value, sizeof(UInt32));
										}

										return data;
									}
				static	CData	getData(UInt32 packetGroupOffset)
									{ return getData(TNumberArray<UInt32>(packetGroupOffset, 1)); }

			private:
								// Lifecycle methods
								STCOAtomPayload(UInt32 packetGroupOffsetCount) :
									mVersion(0), mPacketGroupOffsetCount(EndianU32_NtoB(packetGroupOffsetCount))
									{
										// Setup
										mFlags[0] = 0;
										mFlags[1] = 0;
										mFlags[2] = 0;
									}

								// Instance methods
						CData	getData() const
									{ return CData(this, sizeof(STCOAtomPayload), false); }

			// Properties (in storage endian)
			private:
				UInt8	mVersion;
				UInt8	mFlags[3];

				// For each packet group specified in the stsc Atom, the file offset is specified
				UInt32	mPacketGroupOffsetCount;
				UInt32	mPacketGroupOffsets[];		// Offset to start of packet data from start of file
		};

	// STSDDescriptionHeader
	public:
		struct STSDDescriptionHeader {
			// Methods
			public:
						// Instance methods
				OSType	getType() const
							{ return EndianU32_BtoN(mType); }

			// Properties (in storage endian)
			private:
				UInt32	mLength;
				OSType	mType;
				UInt8	mReserved[6];
				UInt16	mDataRefIndex;
		};

	// STSCAtomPayload (Sample Table Sample-to-Chunk Atom Payload)
	public:
		struct STSCAtomPayload {
			// PacketGroupInfo
			public:
				struct PacketGroupInfo {
					// Methods
					public:
								// Lifecycle methods
								PacketGroupInfo(UInt32 chunkStartIndex, UInt32 packetCount,
										UInt32 sampleDescriptionIndex) :
									mChunkStartIndex(EndianU32_NtoB(chunkStartIndex)),
											mPacketCount(EndianU32_NtoB(packetCount)),
											mSampleDescriptionIndex(EndianU32_NtoB(sampleDescriptionIndex))
									{}
#if defined(TARGET_OS_WINDOWS)
								PacketGroupInfo() : mChunkStartIndex(0), mPacketCount(0), mSampleDescriptionIndex(0) {}
#endif

								// Instance methods
						UInt32	getChunkStartIndex() const
									{ return EndianU32_BtoN(mChunkStartIndex); }
						UInt32	getPacketCount() const
									{ return EndianU32_BtoN(mPacketCount); }
						CData	getData() const
									{ return CData(this, sizeof(PacketGroupInfo), false); }

					// Properties (in storage endian)
					private:
						UInt32	mChunkStartIndex;
						UInt32	mPacketCount;
						UInt32	mSampleDescriptionIndex;
				};

			// Methods
			public:
													// Instance methods
								UInt32				getPacketGroupInfoCount() const
														{ return EndianU32_BtoN(mPacketGroupInfoCount); }
						const	PacketGroupInfo&	getPacketGroupInfo(UInt32 index) const
														{ return mPacketGroupInfos[index]; }

													// Class methods
				static			CData				getData(const TArray<PacketGroupInfo>& packetGroupInfos)
														{
															// Setup
															CData	data;
															data +=
																	STSCAtomPayload(packetGroupInfos.getCount())
																			.getData();

															// Add Packet Group Infos
															for (TArray<PacketGroupInfo>::Iterator iterator =
																			packetGroupInfos.getIterator();
																	iterator; iterator++)
																// Add Packet Group Info
																data += iterator->getData();

															return data;
														}
				static			CData				getData(const PacketGroupInfo& packetGroupInfo)
														{ return getData(TSArray<PacketGroupInfo>(packetGroupInfo)); }

			private:
													// Lifecycle methods
													STSCAtomPayload(UInt32 packetGroupInfoCount) :
														mVersion(0),
																mPacketGroupInfoCount(
																		EndianU32_NtoB(packetGroupInfoCount))
														{
															// Setup
															mFlags[0] = 0;
															mFlags[1] = 0;
															mFlags[2] = 0;
														}

													// Instance methods
						CData						getData() const
														{ return CData(this, sizeof(STSCAtomPayload), false); }

			// Properties (in storage endian)
			private:
				UInt8			mVersion;
				UInt8			mFlags[3];
				UInt32			mPacketGroupInfoCount;
				PacketGroupInfo	mPacketGroupInfos[];
		};

	// STSDAtomPayload
	public:
		struct STSDAtomPayload {
			// Methods
			public:
														// Instance methods
						const	STSDDescriptionHeader&	getFirstDescriptionHeader() const
															{ return *((STSDDescriptionHeader*) mDescriptions); }

														// Class methods
				static			CData					getData(const TArray<CData>& descriptions);

			private:
														// Lifecycle methods
														STSDAtomPayload(UInt32 descriptionCount) :
															mVersion(0),
																	mDescriptionCount(EndianU32_NtoB(descriptionCount))
															{
																// Setup
																mFlags[0] = 0;
																mFlags[1] = 0;
																mFlags[2] = 0;
															}

			// Properties (in storage endian)
			private:
				UInt8	mVersion;
				UInt8	mFlags[3];
				UInt32	mDescriptionCount;
				UInt8	mDescriptions[];
		};

	// STSDH264Description
	public:
		struct STSDH264Description {
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

	// STSDMP4ADescription
	public:
		struct STSDMP4ADescription {
			// Type
			enum { kType = MAKE_OSTYPE('m', 'p', '4', 'a') };

			// Methods
			public:
										// Instance methods
						UInt16			getChannelCount() const
											{ return EndianU16_BtoN(mChannelCounts); }

										// Class methods
				static	TVResult<CData>	getData(Float32 sampleRate, const SAudio::ChannelMap& audioChannelMap,
												UInt16 dataRefIndex, const CData& magicCookie);

			private:
										// Lifecycle methods
										STSDMP4ADescription(UInt32 length, Float32 sampleRate,
												const SAudio::ChannelMap& audioChannelMap, UInt16 dataRefIndex) :
											mLength(EndianU32_NtoB(length)), mType(EndianU32_NtoB(kType)),
													mDataRefIndex(EndianU16_NtoB(dataRefIndex)),
													mQuickTimeEncodingVersion(EndianU16_NtoB(0)),
													mQuickTimeEncodingRevisionLevel(EndianU16_NtoB(0)),
													mQuickTimeEncodingVendor(EndianU32_NtoB(0)),
													mChannelCounts(EndianU16_NtoB(audioChannelMap.getChannelCount())),
													mBits(EndianU16_NtoB(16)),
													mQuickTimeCompressionID(EndianU16_NtoB(0)),
													mQuickTimePacketSize(EndianU16_NtoB(0)),
													mSampleRate(EndianU32_NtoB((UInt32) sampleRate << 16))
											{
												// Setup
												mReserved[0] = 0;
												mReserved[1] = 0;
												mReserved[2] = 0;
												mReserved[3] = 0;
												mReserved[4] = 0;
												mReserved[5] = 0;
											}

										// Instance methods
						CData			getData() const
											{ return CData(this, sizeof(STSDMP4ADescription), false); }

			// Properties (in storage endian)
			private:
				UInt32	mLength;
				OSType	mType;
				UInt8	mReserved[6];
				UInt16	mDataRefIndex;

				UInt16	mQuickTimeEncodingVersion;
				UInt16	mQuickTimeEncodingRevisionLevel;
				OSType	mQuickTimeEncodingVendor;
				UInt16	mChannelCounts;
				UInt16	mBits;
				UInt16	mQuickTimeCompressionID;
				UInt16	mQuickTimePacketSize;
				UInt32	mSampleRate;
		};

	// STSSAtomPayload (Sample Table Sync Sample Atom Payload)
	public:
		struct STSSAtomPayload {
			// Methods
			public:
						// Instance methods
				UInt32	getKeyframesCount() const
							{ return EndianU32_BtoN(mKeyframesCount); }
				UInt32	getKeyframeIndex(UInt32 index) const
							{ return EndianU32_BtoN(mKeyFrameIndexes[index]) - 1; }

			// Properties (in storage endian)
			private:
				UInt8	mVersion;
				UInt8	mFlags[3];
				UInt32	mKeyframesCount;
				UInt32	mKeyFrameIndexes[];
		};

	// STSZAtomPayload (Sample Table Sample siZe Atom Payload)
	public:
		struct STSZAtomPayload {
			// Methods
			public:
								// Instance methods
						UInt32	getPacketByteCount(UInt32 index) const
										{ return (mGlobalPacketByteCount != 0) ?
												EndianU32_BtoN(mGlobalPacketByteCount) :
												EndianU32_BtoN(mPacketByteCounts[index]); }

								// Class methods
				static	CData	getData(const TNumberArray<UInt32>& packetByteCounts)
									{
										// Setup
										CData	data;
										data +=
												STSZAtomPayload(
																(packetByteCounts.getCount() == 1) ?
																		packetByteCounts[0] : 0,
																packetByteCounts.getCount())
														.getData();

										// Add packet byte counts
										if (packetByteCounts.getCount() > 1) {
											// Iterate Packet Byte Counts
											for (TNumberArray<UInt32>::Iterator iterator =
															packetByteCounts.getIterator();
													iterator; iterator++) {
												// Add Packet Byte Count
												UInt32	value = EndianU32_NtoB(*iterator);
												data.appendBytes(&value, sizeof(UInt32));
											}
										}

										return data;
									}

			private:
								// Lifecycle methods
								STSZAtomPayload(UInt32 globalPacketByteCount, UInt32 packetByteCountCount) :
									mVersion(0), mGlobalPacketByteCount(EndianU32_NtoB(globalPacketByteCount)),
											mPacketByteCountCount(EndianU32_NtoB(packetByteCountCount))
									{
										// Setup
										mFlags[0] = 0;
										mFlags[1] = 0;
										mFlags[2] = 0;
									}

								// Instance methods
						CData	getData() const
									{ return CData(this, sizeof(STSZAtomPayload), false); }

			// Properties (in storage endian)
			private:
				UInt8	mVersion;
				UInt8	mFlags[3];

				// If every packet has the same size, specify it here
				UInt32	mGlobalPacketByteCount;	// 0 means use packet sizes below

				// For every individual packet, specify the packet size
				UInt32	mPacketByteCountCount;
				UInt32	mPacketByteCounts[];	// Packet size in bytes
		};

	// STTSAtomPayload (Sample Table Time-to-Sample Atom Payload)
	public:
		struct STTSAtomPayload {
			// Chunk
			public:
				struct Chunk {
					// Methods
					public:
								// Lifecycle methods
								Chunk(UInt32 packetCount, UInt32 packetDuration) :
									mPacketCount(EndianU32_NtoB(packetCount)),
											mPacketDuration(EndianU32_NtoB(packetDuration))
									{}
#if defined(TARGET_OS_WINDOWS)
								Chunk() : mPacketCount(0), mPacketDuration(0) {}
#endif

								// Instance methods
						UInt32	getPacketCount() const
									{ return EndianU32_BtoN(mPacketCount); }
						UInt32	getPacketDuration() const
									{ return EndianU32_BtoN(mPacketDuration); }

						CData	getData() const
									{ return CData(this, sizeof(Chunk), false); }

					// Properties (in storage endian)
					private:
						UInt32	mPacketCount;
						UInt32	mPacketDuration;
				};

			// Methods
			public:
										// Instance methods
								UInt32	getChunkCount() const
											{ return EndianU32_BtoN(mChunkCount); }
						const	Chunk&	getChunk(UInt32 index) const
											{ return mChunks[index]; }

										// Class methods
				static	CData			getData(const TArray<Chunk>& chunks)
											{
												// Setup
												CData	data;
												data += STTSAtomPayload(chunks.getCount()).getData();

												// Add chunks
												for (TArray<Chunk>::Iterator iterator = chunks.getIterator(); iterator;
														iterator++)
													// Add chunk
													data += iterator->getData();

												return data;
											}

			private:
										// Lifecycle methods
										STTSAtomPayload(UInt32 chunkCount) :
											mVersion(0), mChunkCount(EndianU32_NtoB(chunkCount))
											{
												// Setup
												mFlags[0] = 0;
												mFlags[1] = 0;
												mFlags[2] = 0;
											}

										// Instance methods
						CData			getData() const
											{ return CData(this, sizeof(STTSAtomPayload), false); }

			// Properties (in storage endian)
			private:
				UInt8	mVersion;
				UInt8	mFlags[3];
				UInt32	mChunkCount;
				Chunk	mChunks[];
		};

#if defined(TARGET_OS_WINDOWS)
	#pragma warning(default:4200)
#endif

#pragma pack(pop)
};
