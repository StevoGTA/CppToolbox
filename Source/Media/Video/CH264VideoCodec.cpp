//----------------------------------------------------------------------------------------------------------------------
//	CH264VideoCodec.cpp			©2021 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CH264VideoCodec.h"

#include "CCodecRegistry.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CH264VideoCodec

// MARK: Properties

OSType	CH264VideoCodec::mID = MAKE_OSTYPE('h', '2', '6', '4');

// MARK: Instance methods

////----------------------------------------------------------------------------------------------------------------------
//TArray<CH264VideoCodec::NALUInfo> CH264VideoCodec::getNALUInfos(const CData& data)
////----------------------------------------------------------------------------------------------------------------------
//{
//	// Setup
//	TNArray<NALUInfo>	naluInfos;
//
//	const	UInt8*	bytePtr = (const UInt8*) data.getBytePtr();
//			UInt32	offset = 0;
//			Size	bytesRemaining = data.getSize();
//	while (bytesRemaining > 0) {
//		// Get NALU size
//		UInt32	size = EndianU32_BtoN(*((const UInt32*) bytePtr));
//		bytePtr += sizeof(UInt32);
//		offset += sizeof(UInt32);
//		bytesRemaining -= sizeof(UInt32);
//
//		// Add NALU
//		naluInfos += NALUInfo(data, offset);
//
//		// Update
//		bytePtr += size;
//		offset += size;
//		bytesRemaining -= size;
//	}
//
//	return naluInfos;
//}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
OI<SVideoStorageFormat> CH264VideoCodec::composeStorageFormat(const S2DSizeU16& frameSize)
//----------------------------------------------------------------------------------------------------------------------
{
	return OI<SVideoStorageFormat>(new SVideoStorageFormat(mID, frameSize));
}

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local procs

static	I<CVideoCodec>	sInstantiate(OSType id, const I<CDataSource>& dataSource,
								const I<CCodec::DecodeInfo>& decodeInfo,
								const CVideoCodec::DecodeFrameInfo& decodeFrameInfo)
							{ return I<CVideoCodec>(new CH264VideoCodec(dataSource, decodeInfo, decodeFrameInfo)); }

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Declare video codecs

REGISTER_CODEC(h264, CVideoCodec::Info(CH264VideoCodec::mID, CString("h.264"), sInstantiate));
