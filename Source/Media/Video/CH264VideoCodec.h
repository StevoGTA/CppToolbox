//----------------------------------------------------------------------------------------------------------------------
//	CH264VideoCodec.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CBitReader.h"
#include "CVideoCodec.h"
#include "SMediaPacket.h"
#include "SVideoFormats.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CH264VideoCodec

/*
	Helpful references:
		https://stackoverflow.com/questions/29525000/how-to-use-videotoolbox-to-decompress-h-264-video-stream
		https://stackoverflow.com/questions/24884827/possible-locations-for-sequence-picture-parameter-sets-for-h-264-stream/24890903#24890903

		https://chromium.googlesource.com/chromium/src/media/+/refs/heads/main/video/h264_poc.cc
*/

class CH264VideoCodecInternals;
class CH264VideoCodec : public CDecodeOnlyVideoCodec {
	// NALUInfo
	public:
		// Network Abstraction Layer Unit Info
		struct NALUInfo {

			// Type
			enum Type {
				kTypeUnspecified1,											// 0, Non-VCL
				kTypeCodedSliceNonIDRPicture,								// 1, VCL
				kTypeCodedSliceDataPartitionA,								// 2, VCL
				kTypeCodedSliceDataPartitionB,								// 3, VCL
				kTypeCodedSliceDataPartitionC,								// 4, VCL
				kTypeCodedSliceIDRPicture,									// 5, VCL
				kTypeSupplementalEnhancementInformation,					// 6, (SEI) Non-VCL
				kTypeSequenceParameterSet,									// 7, (SPS) Non-VCL
				kTypePictureParameterSet,									// 8, (PPS) Non-VCL
				kTypeAccessUnitDelimiter,									// 9, Non-VCL
				kTypeEndOfSequence,											// 10, Non-VCL
				kTypeEndOfStream,											// 11, Non-VCL
				kTypeFillerData,											// 12, Non-VCL
				kTypeSequenceParameterSetExtension,							// 13, Non-VCL
				kTypePrefixNALU,											// 14, Non-VCL
				kTypeSubsetSequenceParameterSet,							// 15, Non-VCL
				kTypeDepthParameterSet,										// 16, Non-VCL
				kTypeReserved1,												// 17, Non-VCL
				kTypeReserved2,												// 18, Non-VCL
				kTypeCodedSliceAuxiliaryCodedPictureWithoutPartitioning,	// 19, Non-VCL
				kTypeCodedSliceExtension,									// 20, Non-VCL
				kTypeCodedSliceExtensionDepthViewingComponents,				// 21, Non-VCL
				kTypeReserved3,												// 22, Non-VCL
				kTypeReserved4,												// 23, Non-VCL
				kTypeUnspecified2,											// 24, Non-VCL
				kTypeUnspecified3,											// 25, Non-VCL
				kTypeUnspecified4,											// 26, Non-VCL
				kTypeUnspecified5,											// 27, Non-VCL
				kTypeUnspecified6,											// 28, Non-VCL
				kTypeUnspecified7,											// 29, Non-VCL
				kTypeUnspecified8,											// 30, Non-VCL
				kTypeUnspecified9,											// 31, Non-VCL
			};

											// Lifecycle methods
											NALUInfo(const CData& data, UInt32 offset, CData::ByteCount byteCount) :
												mData(data), mOffset(offset), mByteCount(byteCount)
												{}
											NALUInfo(const NALUInfo& other) :
												mData(other.mData), mOffset(other.mOffset), mByteCount(other.mByteCount)
												{}

											// Instance methods
						Type				getType() const
												{ return (Type) (*((const UInt8*) getBytePtr()) & 0x1F); }
				const	UInt8*				getBytePtr() const
												{ return (UInt8*) mData.getBytePtr() + mOffset; }
						CData::ByteCount	getByteCount() const
												{ return mByteCount; }

											// Class methods
		static			TArray<NALUInfo>	getNALUInfos(const CData& data);
		static			CData				composeAnnexB(const TArray<NALUInfo>& spsNALUInfos,
													const TArray<NALUInfo>& ppsNALUInfos,
													const TArray<NALUInfo>& naluInfos);

			// Properties
			private:
				CData				mData;
				UInt32				mOffset;
				CData::ByteCount	mByteCount;
		};

