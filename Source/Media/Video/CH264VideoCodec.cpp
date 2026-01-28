//----------------------------------------------------------------------------------------------------------------------
//	CH264VideoCodec.cpp			©2021 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CH264VideoCodec.h"

#include "CBitReader.h"
#include "CCodecRegistry.h"
#include "CLogServices.h"

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
	#include "CCoreMediaVideoCodec.h"
	#include "SError-Apple.h"
#elif defined(TARGET_OS_WINDOWS)
	#include "CMediaFoundationServices.h"
	#include "CMediaFoundationDecodeVideoCodec.h"
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: Notes

/*
	Helpful references:
		https://stackoverflow.com/questions/29525000/how-to-use-videotoolbox-to-decompress-h-264-video-stream
		https://stackoverflow.com/questions/24884827/possible-locations-for-sequence-picture-parameter-sets-for-h-264-stream/24890903#24890903

		https://chromium.googlesource.com/chromium/src/media/+/refs/heads/main/video/h264_poc.cc
*/

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local data

// Network Abstraction Layer Unit Info
struct SH264NALUInfo {

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
											SH264NALUInfo(const CData& data, UInt32 offset,
													CData::ByteCount byteCount) :
												mData(data), mOffset(offset), mByteCount(byteCount)
												{}
											SH264NALUInfo(const SH264NALUInfo& other) :
												mData(other.mData), mOffset(other.mOffset), mByteCount(other.mByteCount)
												{}

											// Instance methods
					Type					getType() const
												{ return (Type) (*((const UInt8*) getBytePtr()) & 0x1F); }
			const	UInt8*					getBytePtr() const
												{ return (UInt8*) mData.getBytePtr() + mOffset; }
					CData::ByteCount		getByteCount() const
												{ return mByteCount; }

											// Class methods
	static			TArray<SH264NALUInfo>	getNALUInfos(const CData& data)
												{
													// Setup
													TNArray<SH264NALUInfo>		naluInfos;

													const	UInt8*				bytePtr =
																						(const UInt8*)
																								data.getBytePtr();
															UInt32				offset = 0;
															CData::ByteCount	bytesRemaining = data.getByteCount();
													while (bytesRemaining > 0) {
														// Get NALU size
														CData::ByteCount	byteCount =
																					EndianU32_BtoN(
																							*((const UInt32*) bytePtr));
														bytePtr += sizeof(UInt32);
														offset += sizeof(UInt32);
														bytesRemaining -= sizeof(UInt32);

														// Add NALU
														naluInfos += SH264NALUInfo(data, offset, byteCount);

														// Update
														bytePtr += byteCount;
														offset += (UInt32) byteCount;
														bytesRemaining -= byteCount;
													}

													return naluInfos;
												}
	static			CData					composeAnnexB(const TArray<SH264NALUInfo>& spsNALUInfos,
													const TArray<SH264NALUInfo>& ppsNALUInfos,
													const TArray<SH264NALUInfo>& naluInfos)
												{
													// Setup
													CData	data;

													// Add SPS
													for (TIteratorD<SH264NALUInfo> iterator =
																	spsNALUInfos.getIterator();
															iterator.hasValue(); iterator.advance())
														data +=
																mAnnexBMarker +
																		CData(iterator->getBytePtr(),
																				iterator->getByteCount(), false);

													// Add PPS
													for (TIteratorD<SH264NALUInfo> iterator =
																	ppsNALUInfos.getIterator();
															iterator.hasValue(); iterator.advance())
														data +=
																mAnnexBMarker +
																		CData(iterator->getBytePtr(),
																				iterator->getByteCount(), false);

													// Add NALUs
													for (TIteratorD<SH264NALUInfo> iterator = naluInfos.getIterator();
															iterator.hasValue(); iterator.advance())
														data +=
																mAnnexBMarker +
																		CData(iterator->getBytePtr(),
																				iterator->getByteCount(), false);

													return data;
												}

	// Properties
	private:
				CData				mData;
				UInt32				mOffset;
				CData::ByteCount	mByteCount;

		static	CData				mAnnexBMarker;

};
CData	SH264NALUInfo::mAnnexBMarker = CData::fromBase64String(CString(OSSTR("AAAAAQ==")));

