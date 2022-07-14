//----------------------------------------------------------------------------------------------------------------------
//	CH264VideoCodec.cpp			©2021 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CH264VideoCodec.h"

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
// MARK: Local data

static	CData	sAnnexBMarker(CString(OSSTR("AAAAAQ==")));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CH264DecodeVideoCodec

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
class CH264DecodeVideoCodec : public CCoreMediaDecodeVideoCodec {
#elif defined(TARGET_OS_WINDOWS)
class CH264DecodeVideoCodec : public CMediaFoundationDecodeVideoCodec {
#endif
	public:
															CH264DecodeVideoCodec(
																	const I<CMediaPacketSource>& mediaPacketSource,
																	const CData& configurationData, UInt32 timeScale,
																	const TNumericArray<UInt32>& keyframeIndexes);

						void								seek(UInt64 frameTime)
																{ mFrameTiming->seek(frameTime); }

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
						TVResult<CMFormatDescriptionRef>	composeFormatDescription();
						TVResult<CMSampleTimingInfo>		composeSampleTimingInfo(
																	const CMediaPacketSource::DataInfo& dataInfo,
																	UInt32 timeScale);
#elif defined(TARGET_OS_WINDOWS)
						OI<SError>							setup(const SVideoProcessingFormat& videoProcessingFormat);

				const	GUID&								getGUID() const
																{ return MFVideoFormat_H264; }
#endif

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
#elif defined(TARGET_OS_WINDOWS)
		static			TCIResult<IMFSample>				readInputSample(
																	CMediaFoundationDecodeVideoCodec&
																			mediaFoundationDecodeVideoCodec);
#endif

		CH264VideoCodec::DecodeInfo					mDecodeInfo;

