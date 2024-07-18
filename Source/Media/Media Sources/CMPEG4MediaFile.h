//----------------------------------------------------------------------------------------------------------------------
//	CMPEG4MediaFile.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAtomReader.h"
#include "SMediaSource.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMPEG4MediaFile

class CMPEG4MediaFile {
#pragma pack(push, 1)

#if defined(TARGET_OS_WINDOWS)
	#pragma warning(disable:4200)
#endif

	// SmdhdAtomPayload
	public:
		struct SmdhdAtomPayload {
			// Methods
			public:
					// Lifecycle methods
					SmdhdAtomPayload(UniversalTime utcUniversalTime, Float32 sampleRate, UInt64 duration)
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

	// ShdlrAtomPayload
	public:
		struct ShdlrAtomPayload {
			// SubType
			enum SubType {
				kSubTypeSound = MAKE_OSTYPE('s', 'o', 'u', 'n'),
				kSubTypeVideo = MAKE_OSTYPE('v', 'i', 'd', 'e'),
			};

					// Methods
					ShdlrAtomPayload(SubType subType) :
						mVersion(0),
								mQuickTimeType(EndianU32_NtoB(0)), mSubType((SubType) EndianU32_NtoB(subType)),
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
						{ return CData(this, sizeof(ShdlrAtomPayload), false) + CData::mZeroByte + CData::mZeroByte; }

			// Properties (in storage endian)
			private:
				UInt8	mVersion;
				UInt8	mFlags[3];
				OSType	mQuickTimeType;
				SubType	mSubType;
				OSType	mQuickTimeManufacturerType;
				UInt32	mQuickTimeComponentFlags;
				UInt32	mQuickTimeComponentFlagsMask;
				SInt8	mComponentTypeName[];
		};

	// SstsdDescriptionHeader
	public:
		struct SstsdDescriptionHeader {
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

	// SstsdAtomPayload
	public:
		struct SstsdAtomPayload {
			// Methods
			public:
														// Instance methods
						const	SstsdDescriptionHeader&	getFirstDescriptionHeader() const
															{ return *((SstsdDescriptionHeader*) mDescriptions); }

														// Class methods
				static			CData					getData(const TArray<CData>& descriptions);

			private:
														// Lifecycle methods
														SstsdAtomPayload(UInt32 descriptionCount);

			// Properties (in storage endian)
			private:
				UInt8	mVersion;
				UInt8	mFlags[3];
				UInt32	mDescriptionCount;
				UInt8	mDescriptions[];
		};

	// SstsdMP4ADescription
	public:
		struct SstsdMP4ADescription {
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
										SstsdMP4ADescription(UInt32 length, Float32 sampleRate,
												const SAudio::ChannelMap& audioChannelMap, UInt16 dataRefIndex);

										// Instance methods
						CData			getData() const
											{ return CData(this, sizeof(SstsdMP4ADescription), false); }

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

	// SsttsAtomPayload (Sample Table Time-to-Sample Atom Payload)
	public:
		struct SsttsAtomPayload {
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
												data += SsttsAtomPayload(chunks.getCount()).getData();

												// Add chunks
												for (TIteratorD<Chunk> iterator = chunks.getIterator();
														iterator.hasValue(); iterator.advance())
													// Add chunk
													data += iterator->getData();

												return data;
											}

			private:
										// Lifecycle methods
										SsttsAtomPayload(UInt32 chunkCount) :
											mVersion(0), mChunkCount(EndianU32_NtoB(chunkCount))
											{
												// Setup
												mFlags[0] = 0;
												mFlags[1] = 0;
												mFlags[2] = 0;
											}

										// Instance methods
						CData			getData() const
											{ return CData(this, sizeof(SsttsAtomPayload), false); }

			// Properties (in storage endian)
			private:
				UInt8	mVersion;
				UInt8	mFlags[3];
				UInt32	mChunkCount;
				Chunk	mChunks[];
		};

	// SstscAtomPayload (Sample Table Sample-to-Chunk Atom Payload)
	public:
		struct SstscAtomPayload {
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
																	SstscAtomPayload(packetGroupInfos.getCount())
																			.getData();

															// Add Packet Group Infos
															for (TIteratorD<PacketGroupInfo> iterator =
																			packetGroupInfos.getIterator();
																	iterator.hasValue(); iterator.advance())
																// Add Packet Group Info
																data += iterator->getData();