	// SequenceParameterSetPayload
	public:
		struct SequenceParameterSetPayload {
			// Lifecycle methods
			SequenceParameterSetPayload(const CData& data)
				{
					// Setup
					CBitReader	bitReader(I<CSeekableDataSource>(new CDataDataSource(data)), true);

					// Decode
					mForbiddenZero = bitReader.readUInt8(1).getValue();
					mNALRef = bitReader.readUInt8(2).getValue();
					mNALUnitType = (NALUInfo::Type) bitReader.readUInt8(5).getValue();
					mProfile = bitReader.readUInt8().getValue();
					mConstraintSet0Flag = bitReader.readUInt8(1).getValue();
					mConstraintSet1Flag = bitReader.readUInt8(1).getValue();
					mConstraintSet2Flag = bitReader.readUInt8(1).getValue();
					mConstraintSet3Flag = bitReader.readUInt8(1).getValue();
					mConstraintSet4Flag = bitReader.readUInt8(1).getValue();
					mConstraintSet5Flag = bitReader.readUInt8(1).getValue();
					mReserved2Bits = bitReader.readUInt8(2).getValue();
					mLevel = bitReader.readUInt8().getValue();
					mSPSID = (UInt8) bitReader.readUEColumbusCode().getValue();
					mFrameNumberBitCount = (UInt8) bitReader.readUEColumbusCode().getValue() + 4;
					mPicOrderCountType = (UInt8) bitReader.readUEColumbusCode().getValue();
					mPicOrderCountLSBBitCount = (UInt8) bitReader.readUEColumbusCode().getValue() + 4;
				}

			// Properties
			UInt8			mForbiddenZero;
			UInt8			mNALRef;
			NALUInfo::Type	mNALUnitType;
			UInt8			mProfile;
			UInt8			mConstraintSet0Flag : 1;
			UInt8			mConstraintSet1Flag : 1;
			UInt8			mConstraintSet2Flag : 1;
			UInt8			mConstraintSet3Flag : 1;
			UInt8			mConstraintSet4Flag : 1;
			UInt8			mConstraintSet5Flag : 1;
			UInt8			mReserved2Bits : 2;
			UInt8			mLevel;
			UInt8			mSPSID;
			UInt8			mFrameNumberBitCount;
			UInt8			mPicOrderCountType;
			UInt8			mPicOrderCountLSBBitCount;
		};

	// DecodeInfo
	public:
		class DecodeInfo : public CCodec::MediaPacketSourceDecodeInfo {
			// SPSPPSInfo
			public:
				struct SPSPPSInfo {
												// Lifecycle methods
												SPSPPSInfo(const TArray<NALUInfo>& spsNALUInfos,
														const TArray<NALUInfo>& ppsNALUInfos) :
													mSPSNALUInfos(spsNALUInfos), mPPSNALUInfos(ppsNALUInfos)
													{}
												SPSPPSInfo(const SPSPPSInfo& other) :
													mSPSNALUInfos(other.mSPSNALUInfos),
															mPPSNALUInfos(other.mPPSNALUInfos)
													{}

												// Instance methods
					const	TArray<NALUInfo>&	getSPSNALUInfos() const
													{ return mSPSNALUInfos; }
					const	TArray<NALUInfo>&	getPPSNALUInfos() const
													{ return mPPSNALUInfos; }

					// Properties
					private:
						TArray<NALUInfo>	mSPSNALUInfos;
						TArray<NALUInfo>	mPPSNALUInfos;
				};

			// Configuration
			private:
#if defined(TARGET_OS_WINDOWS)
	#pragma warning(disable:4200)
#endif
				struct Configuration {
					// Properties
					UInt8	mVersion;
					UInt8	mProfile;
					UInt8	mProfileCompatibility;
					UInt8	mLevel;
					UInt8	mLengthCoded;
					UInt8	mSPSPPSInfo[];
				};
#if defined(TARGET_OS_WINDOWS)
	#pragma warning(default:4200)
#endif

			// Methods
			public:
												// Lifecycle methods
												DecodeInfo(const I<CMediaPacketSource>& mediaPacketSource,
														const CData& configurationData, UInt32 timeScale,
														const TNumericArray<UInt32>& keyframeIndexes) :
													CCodec::MediaPacketSourceDecodeInfo(mediaPacketSource),
															mConfigurationData(configurationData),
															mTimeScale(timeScale), mKeyframeIndexes(keyframeIndexes)
													{}