		OI<CH264VideoCodec::DecodeInfo::SPSPPSInfo>	mCurrentSPSPPSInfo;
		OI<CH264VideoCodec::FrameTiming>			mFrameTiming;
};

//----------------------------------------------------------------------------------------------------------------------
CH264DecodeVideoCodec::CH264DecodeVideoCodec(const I<CMediaPacketSource>& mediaPacketSource,
		const CData& configurationData, UInt32 timeScale, const TNumericArray<UInt32>& keyframeIndexes) :
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
		CCoreMediaDecodeVideoCodec(CH264VideoCodec::mID, mediaPacketSource, timeScale, keyframeIndexes),
#elif defined(TARGET_OS_WINDOWS)
		CMediaFoundationDecodeVideoCodec(CH264VideoCodec::mID, mediaPacketSource, timeScale, keyframeIndexes,
				readInputSample),
#endif
		mDecodeInfo(mediaPacketSource, configurationData, timeScale, keyframeIndexes)
//----------------------------------------------------------------------------------------------------------------------
{
}

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
//----------------------------------------------------------------------------------------------------------------------
TVResult<CMFormatDescriptionRef> CH264DecodeVideoCodec::composeFormatDescription()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
			CH264VideoCodec::DecodeInfo::SPSPPSInfo	spsppsInfo = mDecodeInfo.getSPSPPSInfo();
	const	TArray<CH264VideoCodec::NALUInfo>		spsNALUInfos = spsppsInfo.getSPSNALUInfos();
	const	TArray<CH264VideoCodec::NALUInfo>		ppsNALUInfos = spsppsInfo.getPPSNALUInfos();
			OSStatus								status;

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
					parameterSetPointers, parameterSetSizes, mDecodeInfo.getNALUHeaderLengthSize(),
					&formatDescriptionRef);
	ReturnValueIfFailed(status, OSSTR("CMVideoFormatDescriptionCreateFromH264ParameterSets"),
			TVResult<CMFormatDescriptionRef>(SErrorFromOSStatus(status)));

	// Finish setup
	CH264VideoCodec::SequenceParameterSetPayload	spsPayload(
															CData(parameterSetPointers[0],
																	(CData::ByteCount) parameterSetSizes[0], false));
	mFrameTiming = OI<CH264VideoCodec::FrameTiming>(new CH264VideoCodec::FrameTiming(spsPayload));

	return TVResult<CMFormatDescriptionRef>(formatDescriptionRef);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CMSampleTimingInfo> CH264DecodeVideoCodec::composeSampleTimingInfo(
		const CMediaPacketSource::DataInfo& dataInfo, UInt32 timeScale)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update frame timing
	TIResult<CH264VideoCodec::FrameTiming::Times>	times = mFrameTiming->updateFrom(dataInfo);
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
OI<SError> CH264DecodeVideoCodec::setup(const SVideoProcessingFormat& videoProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Do super
	OI<SError>	error = CMediaFoundationDecodeVideoCodec::setup(videoProcessingFormat);
	ReturnErrorIfError(error);

	// Finish setup
	mCurrentSPSPPSInfo = OI<CH264VideoCodec::DecodeInfo::SPSPPSInfo>(mDecodeInfo.getSPSPPSInfo());

	const	CH264VideoCodec::NALUInfo&						spsNALUInfo =
																	mCurrentSPSPPSInfo->getSPSNALUInfos().getFirst();
			CH264VideoCodec::SequenceParameterSetPayload	spsPayload(
																	CData(spsNALUInfo.getBytePtr(),
																			spsNALUInfo.getByteCount(), false));
	mFrameTiming = OI<CH264VideoCodec::FrameTiming>(new CH264VideoCodec::FrameTiming(spsPayload));

	return OI<SError>();
}
#endif

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
#elif defined(TARGET_OS_WINDOWS)
//----------------------------------------------------------------------------------------------------------------------
TCIResult<IMFSample> CH264DecodeVideoCodec::readInputSample(
		CMediaFoundationDecodeVideoCodec& mediaFoundationDecodeVideoCodec)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CH264DecodeVideoCodec&	videoCodec = (CH264DecodeVideoCodec&) mediaFoundationDecodeVideoCodec;

	// Get next packet
	TIResult<CMediaPacketSource::DataInfo>	dataInfo = videoCodec.mDecodeInfo.getMediaPacketSource()->readNext();
	ReturnValueIfResultError(dataInfo, TCIResult<IMFSample>(dataInfo.getError()));

	// Update frame timing
	TIResult<CH264VideoCodec::FrameTiming::Times>	times = videoCodec.mFrameTiming->updateFrom(*dataInfo);
	ReturnValueIfResultError(dataInfo, TCIResult<IMFSample>(times.getError()));

	// Create input sample
	TArray<CH264VideoCodec::NALUInfo>	naluInfos = CH264VideoCodec::NALUInfo::getNALUInfos(dataInfo->getData());
	CData								annexBData =
												CH264VideoCodec::NALUInfo::composeAnnexB(
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

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CH264VideoCodec::NALUInfo

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TArray<CH264VideoCodec::NALUInfo> CH264VideoCodec::NALUInfo::getNALUInfos(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNArray<NALUInfo>			naluInfos;

	const	UInt8*				bytePtr = (const UInt8*) data.getBytePtr();
			UInt32				offset = 0;
			CData::ByteCount	bytesRemaining = data.getByteCount();
	while (bytesRemaining > 0) {
		// Get NALU size
		CData::ByteCount	byteCount = EndianU32_BtoN(*((const UInt32*) bytePtr));
		bytePtr += sizeof(UInt32);
		offset += sizeof(UInt32);
		bytesRemaining -= sizeof(UInt32);

		// Add NALU
		naluInfos += NALUInfo(data, offset, byteCount);

		// Update
		bytePtr += byteCount;
		offset += (UInt32) byteCount;
		bytesRemaining -= byteCount;
	}

	return naluInfos;
}

//----------------------------------------------------------------------------------------------------------------------
CData CH264VideoCodec::NALUInfo::composeAnnexB(const TArray<NALUInfo>& spsNALUInfos,
		const TArray<NALUInfo>& ppsNALUInfos, const TArray<NALUInfo>& naluInfos)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CData	data;

	// Add SPS
	for (TIteratorD<NALUInfo> iterator = spsNALUInfos.getIterator(); iterator.hasValue(); iterator.advance())
		data += sAnnexBMarker + CData(iterator->getBytePtr(), iterator->getByteCount(), false);

	// Add PPS
	for (TIteratorD<NALUInfo> iterator = ppsNALUInfos.getIterator(); iterator.hasValue(); iterator.advance())
		data += sAnnexBMarker + CData(iterator->getBytePtr(), iterator->getByteCount(), false);

	// Add NALUs
	for (TIteratorD<NALUInfo> iterator = naluInfos.getIterator(); iterator.hasValue(); iterator.advance())
		data += sAnnexBMarker + CData(iterator->getBytePtr(), iterator->getByteCount(), false);

	return data;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CH264VideoCodec::FrameTiming

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CH264VideoCodec::FrameTiming::FrameTiming(const CH264VideoCodec::SequenceParameterSetPayload& spsPayload) :
		mCurrentFrameNumberBitCount(spsPayload.mFrameNumberBitCount),
				mCurrentPicOrderCountLSBBitCount(spsPayload.mPicOrderCountLSBBitCount),
				mPicOrderCountMSBChangeThreshold(1 << (mCurrentPicOrderCountLSBBitCount - 1)), mPicOrderCountMSB(0),
				mPreviousPicOrderCountLSB(0), mLastIDRFrameTime(0), mCurrentFrameTime(0), mNextFrameTime(0)
//----------------------------------------------------------------------------------------------------------------------
{
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
TIResult<CH264VideoCodec::FrameTiming::Times> CH264VideoCodec::FrameTiming::updateFrom(
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
		OV<UInt32>		size = *bitReader.readUInt32();
		UInt64			pos = bitReader.getPos();

		OV<UInt8>		forbidden_zero_bit = *bitReader.readUInt8(1);	(void) forbidden_zero_bit;
		OV<UInt8>		nal_ref_idc = *bitReader.readUInt8(2);	(void) nal_ref_idc;
		OV<UInt8>		nal_unit_type = *bitReader.readUInt8(5);
		NALUInfo::Type	naluType = (NALUInfo::Type) *nal_unit_type;

		// Check type
		if (naluType == NALUInfo::kTypeCodedSliceNonIDRPicture) {
			// Coded Slice Non-IDR Picture
			OV<UInt32>	first_mb_in_slice = *bitReader.readUEColumbusCode();	(void) first_mb_in_slice;
			OV<UInt32>	slice_type = *bitReader.readUEColumbusCode();
			OV<UInt32>	pic_parameter_set_id = *bitReader.readUEColumbusCode();	(void) pic_parameter_set_id;
			OV<UInt8>	frame_num = *bitReader.readUInt8(mCurrentFrameNumberBitCount);	(void) frame_num;
			OV<UInt8>	pic_order_cnt_lsb = *bitReader.readUInt8(mCurrentPicOrderCountLSBBitCount);

			sliceType = (UInt8) *slice_type;
			picOrderCntLSB = *pic_order_cnt_lsb;
			break;
		} else if (naluType == NALUInfo::kTypeCodedSliceIDRPicture) {
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
		} else if (naluType == NALUInfo::kTypeSupplementalEnhancementInformation) {
			// SEI
		} else
			// Unhandled
			CLogServices::logMessage(CString("Unhandled NALU type: ") + CString(naluType));

		// Next NALU
		OI<SError>	error = bitReader.setPos(CBitReader::kPositionFromBeginning, pos + *size);
		LogIfErrorAndReturnValue(error, "reading next NALU", TIResult<Times>(*error));
	}

	// Handle results
	if (sliceType == 2) {
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
	TIResult<Times>	times = TIResult<Times>(Times(mNextFrameTime, mCurrentFrameTime));

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
OI<SVideoStorageFormat> CH264VideoCodec::composeVideoStorageFormat(const S2DSizeU16& frameSize, Float32 framerate)
//----------------------------------------------------------------------------------------------------------------------
{
	return OI<SVideoStorageFormat>(new SVideoStorageFormat(mID, frameSize, framerate));
}

//----------------------------------------------------------------------------------------------------------------------
I<CDecodeVideoCodec> CH264VideoCodec::create(const I<CRandomAccessDataSource>& randomAccessDataSource,
		const TArray<SMediaPacketAndLocation>& packetAndLocations, const CData& configurationData, UInt32 timeScale,
		const TNumericArray<UInt32>& keyframeIndexes)
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

REGISTER_CODEC(h264, CVideoCodec::Info(CH264VideoCodec::mID, CH264VideoCodec::mName));