															return data;
														}
				static			CData				getData(const PacketGroupInfo& packetGroupInfo)
														{ return getData(TSArray<PacketGroupInfo>(packetGroupInfo)); }

			private:
													// Lifecycle methods
													SstscAtomPayload(UInt32 packetGroupInfoCount) :
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
														{ return CData(this, sizeof(SstscAtomPayload), false); }

			// Properties (in storage endian)
			private:
				UInt8			mVersion;
				UInt8			mFlags[3];
				UInt32			mPacketGroupInfoCount;
				PacketGroupInfo	mPacketGroupInfos[];
		};

	// SstszAtomPayload (Sample Table Sample siZe Atom Payload)
	public:
		struct SstszAtomPayload {
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
												SstszAtomPayload(
																(packetByteCounts.getCount() == 1) ?
																		packetByteCounts[0] : 0,
																packetByteCounts.getCount())
														.getData();

										// Add packet byte counts
										if (packetByteCounts.getCount() > 1) {
											// Iterate Packet Byte Counts
											for (TIteratorM<TNumber<UInt32>, UInt32> iterator =
															packetByteCounts.getIterator();
													iterator.hasValue(); iterator.advance()) {
												// Add Packet Byte Count
												UInt32	value = EndianU32_NtoB(*iterator);
												data.appendBytes(&value, sizeof(UInt32));
											}
										}

										return data;
									}

			private:
								// Lifecycle methods
								SstszAtomPayload(UInt32 globalPacketByteCount, UInt32 packetByteCountCount) :
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
									{ return CData(this, sizeof(SstszAtomPayload), false); }

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

	// SstcoAtomPayload (Sample Table Chunk Offset Atom Payload)
	public:
		struct SstcoAtomPayload {
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
										data += SstcoAtomPayload(packetGroupOffsets.getCount()).getData();

										// Add packet group offsets
										for (TIteratorM<TNumber<UInt32>, UInt32> iterator =
														packetGroupOffsets.getIterator();
												iterator.hasValue(); iterator.advance()) {
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
								SstcoAtomPayload(UInt32 packetGroupOffsetCount) :
									mVersion(0), mPacketGroupOffsetCount(EndianU32_NtoB(packetGroupOffsetCount))
									{
										// Setup
										mFlags[0] = 0;
										mFlags[1] = 0;
										mFlags[2] = 0;
									}

								// Instance methods
						CData	getData() const
									{ return CData(this, sizeof(SstcoAtomPayload), false); }

			// Properties (in storage endian)
			private:
				UInt8	mVersion;
				UInt8	mFlags[3];

				// For each packet group specified in the stsc Atom, the file offset is specified
				UInt32	mPacketGroupOffsetCount;
				UInt32	mPacketGroupOffsets[];		// Offset to start of packet data from start of file
		};


#if defined(TARGET_OS_WINDOWS)
	#pragma warning(default:4200)
#endif

#pragma pack(pop)

	// Internals
	public:
		struct Internals;

	// Methods
	public:
															// Lifecycle methods
															CMPEG4MediaFile() {}
		virtual												~CMPEG4MediaFile() {}

															// Instance methods
		virtual	I<SMediaSource::ImportResult>				import(const SMediaSource::ImportSetup& importSetup);
				TArray<SMedia::PacketAndLocation>			composePacketAndLocations(const Internals& internals) const;

	protected:
															// Instance methods
		virtual	TVResult<SMediaSource::Tracks::AudioTrack>	composeAudioTrack(
																	const I<CRandomAccessDataSource>&
																			randomAccessDataSource,
																	UInt32 options, OSType type,
																	UniversalTimeInterval duration,
																	const Internals& internals);
		virtual	TVResult<SMediaSource::Tracks::VideoTrack>	composeVideoTrack(
																	const I<CRandomAccessDataSource>&
																			randomAccessDataSource,
																	UInt32 options, OSType type,
																	UInt32 timeScale,
																	UniversalTimeInterval duration,
																	const Internals& internals);
		virtual	OV<SError>									importTrack(
																	const I<CRandomAccessDataSource>&
																			randomAccessDataSource,
																	OSType type,
																	const SstsdDescriptionHeader& stsdDescriptionHeader,
																	const Internals& internals)
																{ return OV<SError>(); }
		virtual	void										processFileMetadata(const CData& metaAtomPayloadData) {}

															// Subclass methods
				TVResult<CData>								getDecompressionData(const Internals& internals,
																	SInt64 offset) const;

	// Properties
	public:
		static	OSType	mID;
};