												// Instance methods
						UInt32					getNALUHeaderLengthSize() const
													{
														// Setup
														const	Configuration&	configuration =
																						*((Configuration*)
																								mConfigurationData
																										.getBytePtr());

														return (configuration.mLengthCoded & ~0xFC) + 1;

													}
						SPSPPSInfo				getSPSPPSInfo() const
													{
														// Setup
														const	Configuration&		configuration =
																							*((Configuration*)
																									mConfigurationData
																											.getBytePtr());
														const	UInt8*				bytePtr =
																							&configuration
																									.mSPSPPSInfo[0];
																UInt32				offset = sizeof(Configuration);

																TNArray<NALUInfo>	spsNALUInfos;
																TNArray<NALUInfo>	ppsNALUInfos;

														// Compose SPS NALUInfos
														UInt32	count = *bytePtr & 0x1F;
														bytePtr++;
														offset++;
														for (UInt32 i = 0; i < count; i++) {
															// Add SPS NALUInfo
															UInt16	byteCount = getByteCount(bytePtr);
															spsNALUInfos +=
																	NALUInfo(mConfigurationData, offset + 2, byteCount);

															// Update
															bytePtr += 2 + byteCount;
															offset += 2 + byteCount;
														}

														// Compose PPS NALUInfos
														count = *bytePtr;
														bytePtr++;
														offset++;
														for (UInt32 i = 0; i < count; i++) {
															// Add PPS NALUInfo
															UInt16	byteCount = getByteCount(bytePtr);
															ppsNALUInfos +=
																	NALUInfo(mConfigurationData, offset + 2, byteCount);

															// Update
															bytePtr += 2 + byteCount;
															offset += 2 + byteCount;
														}

														return SPSPPSInfo(spsNALUInfos, ppsNALUInfos);
													}

						UInt32					getTimeScale() const
													{ return mTimeScale; }

				const	TNumericArray<UInt32>&	getKeyframeIndexes() const
													{ return mKeyframeIndexes; }

			private:
												// Private methods
						UInt16					getByteCount(const UInt8* ptr) const
													{ return (*ptr << 8) | *(ptr + 1); }

			// Properties
			private:
				CData					mConfigurationData;
				UInt32					mTimeScale;
				TNumericArray<UInt32>	mKeyframeIndexes;
		};

	// FrameTiming
	public:
		class FrameTiming {
			// Times
			public:
				struct Times {
					// Methods
					Times(UInt64 decodeTime, UInt64 presentationTime) :
						mDecodeTime(decodeTime), mPresentationTime(presentationTime)
						{}

					// Properties
					UInt64	mDecodeTime;
					UInt64	mPresentationTime;
				};

			// Methods
			public:
								// Lifecycle methods
								FrameTiming(const CH264VideoCodec::SequenceParameterSetPayload& spsPayload);

								// Instance methods
				void			seek(UInt64 frameTime)
									{ mNextFrameTime = frameTime; }
				TIResult<Times>	updateFrom(const CMediaPacketSource::DataInfo& dataInfo);

			// Properties
			private:
				UInt8	mCurrentFrameNumberBitCount;
				UInt8	mCurrentPicOrderCountLSBBitCount;
				UInt8	mPicOrderCountMSBChangeThreshold;

				UInt64	mPicOrderCountMSB;
				UInt64	mPreviousPicOrderCountLSB;
				UInt64	mLastIDRFrameTime;
				UInt64	mCurrentFrameTime;
				UInt64	mNextFrameTime;
		};

	// Methods
	public:
										// Lifecycle methods
										CH264VideoCodec();
										~CH264VideoCodec();

										// CVideoCodec methods
				OI<SError>				setupForDecode(const SVideoProcessingFormat& videoProcessingFormat,
												const I<CCodec::DecodeInfo>& decodeInfo);
				void					seek(UniversalTimeInterval timeInterval);
				TIResult<CVideoFrame>	decode();

										// Class methods
		static	OI<SVideoStorageFormat>	composeVideoStorageFormat(const S2DSizeU16& frameSize, Float32 framerate);
		static	I<CCodec::DecodeInfo>	composeDecodeInfo(const I<CSeekableDataSource>& seekableDataSource,
												const TArray<SMediaPacketAndLocation>& packetAndLocations,
												const CData& configurationData, UInt32 timeScale,
												const TNumericArray<UInt32>& keyframeIndexes);

	// Properties
	public:
		static	OSType						mID;

	private:
				CH264VideoCodecInternals*	mInternals;
};
