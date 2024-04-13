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
	#include "CMediaFoundationVideoCodec.h"
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

			// Decode
			mForbiddenZero = *bitReader.readUInt8(1);
			mNALRef = *bitReader.readUInt8(2);
			mNALUnitType = (SH264NALUInfo::Type) *bitReader.readUInt8(5);
			mProfile = *bitReader.readUInt8();
			mConstraintSet0Flag = *bitReader.readUInt8(1);
			mConstraintSet1Flag = *bitReader.readUInt8(1);
			mConstraintSet2Flag = *bitReader.readUInt8(1);
			mConstraintSet3Flag = *bitReader.readUInt8(1);
			mConstraintSet4Flag = *bitReader.readUInt8(1);
			mConstraintSet5Flag = *bitReader.readUInt8(1);
			mReserved2Bits = *bitReader.readUInt8(2);
			mLevel = *bitReader.readUInt8();
			mSPSID = (UInt8) *bitReader.readUEColumbusCode();
			mFrameNumberBitCount = (UInt8) *bitReader.readUEColumbusCode() + 4;
			mPicOrderCountType = (UInt8) *bitReader.readUEColumbusCode();
			mPicOrderCountLSBBitCount = (UInt8) *bitReader.readUEColumbusCode() + 4;
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
									mCurrentFrameNumberBitCount(spsPayload.mFrameNumberBitCount),
											mCurrentPicOrderCountLSBBitCount(spsPayload.mPicOrderCountLSBBitCount),
											mPicOrderCountMSBChangeThreshold(
													1 << (mCurrentPicOrderCountLSBBitCount - 1)),
											mPicOrderCountMSB(0), mPreviousPicOrderCountLSB(0), mLastIDRFrameTime(0),
													mCurrentFrameTime(0), mNextFrameTime(0)
									{}

								// Instance methods
				void			seek(UInt64 frameTime)
									{ mNextFrameTime = frameTime; }
				TVResult<Times>	updateFrom(const CMediaPacketSource::DataInfo& dataInfo);

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
	ReturnValueIfFailed(status, OSSTR("CMVideoFormatDescriptionCreateFromH264ParameterSets"),
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
	ReturnValueIfFailed(result, OSSTR("SetSampleTime"), TCIResult<IMFSample>(SErrorFromHRESULT(result)));

	result = sample.getInstance()->SetSampleDuration(dataInfo->getDuration());
	ReturnValueIfFailed(result, OSSTR("SetSampleDuration"), TCIResult<IMFSample>(SErrorFromHRESULT(result)));

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
	UInt8		sliceType;
	UInt8		picOrderCntLSB;

	// Iterate NALUs
	while (true) {
		// Get info
		OV<UInt32>			size = *bitReader.readUInt32();
		UInt64				pos = bitReader.getPos();

		OV<UInt8>			forbidden_zero_bit = *bitReader.readUInt8(1);	(void) forbidden_zero_bit;
		OV<UInt8>			nal_ref_idc = *bitReader.readUInt8(2);	(void) nal_ref_idc;
		OV<UInt8>			nal_unit_type = *bitReader.readUInt8(5);
		SH264NALUInfo::Type	naluType = (SH264NALUInfo::Type) *nal_unit_type;

		// Check type
		if (naluType == SH264NALUInfo::kTypeCodedSliceNonIDRPicture) {
			// Coded Slice Non-IDR Picture
			OV<UInt32>	first_mb_in_slice = *bitReader.readUEColumbusCode();	(void) first_mb_in_slice;
			OV<UInt32>	slice_type = *bitReader.readUEColumbusCode();
			OV<UInt32>	pic_parameter_set_id = *bitReader.readUEColumbusCode();	(void) pic_parameter_set_id;
			OV<UInt8>	frame_num = *bitReader.readUInt8(mCurrentFrameNumberBitCount);	(void) frame_num;
			OV<UInt8>	pic_order_cnt_lsb = *bitReader.readUInt8(mCurrentPicOrderCountLSBBitCount);

			sliceType = (UInt8) *slice_type;
			picOrderCntLSB = *pic_order_cnt_lsb;
			break;
		} else if (naluType == SH264NALUInfo::kTypeCodedSliceIDRPicture) {
			// Coded Slice IDR Picture
			OV<UInt32>	first_mb_in_slice = *bitReader.readUEColumbusCode();	(void) first_mb_in_slice;
			OV<UInt32>	slice_type = *bitReader.readUEColumbusCode();
			OV<UInt32>	pic_parameter_set_id = *bitReader.readUEColumbusCode();	(void) pic_parameter_set_id;
			OV<UInt8>	frame_num = *bitReader.readUInt8(mCurrentFrameNumberBitCount);	(void) frame_num;
			OV<UInt32>	idr_pic_id = *bitReader.readUEColumbusCode();	(void) idr_pic_id;
			OV<UInt8>	pic_order_cnt_lsb = *bitReader.readUInt8(mCurrentPicOrderCountLSBBitCount);

			sliceType = (UInt8) *slice_type;
			picOrderCntLSB = *pic_order_cnt_lsb;
			break;
		} else if (naluType == SH264NALUInfo::kTypeSupplementalEnhancementInformation) {
			// SEI
		} else
			// Unhandled
			CLogServices::logMessage(CString("Unhandled NALU type: ") + CString(naluType));

		// Next NALU
		OV<SError>	error = bitReader.setPos(CBitReader::kPositionFromBeginning, pos + *size);
		LogIfErrorAndReturnValue(error, "reading next NALU", TVResult<Times>(*error));
	}

	// Handle results
	if ((sliceType % 5) == kSliceTypeI) {
		// IDR
		mCurrentFrameTime = mNextFrameTime;
		mPicOrderCountMSB = 0;
		mPreviousPicOrderCountLSB = 0;
		mLastIDRFrameTime = mNextFrameTime;
	} else {
		// Non-IDR
		if ((picOrderCntLSB > mPreviousPicOrderCountLSB) &&
				((picOrderCntLSB - mPreviousPicOrderCountLSB) > mPicOrderCountMSBChangeThreshold))
			// Update
			mPicOrderCountMSB -= (UInt64) 1 << mCurrentPicOrderCountLSBBitCount;
		else if ((mPreviousPicOrderCountLSB > picOrderCntLSB) &&
				((mPreviousPicOrderCountLSB - picOrderCntLSB) > mPicOrderCountMSBChangeThreshold))
			// Update
			mPicOrderCountMSB += (UInt64) 1 << mCurrentPicOrderCountLSBBitCount;

		// Update
		mPreviousPicOrderCountLSB = picOrderCntLSB;
		mCurrentFrameTime = mLastIDRFrameTime + (mPicOrderCountMSB + picOrderCntLSB) / 2 * dataInfo.getDuration();
	}

	// Compose results
	TVResult<Times>	times = TVResult<Times>(Times(mNextFrameTime, mCurrentFrameTime));

	// Update
	mNextFrameTime += dataInfo.getDuration();

	return times;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CH264VideoCodec

// MARK: Properties

const	OSType	CH264VideoCodec::mID = MAKE_OSTYPE('h', '2', '6', '4');
const	CString	CH264VideoCodec::mName(OSSTR("h.264"));

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
SVideo::Format CH264VideoCodec::composeVideoTrackFormat(const S2DSizeU16& frameSize, Float32 framerate)
//----------------------------------------------------------------------------------------------------------------------
{
	return SVideo::Format(mID, frameSize, framerate);
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