// SH264SequenceParameterSetPayload
struct SH264SequenceParameterSetPayload {
	// Lifecycle methods
	SH264SequenceParameterSetPayload(const CData& data)
		{
			// Setup
			CBitReader	bitReader(I<CRandomAccessDataSource>(new CDataDataSource(data)), true);

			// NAL header
			mForbiddenZero = *bitReader.readUInt8(1);
			mNALRef = *bitReader.readUInt8(2);
			mNALUnitType = (SH264NALUInfo::Type) *bitReader.readUInt8(5);

			// Profile
			mProfile = *bitReader.readUInt8();

			// Constratins
			mConstraintSet0Flag = *bitReader.readUInt8(1);
			mConstraintSet1Flag = *bitReader.readUInt8(1);
			mConstraintSet2Flag = *bitReader.readUInt8(1);
			mConstraintSet3Flag = *bitReader.readUInt8(1);
			mConstraintSet4Flag = *bitReader.readUInt8(1);
			mConstraintSet5Flag = *bitReader.readUInt8(1);
			mReserved2Bits = *bitReader.readUInt8(2);

			// Level
			mLevel = *bitReader.readUInt8();

			// SPS ID
			mSPSID = (UInt8) *bitReader.readUEExpGolombCode();

			// Check profile
			if ((mProfile == 44) || (mProfile == 83) || (mProfile == 86) || (mProfile == 100) || (mProfile == 110) ||
					(mProfile == 118) || (mProfile == 122) ||	(mProfile == 128) || (mProfile == 244)) {
				// Read high profile chroma format info
				UInt8	chromaFormatIDC = *bitReader.readUEExpGolombCode();
				if (chromaFormatIDC == 3)
					// Read color plane flag
					bitReader.readUInt8(1);

				UInt8	bitDepthLumaMinus8 = *bitReader.readUEExpGolombCode();	(void) bitDepthLumaMinus8;
				UInt8	bitDepthChromaMinus8 = *bitReader.readUEExpGolombCode();  (void) bitDepthChromaMinus8;
				UInt8	qpprimeYZeroTransformBypassFlag = *bitReader.readUInt8(1); (void) qpprimeYZeroTransformBypassFlag;

				UInt8	seqScalingMatrixPresentFlag = *bitReader.readUInt8(1);
				if (seqScalingMatrixPresentFlag) {
					// Read
					UInt8	count  = (chromaFormatIDC != 3) ? 8 : 12;
					for (UInt8 i = 0; i < count; i++) {
						// Check if seq scaling list is present
						if (*bitReader.readUInt8(1)) {
							// Read
							UInt8	listSize = (i < 6) ? 16 : 64;
							UInt8	last = 8, next = 8;
							for (UInt8 j = 0; j < listSize; j++) {
								// Check next
								if (next != 0) {
									// Read
									SInt8 delta = *bitReader.readSEExpGolombCode();

									// Uupdate
									next = (last + delta + 256) % 256;
								}

								// Update
								last = (next == 0) ? last : next;
							}
						}
					}
				}
			}

			mFrameNumberBitCount = (UInt8) *bitReader.readUEExpGolombCode() + 4;

			mPicOrderCountType = (UInt8) *bitReader.readUEExpGolombCode();
			if (mPicOrderCountType == 0)
				// Type 0, read lsb bit count
				mPicOrderCountLSBBitCount = (UInt8) *bitReader.readUEExpGolombCode() + 4;
			else if (mPicOrderCountType == 1) {
				// Type 1
				UInt8	deltaPicorderAlwaysZeroFlag = *bitReader.readUInt8(1);	(void) deltaPicorderAlwaysZeroFlag;
				UInt8	offsetForNonRefPic = *bitReader.readSEExpGolombCode();	(void) offsetForNonRefPic;
				UInt8	offsetForTopToBottomField = *bitReader.readSEExpGolombCode();	(void) offsetForTopToBottomField;
				UInt32	numRefFramesInPicOrderCntCycle = *bitReader.readUEExpGolombCode();
				for (UInt32 i = 0; i < numRefFramesInPicOrderCntCycle; i++) {
					// Read offset
					UInt32	offsetForRefFrame = *bitReader.readSEExpGolombCode();	(void) offsetForRefFrame;
				}
			} else
				// n/a
				mPicOrderCountLSBBitCount = 0;

			UInt32	maxNumRefFrames = *bitReader.readUEExpGolombCode();	(void) maxNumRefFrames;
			UInt8	gapsInFrameNumValueAllowedFlag = *bitReader.readUInt8(1);	(void) gapsInFrameNumValueAllowedFlag;
			UInt32	picWidthInMBSMinus1 = *bitReader.readUEExpGolombCode();	(void) picWidthInMBSMinus1;
			UInt32	picHeightInMapUnitsMinus1 = *bitReader.readUEExpGolombCode();	(void) picHeightInMapUnitsMinus1;

			mFrameMbsOnlyFlag = *bitReader.readUInt8(1);
		}

	// Properties
	UInt8				mForbiddenZero;
	UInt8				mNALRef;
	SH264NALUInfo::Type	mNALUnitType;
	UInt8				mProfile;
	UInt8				mConstraintSet0Flag : 1;
	UInt8				mConstraintSet1Flag : 1;
	UInt8				mConstraintSet2Flag : 1;
	UInt8				mConstraintSet3Flag : 1;
	UInt8				mConstraintSet4Flag : 1;
	UInt8				mConstraintSet5Flag : 1;
	UInt8				mReserved2Bits : 2;
	UInt8				mLevel;
	UInt8				mSPSID;
	UInt8				mFrameNumberBitCount;
	UInt8				mPicOrderCountType;
	UInt8				mPicOrderCountLSBBitCount;
	UInt8				mFrameMbsOnlyFlag;
};

