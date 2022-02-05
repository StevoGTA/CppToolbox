//----------------------------------------------------------------------------------------------------------------------
//	CH264VideoCodec.cpp			©2021 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CH264VideoCodec.h"

#include "CCodecRegistry.h"
#include "CLogServices.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CData	sAnnexBMarker(CString(OSSTR("AAAAAQ==")));

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
	CBitReader	bitReader(I<CSeekableDataSource>(new CDataDataSource(dataInfo.getData())), true);
	UInt8		sliceType;
	UInt8		picOrderCntLSB;

	// Iterate NALUs
	while (true) {
		// Get info
		OV<UInt32>		size = bitReader.readUInt32().getValue();
		UInt64			pos = bitReader.getPos();

		OV<UInt8>		forbidden_zero_bit = bitReader.readUInt8(1).getValue();	(void) forbidden_zero_bit;
		OV<UInt8>		nal_ref_idc = bitReader.readUInt8(2).getValue();	(void) nal_ref_idc;
		OV<UInt8>		nal_unit_type = bitReader.readUInt8(5).getValue();
		NALUInfo::Type	naluType = (NALUInfo::Type) *nal_unit_type;

		// Check type
		if (naluType == NALUInfo::kTypeCodedSliceNonIDRPicture) {
			// Coded Slice Non-IDR Picture
			OV<UInt32>	first_mb_in_slice = bitReader.readUEColumbusCode().getValue();	(void) first_mb_in_slice;
			OV<UInt32>	slice_type = bitReader.readUEColumbusCode().getValue();
			OV<UInt32>	pic_parameter_set_id = bitReader.readUEColumbusCode().getValue();	(void) pic_parameter_set_id;
			OV<UInt8>	frame_num = bitReader.readUInt8(mCurrentFrameNumberBitCount).getValue();	(void) frame_num;
			OV<UInt8>	pic_order_cnt_lsb = bitReader.readUInt8(mCurrentPicOrderCountLSBBitCount).getValue();

			sliceType = (UInt8) *slice_type;
			picOrderCntLSB = *pic_order_cnt_lsb;
			break;
		} else if (naluType == NALUInfo::kTypeCodedSliceIDRPicture) {
			// Coded Slice IDR Picture
			OV<UInt32>	first_mb_in_slice = bitReader.readUEColumbusCode().getValue();	(void) first_mb_in_slice;
			OV<UInt32>	slice_type = bitReader.readUEColumbusCode().getValue();
			OV<UInt32>	pic_parameter_set_id = bitReader.readUEColumbusCode().getValue();	(void) pic_parameter_set_id;
			OV<UInt8>	frame_num = bitReader.readUInt8(mCurrentFrameNumberBitCount).getValue();	(void) frame_num;
			OV<UInt32>	idr_pic_id = bitReader.readUEColumbusCode().getValue();	(void) idr_pic_id;
			OV<UInt8>	pic_order_cnt_lsb = bitReader.readUInt8(mCurrentPicOrderCountLSBBitCount).getValue();

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

OSType	CH264VideoCodec::mID = MAKE_OSTYPE('h', '2', '6', '4');

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
OI<SVideoStorageFormat> CH264VideoCodec::composeStorageFormat(const S2DSizeU16& frameSize, Float32 framerate)
//----------------------------------------------------------------------------------------------------------------------
{
	return OI<SVideoStorageFormat>(new SVideoStorageFormat(mID, frameSize, framerate));
}

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local procs

static	I<CVideoCodec>	sInstantiate(OSType id)
							{ return I<CVideoCodec>(new CH264VideoCodec()); }

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Declare video codecs

REGISTER_CODEC(h264, CVideoCodec::Info(CH264VideoCodec::mID, CString("h.264"), sInstantiate));
