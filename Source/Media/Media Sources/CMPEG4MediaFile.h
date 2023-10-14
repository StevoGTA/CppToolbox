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
											_.mInfoV0.mQuickTimeQuality = EndianU16_NtoB(0)'
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
			UInt32				getTimeScale() const
									{
										return (mVersion == 0) ?
												EndianU32_BtoN(_.mInfoV0.mTimeScale) :
												EndianU32_BtoN(_.mInfoV1.mTimeScale);
									}
			UInt64				getDuration() const
									{
										return (mVersion == 0) ?
												(UInt64) EndianU32_BtoN(_.mInfoV0.mDuration) :
												EndianU64_BtoN(_.mInfoV1.mDuration);
									}
			CData::ByteCount	getByteCount() const
									{ return (mVersion == 0) ? 4 + sizeof(Infos::InfoV0) : 4 + sizeof(Infos::InfoV1); }

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
						UInt64	mCreationDate;
						UInt64	mModificationDate;

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
						{ return CData(this, sizeof(ShdlrAtomPayload)) + CData::mZeroByte + CData::mZeroByte; }

			// Properties (in storage endian)
			private:
				UInt8	mVersion;						// 0
				UInt8	mFlags[3];
				OSType	mQuickTimeType;
				SubType	mSubType;
				OSType	mQuickTimeManufacturerType;
				UInt32	mQuickTimeComponentFlags;
				UInt32	mQuickTimeComponentFlagsMask;
				SInt8	mComponentTypeName[];
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
		virtual														~CMPEG4MediaFile() {}

																	// Instance methods
		virtual			I<SMediaSource::ImportResult>				import(
																			const SMediaSource::ImportSetup&
																					importSetup);
						TArray<SMedia::PacketAndLocation>			composePacketAndLocations(
																			const Internals& internals) const;

	protected:
																	// Lifecycle methods
																	CMPEG4MediaFile() {}

																	// Instance methods
		virtual			TVResult<CMediaTrackInfos::AudioTrackInfo>	composeAudioTrackInfo(
																			const I<CRandomAccessDataSource>&
																					randomAccessDataSource,
																			UInt32 options, OSType type,
																			UniversalTimeInterval duration,
																			const Internals& internals);
		virtual			TVResult<CMediaTrackInfos::VideoTrackInfo>	composeVideoTrackInfo(
																			const I<CRandomAccessDataSource>&
																					randomAccessDataSource,
																			UInt32 options, OSType type,
																			UInt32 timeScale,
																			UniversalTimeInterval duration,
																			const Internals& internals);
		virtual			void										process(const CAtomReader& atomReader,
																			const CAtomReader::Atom& atom) {}

																	// Subclass methods
				const	void*										getSampleDescription(const Internals& internals)
																			const;
						TVResult<CData>								getDecompressionData(const Internals& internals,
																			SInt64 offset) const;

	// Properties
	public:
		static	OSType	mID;
};