enum ESliceType {
	kSliceTypeP		= 0,
	kSliceTypeB		= 1,
	kSliceTypeI		= 2,
	kSliceTypeSP	= 3,
	kSliceTypeSI	= 4,
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CH264DecodeVideoCodec

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
class CH264DecodeVideoCodec : public CCoreMediaDecodeVideoCodec {
#elif defined(TARGET_OS_WINDOWS)
class CH264DecodeVideoCodec : public CMediaFoundationDecodeVideoCodec {
#endif
	// SPSPPSInfo
	public:
		struct SPSPPSInfo {
											// Lifecycle methods
											SPSPPSInfo(const TArray<SH264NALUInfo>& spsNALUInfos,
													const TArray<SH264NALUInfo>& ppsNALUInfos) :
												mSPSNALUInfos(spsNALUInfos), mPPSNALUInfos(ppsNALUInfos)
												{}
											SPSPPSInfo(const SPSPPSInfo& other) :
												mSPSNALUInfos(other.mSPSNALUInfos),
														mPPSNALUInfos(other.mPPSNALUInfos)
												{}

											// Instance methods
			const	TArray<SH264NALUInfo>&	getSPSNALUInfos() const
												{ return mSPSNALUInfos; }
			const	TArray<SH264NALUInfo>&	getPPSNALUInfos() const
												{ return mPPSNALUInfos; }

			// Properties
			private:
				TArray<SH264NALUInfo>	mSPSNALUInfos;
				TArray<SH264NALUInfo>	mPPSNALUInfos;
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
					Times(const Times& other) :
						mDecodeTime(other.mDecodeTime), mPresentationTime(other.mPresentationTime)
						{}

					// Properties
					UInt64	mDecodeTime;
					UInt64	mPresentationTime;
				};

			// Methods
			public:
								// Lifecycle methods
								FrameTiming(const SH264SequenceParameterSetPayload& spsPayload) :
									mCurrentSPSID(spsPayload.mSPSID),
											mFrameNumberBitCount(spsPayload.mFrameNumberBitCount),
											mPicOrderCountType(spsPayload.mPicOrderCountType),
											mPicOrderCountLSBBitCount(spsPayload.mPicOrderCountLSBBitCount),
											mMaxPicOrderCntLSB(1ULL << spsPayload.mPicOrderCountLSBBitCount),
											mPicOrderCountDivisor(0),
											mFrameMbsOnlyFlag(spsPayload.mFrameMbsOnlyFlag),
											mLastIDRFrameTime(0),
											mCurrentFrameTime(0),
											mNextFrameTime(0),
											mHasReference(false)
									{
										// Initialize POC state
										resetPOCState();
									}

								// Instance methods
				void			seek(UInt64 frameTime)
									{
										// Update
										mNextFrameTime = frameTime;

										// Reset POC state
										resetPOCState();
									}
				TVResult<Times>	updateFrom(const CMediaPacketSource::DataInfo& dataInfo);
				void			updateSPS(const SH264SequenceParameterSetPayload& spsPayload)
									{
										// Check if SPS actually changed
										if ((spsPayload.mSPSID != mCurrentSPSID) ||
												(spsPayload.mPicOrderCountType != mPicOrderCountType) ||
												(spsPayload.mFrameNumberBitCount != mFrameNumberBitCount) ||
												(spsPayload.mPicOrderCountLSBBitCount != mPicOrderCountLSBBitCount)) {
											// Update SPS parameters
											mCurrentSPSID = spsPayload.mSPSID;

											mFrameNumberBitCount = spsPayload.mFrameNumberBitCount;

											mPicOrderCountType = spsPayload.mPicOrderCountType;
											mPicOrderCountLSBBitCount = spsPayload.mPicOrderCountLSBBitCount;
											mMaxPicOrderCntLSB = 1ULL << spsPayload.mPicOrderCountLSBBitCount;
											mPicOrderCountDivisor = 0;

											mFrameMbsOnlyFlag = spsPayload.mFrameMbsOnlyFlag;

											// Reset POC state
											resetPOCState();
										}
									}

			private:
								// Instance methods
				UInt64			calculatePOCType0(bool isIDR, UInt64 picOrderCntLSB, bool isReference);
				UInt64			calculatePOCType1(bool isIDR, UInt64 frameNum, bool isReference);
				UInt64			calculatePOCType2(bool isIDR, UInt64 frameNum);

				void			resetPOCState()
									{
										// Reset
										mPOCState.mType0.mPicOrderCountMSB = 0;
										mPOCState.mType0.mPreviousPicOrderCountLSB = 0;
										mPOCState.mType0.mPreviousPicOrderCountMSB = 0;

										mPOCState.mType1.mPreviousFrameNum = 0;
										mPOCState.mType1.mPreviousFrameNumOffset = 0;
										mPOCState.mType1.mFrameNumOffset = 0;

										mPOCState.mType2.mPreviousFrameNum = 0;
										mPOCState.mType2.mPreviousFrameNumOffset = 0;
										mPOCState.mType2.mFrameNumOffset = 0;

										mHasReference = false;
									}

