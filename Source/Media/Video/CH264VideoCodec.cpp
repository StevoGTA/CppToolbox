//----------------------------------------------------------------------------------------------------------------------
//	CH264VideoCodec.cpp			©2021 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CH264VideoCodec.h"

#include "CCodecRegistry.h"

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
	TNArray<NALUInfo>	naluInfos;

	const	UInt8*		bytePtr = (const UInt8*) data.getBytePtr();
			UInt32		offset = 0;
			CData::Size	bytesRemaining = data.getSize();
	while (bytesRemaining > 0) {
		// Get NALU size
		CData::Size	size = EndianU32_BtoN(*((const UInt32*) bytePtr));
		bytePtr += sizeof(UInt32);
		offset += sizeof(UInt32);
		bytesRemaining -= sizeof(UInt32);

		// Add NALU
		naluInfos += NALUInfo(data, offset, size);

		// Update
		bytePtr += size;
		offset += (UInt32) size;
		bytesRemaining -= size;
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
		data += sAnnexBMarker + CData(iterator->getBytePtr(), iterator->getSize(), false);

	// Add PPS
	for (TIteratorD<NALUInfo> iterator = ppsNALUInfos.getIterator(); iterator.hasValue(); iterator.advance())
		data += sAnnexBMarker + CData(iterator->getBytePtr(), iterator->getSize(), false);

	// Add NALUs
	for (TIteratorD<NALUInfo> iterator = naluInfos.getIterator(); iterator.hasValue(); iterator.advance())
		data += sAnnexBMarker + CData(iterator->getBytePtr(), iterator->getSize(), false);

	return data;
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