			// Properties
			private:
				// Current SPS tracking
				UInt8	mCurrentSPSID;

				// SPS parameters
				UInt8	mFrameNumberBitCount;

				UInt8	mPicOrderCountType;
				UInt8	mPicOrderCountLSBBitCount;
				UInt64	mMaxPicOrderCntLSB;
				UInt64	mPicOrderCountDivisor;

				UInt8	mFrameMbsOnlyFlag;

				// POC state
				union {
					struct {
						UInt64	mPicOrderCountMSB;
						UInt64	mPreviousPicOrderCountLSB;
						UInt64	mPreviousPicOrderCountMSB;
					} mType0;

					struct {
						UInt64	mPreviousFrameNum;
						UInt64	mPreviousFrameNumOffset;
						UInt64	mFrameNumOffset;
					} mType1;

					struct {
						UInt64	mPreviousFrameNum;
						UInt64	mPreviousFrameNumOffset;
						UInt64	mFrameNumOffset;
					} mType2;
				} mPOCState;

				// Frame timing
				UInt64	mLastIDRFrameTime;
				UInt64	mCurrentFrameTime;
				UInt64	mNextFrameTime;

				// Reference tracking
				bool	mHasReference;
		};

	// Methods
	public:
													CH264DecodeVideoCodec(
															const I<CMediaPacketSource>& mediaPacketSource,
															const CData& configurationData, UInt32 timeScale,
															const TNumberArray<UInt32>& keyframeIndexes);

				void								seek(UInt64 frameTime)
														{ mFrameTiming->seek(frameTime); }

				SPSPPSInfo							getSPSPPSInfo() const;
				UInt16								getByteCount(const UInt8* ptr) const
														{ return (*ptr << 8) | *(ptr + 1); }
				UInt32								getNALUHeaderLengthSize() const;

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
				TVResult<CMFormatDescriptionRef>	composeFormatDescription();
				TVResult<CMSampleTimingInfo>		composeSampleTimingInfo(
															const CMediaPacketSource::DataInfo& dataInfo,
															UInt32 timeScale);
#elif defined(TARGET_OS_WINDOWS)
				OV<SError>							setup(const CVideoProcessor::Format& videoProcessorFormat);

				OR<const GUID>						getGUID() const
														{ return OR<const GUID>(MFVideoFormat_H264); }
#endif

#if defined(TARGET_OS_WINDOWS)
		static	TCIResult<IMFSample>				readInputSample(
															CMediaFoundationDecodeVideoCodec&
																	mediaFoundationDecodeVideoCodec);
#endif

		I<CMediaPacketSource>	mMediaPacketSource;
		CData					mConfigurationData;
		UInt32					mTimeScale;
		TNumberArray<UInt32>	mKeyframeIndexes;

		OV<SPSPPSInfo>			mCurrentSPSPPSInfo;
		OI<FrameTiming>			mFrameTiming;
};

//----------------------------------------------------------------------------------------------------------------------
CH264DecodeVideoCodec::CH264DecodeVideoCodec(const I<CMediaPacketSource>& mediaPacketSource,
		const CData& configurationData, UInt32 timeScale, const TNumberArray<UInt32>& keyframeIndexes) :
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
		CCoreMediaDecodeVideoCodec(CH264VideoCodec::mID, mediaPacketSource, timeScale, keyframeIndexes),
#elif defined(TARGET_OS_WINDOWS)
		CMediaFoundationDecodeVideoCodec(CH264VideoCodec::mID, mediaPacketSource, timeScale, keyframeIndexes,
				readInputSample),
#endif
		mMediaPacketSource(mediaPacketSource), mConfigurationData(configurationData), mTimeScale(timeScale),
				mKeyframeIndexes(keyframeIndexes)
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
CH264DecodeVideoCodec::SPSPPSInfo CH264DecodeVideoCodec::getSPSPPSInfo() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	Configuration&			configuration = *((Configuration*) mConfigurationData.getBytePtr());
	const	UInt8*					bytePtr = &configuration.mSPSPPSInfo[0];
			UInt32					offset = sizeof(Configuration);

			TNArray<SH264NALUInfo>	spsNALUInfos;
			TNArray<SH264NALUInfo>	ppsNALUInfos;

	// Compose SPS NALUInfos
	UInt32	count = *bytePtr & 0x1F;
	bytePtr++;
	offset++;
	for (UInt32 i = 0; i < count; i++) {
		// Add SPS NALUInfo
		UInt16	byteCount = getByteCount(bytePtr);
		spsNALUInfos += SH264NALUInfo(mConfigurationData, offset + 2, byteCount);

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
		ppsNALUInfos += SH264NALUInfo(mConfigurationData, offset + 2, byteCount);

		// Update
		bytePtr += 2 + byteCount;
		offset += 2 + byteCount;
	}

	return SPSPPSInfo(spsNALUInfos, ppsNALUInfos);
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CH264DecodeVideoCodec::getNALUHeaderLengthSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	Configuration&	configuration = *((Configuration*) mConfigurationData.getBytePtr());

	return (configuration.mLengthCoded & ~0xFC) + 1;
}

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
//----------------------------------------------------------------------------------------------------------------------
TVResult<CMFormatDescriptionRef> CH264DecodeVideoCodec::composeFormatDescription()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
			SPSPPSInfo				spsppsInfo = getSPSPPSInfo();
	const	TArray<SH264NALUInfo>	spsNALUInfos = spsppsInfo.getSPSNALUInfos();
	const	TArray<SH264NALUInfo>	ppsNALUInfos = spsppsInfo.getPPSNALUInfos();
			OSStatus				status;

	// Setup format description
			CArray::ItemCount	spsCount = spsNALUInfos.getCount();
			CArray::ItemCount	ppsCount = ppsNALUInfos.getCount();
	const	uint8_t*			parameterSetPointers[spsCount + ppsCount];
			size_t				parameterSetSizes[spsCount + ppsCount];
	for (CArray::ItemIndex i = 0; i < spsCount; i++) {
		// Store
		parameterSetPointers[i] = spsNALUInfos[i].getBytePtr();
		parameterSetSizes[i] = spsNALUInfos[i].getByteCount();
	}
	for (CArray::ItemIndex i = 0; i < ppsCount; i++) {
		// Store
		parameterSetPointers[spsCount + i] = ppsNALUInfos[i].getBytePtr();
		parameterSetSizes[spsCount + i] = ppsNALUInfos[i].getByteCount();
	}

	CMFormatDescriptionRef	formatDescriptionRef;
	status =
			::CMVideoFormatDescriptionCreateFromH264ParameterSets(kCFAllocatorDefault, spsCount + ppsCount,
					parameterSetPointers, parameterSetSizes, getNALUHeaderLengthSize(),
					&formatDescriptionRef);
	ReturnValueIfFailed(status, CString(OSSTR("CMVideoFormatDescriptionCreateFromH264ParameterSets")),
			TVResult<CMFormatDescriptionRef>(SErrorFromOSStatus(status)));

	// Finish setup
	SH264SequenceParameterSetPayload	spsPayload(
												CData(parameterSetPointers[0], (CData::ByteCount) parameterSetSizes[0],
														false));
	mFrameTiming = OI<FrameTiming>(new FrameTiming(spsPayload));

	return TVResult<CMFormatDescriptionRef>(formatDescriptionRef);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CMSampleTimingInfo> CH264DecodeVideoCodec::composeSampleTimingInfo(
		const CMediaPacketSource::DataInfo& dataInfo, UInt32 timeScale)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update frame timing
	TVResult<FrameTiming::Times>	times = mFrameTiming->updateFrom(dataInfo);
	ReturnValueIfResultError(times, TVResult<CMSampleTimingInfo>(times.getError()));

	// Compose sample timing info
	CMSampleTimingInfo	sampleTimingInfo;
	sampleTimingInfo.duration = ::CMTimeMake(dataInfo.getDuration(), timeScale);
	sampleTimingInfo.decodeTimeStamp = ::CMTimeMake(times->mDecodeTime, timeScale);
	sampleTimingInfo.presentationTimeStamp = ::CMTimeMake(times->mPresentationTime, timeScale);

	return TVResult<CMSampleTimingInfo>(sampleTimingInfo);
}

#elif defined(TARGET_OS_WINDOWS)

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CH264DecodeVideoCodec::setup(const CVideoProcessor::Format& videoProcessorFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Do super
	OV<SError>	error = CMediaFoundationDecodeVideoCodec::setup(videoProcessorFormat);
	ReturnErrorIfError(error);

	// Finish setup
	mCurrentSPSPPSInfo = OV<SPSPPSInfo>(getSPSPPSInfo());

	const	SH264NALUInfo&						spsNALUInfo = mCurrentSPSPPSInfo->getSPSNALUInfos().getFirst();
			SH264SequenceParameterSetPayload	spsPayload(
														CData(spsNALUInfo.getBytePtr(), spsNALUInfo.getByteCount(),
																false));
	mFrameTiming = OI<FrameTiming>(new FrameTiming(spsPayload));

	return OV<SError>();
}
#endif

#if defined(TARGET_OS_WINDOWS)
//----------------------------------------------------------------------------------------------------------------------
TCIResult<IMFSample> CH264DecodeVideoCodec::readInputSample(
		CMediaFoundationDecodeVideoCodec& mediaFoundationDecodeVideoCodec)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CH264DecodeVideoCodec&	videoCodec = (CH264DecodeVideoCodec&) mediaFoundationDecodeVideoCodec;

	// Get next packet
	TVResult<CMediaPacketSource::DataInfo>	dataInfo = videoCodec.mMediaPacketSource->readNext();
	ReturnValueIfResultError(dataInfo, TCIResult<IMFSample>(dataInfo.getError()));

	// Update frame timing
	TVResult<FrameTiming::Times>	times = videoCodec.mFrameTiming->updateFrom(*dataInfo);
	ReturnValueIfResultError(dataInfo, TCIResult<IMFSample>(times.getError()));

	// Create input sample
	TArray<SH264NALUInfo>	naluInfos = SH264NALUInfo::getNALUInfos(dataInfo->getData());
	CData					annexBData =
									SH264NALUInfo::composeAnnexB(
											videoCodec.mCurrentSPSPPSInfo->getSPSNALUInfos(),
											videoCodec.mCurrentSPSPPSInfo->getPPSNALUInfos(), naluInfos);
	TCIResult<IMFSample>				sample = CMediaFoundationServices::createSample(annexBData);
	ReturnValueIfResultError(sample, sample);

	HRESULT	result = sample.getInstance()->SetSampleTime(times->mPresentationTime * 10000 / videoCodec.getTimeScale());
	ReturnValueIfFailed(result, CString(OSSTR("SetSampleTime")), TCIResult<IMFSample>(SErrorFromHRESULT(result)));

	result = sample.getInstance()->SetSampleDuration(dataInfo->getDuration());
	ReturnValueIfFailed(result, CString(OSSTR("SetSampleDuration")), TCIResult<IMFSample>(SErrorFromHRESULT(result)));

	return sample;
}
#endif

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
TVResult<CH264DecodeVideoCodec::FrameTiming::Times> CH264DecodeVideoCodec::FrameTiming::updateFrom(
		const CMediaPacketSource::DataInfo& dataInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CBitReader	bitReader(I<CRandomAccessDataSource>(new CDataDataSource(dataInfo.getData())), true);

	// Iterate NALUs to find the first slice
	bool	isIDR = false;
	bool	isReference = false;
	UInt64	frameNum = 0;
	UInt64	picOrderCntLSB = 0;
	ESliceType	sliceType = (ESliceType) -1;
	while (true) {
		// Get NALU info
		UInt32	size = *bitReader.readUInt32();
		UInt64	pos = bitReader.getPos();

		// Read NALU header
		UInt8				forbiddenZeroBit = *bitReader.readUInt8(1);	(void) forbiddenZeroBit;
		UInt8				nalRefIDC = *bitReader.readUInt8(2);
		SH264NALUInfo::Type	naluType = (SH264NALUInfo::Type) *bitReader.readUInt8(5);

		// Check if this is a reference picture (nal_ref_idc != 0)
		isReference = (nalRefIDC != 0);

		// Note: Slices reference PPS via pic_parameter_set_id, and PPS references SPS via seq_parameter_set_id.
		// For a complete implementation, we would need to:
		// 1. Parse and store all PPS NALUs with their seq_parameter_set_id
		// 2. When we see a slice, look up its PPS to find which SPS is active
		// 3. Update SPS parameters if the active SPS changed
		// For now, we assume the SPS that appears in the stream is the active one.

		// Check type
		bool	foundSPS = false;
		UInt8	activeSPSID = 0xFF;
		if (naluType == SH264NALUInfo::kTypeSequenceParameterSet) {
			// Found an SPS - parse it to check if parameters changed
			// This is important for handling SPS changes mid-stream
			foundSPS = true;

			// Update SPS
			bitReader.setPos(CBitReader::kPositionFromBeginning, pos);
			updateSPS(SH264SequenceParameterSetPayload(*bitReader.readData(size)));
		} else if (naluType == SH264NALUInfo::kTypeCodedSliceNonIDRPicture) {
			// Non-IDR slice
			UInt32	firstMBInSlice = *bitReader.readUEExpGolombCode();	(void) firstMBInSlice;
			sliceType = (ESliceType) *bitReader.readUEExpGolombCode();
			activeSPSID = (UInt8) *bitReader.readUEExpGolombCode();
			frameNum = *bitReader.readUInt64(mFrameNumberBitCount);

			// Check pic order type type
			if (mPicOrderCountType == 0)
				// Type 0
				picOrderCntLSB = *bitReader.readUInt64(mPicOrderCountLSBBitCount);

			isIDR = false;
			break;
		} else if (naluType == SH264NALUInfo::kTypeCodedSliceIDRPicture) {
			// IDR slice
			UInt32	firstMBInSlice = *bitReader.readUEExpGolombCode();	(void) firstMBInSlice;
			sliceType = (ESliceType) *bitReader.readUEExpGolombCode();
			activeSPSID = (UInt8) *bitReader.readUEExpGolombCode();
			frameNum = *bitReader.readUInt64(mFrameNumberBitCount);
			UInt32	idrPicID = *bitReader.readUEExpGolombCode();	(void) idrPicID;

			// Check pic order type type
			if (mPicOrderCountType == 0)
				// Type 0
				picOrderCntLSB = *bitReader.readUInt64(mPicOrderCountLSBBitCount);

			isIDR = true;
			isReference = true;
			break;
		} else if (naluType == SH264NALUInfo::kTypeSupplementalEnhancementInformation) {
			// SEI
		} else if (naluType == SH264NALUInfo::kTypePictureParameterSet) {
			// PPS - could parse to track PPS->SPS mapping, but skip for now
		} else
			// Unhandled
			CLogServices::logMessage(CString(OSSTR("Unhandled NALU type: ")) + CString(naluType));

		// Next NALU
		OV<SError>	error = bitReader.setPos(CBitReader::kPositionFromBeginning, pos + size);
		LogIfErrorAndReturnValue(error, CString(OSSTR("reading next NALU")), TVResult<Times>(*error));
	}

	// Calculate POC
	UInt64 picOrderCnt;
	switch (mPicOrderCountType) {
		case 0:
			// Type 0: pic_order_cnt_lsb based
			picOrderCnt = calculatePOCType0(isIDR, picOrderCntLSB, isReference);
			break;

		case 1:
			// Type 1: frame_num based with offset
			picOrderCnt = calculatePOCType1(isIDR, frameNum, isReference);
			break;

		case 2:
			// Type 2: simple frame_num based (pictures in display order)
			picOrderCnt = calculatePOCType2(isIDR, frameNum);
			break;

		default:
			// Unsupported
			CLogServices::logError(CString(OSSTR("Unsupported mPicOrderCountType: ")) + CString(mPicOrderCountType));

			return TVResult<Times>(
					SError(CString(OSSTR("CH264DecodeVideoCodec::FrameTiming")), 1,
							CString(OSSTR("Unsupported mPicOrderCountType: ")) + CString(mPicOrderCountType)));
	}

	// Check if is IDR
	if (isIDR)
		// Update last IDR time
		mLastIDRFrameTime = mNextFrameTime;

	// Calculate presentation time from POC
	// The correct formula depends on frame_mbs_only_flag from SPS:
	//
	// If frame_mbs_only_flag == 1 (progressive/frame coding):
	//   - POC should increment by 2 per frame: 0, 2, 4, 6...
	//   - Frame index = POC / 2
	//
	// If frame_mbs_only_flag == 0 (interlaced/field coding possible):
	//   - POC increments by 1 per field: 0, 1, 2, 3...
	//   - Each frame consists of 2 fields (top and bottom)
	//   - Frame index = POC / 2
	//
	// HOWEVER, some encoders break the spec and use POC type 0 with POC incrementing
	// by 1 even for progressive content (treating each frame as if it's a field).
	// In this case, frame index = POC directly.
	//
	// We can detect this by checking if frame_mbs_only_flag == 1 but POC increments by 1.
	UInt64	frameIndex;
	if (mFrameMbsOnlyFlag == 1) {
		// Progressive coding - POC should increment by 2
		if (mPicOrderCountDivisor > 0)
			// Have figured the divisor
			frameIndex = picOrderCnt / mPicOrderCountDivisor;
		else if (picOrderCnt > 0) {
			// Can set the divisor
			mPicOrderCountDivisor = picOrderCnt;
			frameIndex = 1;
		} else
			// First frame
			frameIndex = 0;
	} else
		// Field coding - POC increments by 1, divide by 2 to get frame index
		frameIndex = picOrderCnt / 2;

	mCurrentFrameTime = mLastIDRFrameTime + frameIndex * dataInfo.getDuration();

	// Compose results
	// Decode time: order packets arrive (sequential)
	// Presentation time: order frames should be displayed (based on POC)
	TVResult<Times>	times = TVResult<Times>(Times(mNextFrameTime, mCurrentFrameTime));

	// Update decode time for next frame
	mNextFrameTime += dataInfo.getDuration();

	// Check if is reference
	if (isReference)
		// Has reference
		mHasReference = true;

	return times;
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CH264DecodeVideoCodec::FrameTiming::calculatePOCType0(bool isIDR, UInt64 picOrderCntLSB, bool isReference)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt64 picOrderCnt;

	// Check if is IDR
	if (isIDR) {
		// IDR frame - reset POC tracking
		mPOCState.mType0.mPicOrderCountMSB = 0;
		mPOCState.mType0.mPreviousPicOrderCountLSB = 0;
		mPOCState.mType0.mPreviousPicOrderCountMSB = 0;

		picOrderCnt = 0;
	} else {
		// Non-IDR, calculate PicOrderCntMsb as per H.264 spec 8.2.1.1
		if ((picOrderCntLSB < mPOCState.mType0.mPreviousPicOrderCountLSB) &&
				((mPOCState.mType0.mPreviousPicOrderCountLSB - picOrderCntLSB) >= (mMaxPicOrderCntLSB / 2)))
			// Wrapped forward
			mPOCState.mType0.mPicOrderCountMSB = mPOCState.mType0.mPreviousPicOrderCountMSB + mMaxPicOrderCntLSB;
		else if ((picOrderCntLSB > mPOCState.mType0.mPreviousPicOrderCountLSB) &&
			   ((picOrderCntLSB - mPOCState.mType0.mPreviousPicOrderCountLSB) > (mMaxPicOrderCntLSB / 2)))
			// Wrapped backward
			mPOCState.mType0.mPicOrderCountMSB = mPOCState.mType0.mPreviousPicOrderCountMSB - mMaxPicOrderCntLSB;
		else
			// No wrap
			mPOCState.mType0.mPicOrderCountMSB = mPOCState.mType0.mPreviousPicOrderCountMSB;

		// TopFieldOrderCnt for frame coding
		picOrderCnt = mPOCState.mType0.mPicOrderCountMSB + picOrderCntLSB;
	}

	// Check if reference
	if (isReference) {
		// Update state for next frame
		mPOCState.mType0.mPreviousPicOrderCountLSB = picOrderCntLSB;
		mPOCState.mType0.mPreviousPicOrderCountMSB = mPOCState.mType0.mPicOrderCountMSB;
	}

	return picOrderCnt;
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CH264DecodeVideoCodec::FrameTiming::calculatePOCType1(bool isIDR, UInt64 frameNum, bool isReference)
//----------------------------------------------------------------------------------------------------------------------
{
	// POC Type 1 uses frame_num and FrameNumOffset
	// This is a simplified implementation - full support requires SPS fields like
	// num_ref_frames_in_pic_order_cnt_cycle, offset_for_ref_frame[], etc.
	UInt64 maxFrameNum = 1ULL << mFrameNumberBitCount;

	// Check if is IDR
	if (isIDR) {
		// IDR frame - reset POC tracking
		mPOCState.mType1.mFrameNumOffset = 0;
		mPOCState.mType1.mPreviousFrameNum = 0;
	} else
		// Non-IDR, calculate FrameNumOffset
		mPOCState.mType1.mFrameNumOffset =
				(mPOCState.mType1.mPreviousFrameNum > frameNum) ?
						mPOCState.mType1.mPreviousFrameNumOffset + maxFrameNum :
						mPOCState.mType1.mPreviousFrameNumOffset;

	// Simplified POC calculation for type 1.  In practice, this requires additional SPS parameters
	UInt64 picOrderCnt = (mPOCState.mType1.mFrameNumOffset + frameNum) * 2;

	// Check if reference
	if (isReference) {
		// Update state for next frame
		mPOCState.mType1.mPreviousFrameNum = frameNum;
		mPOCState.mType1.mPreviousFrameNumOffset = mPOCState.mType1.mFrameNumOffset;
	}

	return picOrderCnt;
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CH264DecodeVideoCodec::FrameTiming::calculatePOCType2(bool isIDR, UInt64 frameNum)
//----------------------------------------------------------------------------------------------------------------------
{
	// POC Type 2: Simple frame number based ordering
	UInt64 maxFrameNum = 1ULL << mFrameNumberBitCount;

	// Check if is IDR
	if (isIDR) {
		// IDR frame - reset POC tracking
		mPOCState.mType2.mFrameNumOffset = 0;
		mPOCState.mType2.mPreviousFrameNum = frameNum;

		return 0;
	}

	// Calculate FrameNumOffset
	mPOCState.mType2.mFrameNumOffset =
			(mPOCState.mType2.mPreviousFrameNum > frameNum) ?
					mPOCState.mType2.mPreviousFrameNumOffset + maxFrameNum :
					mPOCState.mType2.mPreviousFrameNumOffset;

	// Pictures are in display order
	UInt64 tempPicOrderCnt = 2 * (mPOCState.mType2.mFrameNumOffset + frameNum);

	// Update state
	mPOCState.mType2.mPreviousFrameNum = frameNum;
	mPOCState.mType2.mPreviousFrameNumOffset = mPOCState.mType2.mFrameNumOffset;

	return tempPicOrderCnt;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CH264VideoCodec

// MARK: Properties

const	OSType	CH264VideoCodec::mID = MAKE_OSTYPE('h', '2', '6', '4');
const	CString	CH264VideoCodec::mName(OSSTR("h.264"));

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
SVideo::Format CH264VideoCodec::composeVideoTrackFormat(const S2DSizeU16& frameSize, Float32 frameRate)
//----------------------------------------------------------------------------------------------------------------------
{
	return SVideo::Format(mID, frameSize, frameRate);
}

//----------------------------------------------------------------------------------------------------------------------
I<CDecodeVideoCodec> CH264VideoCodec::create(const I<CRandomAccessDataSource>& randomAccessDataSource,
		const TArray<SMedia::PacketAndLocation>& packetAndLocations, const CData& configurationData, UInt32 timeScale,
		const TNumberArray<UInt32>& keyframeIndexes)
//----------------------------------------------------------------------------------------------------------------------
{
	return I<CDecodeVideoCodec>(
			new CH264DecodeVideoCodec(
					I<CMediaPacketSource>(
							new CSeekableVaryingMediaPacketSource(randomAccessDataSource, packetAndLocations)),
					configurationData, timeScale, keyframeIndexes));
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Declare video codecs

REGISTER_VIDEO_CODEC(h264, CCodec::Info(CH264VideoCodec::mID, CH264VideoCodec::mName));
