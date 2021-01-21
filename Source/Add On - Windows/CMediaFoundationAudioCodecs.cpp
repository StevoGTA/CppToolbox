//----------------------------------------------------------------------------------------------------------------------
//	CMediaFoundationAudioCodecs.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAACAudioCodec.h"

#include "CMediaFoundationServices.h"

#include "CLogServices-Windows.h"
#include "SError-Windows.h"

#undef Delete

#include <mfapi.h>
#include <mftransform.h>
#include <Mferror.h>

#define Delete(x)	{ delete x; x = nil; }

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sErrorDomain(OSSTR("CMediaFoundationAudioCodecs"));
static	SError	sSetupDidNotCompleteError(sErrorDomain, 1, CString(OSSTR("Setup did not complete")));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAACAudioCodecInternals

class CAACAudioCodecInternals {
	public:
							CAACAudioCodecInternals(OSType codecID) : mCodecID(codecID), mNextPacketIndex(0) {}

		static	OI<SError>	readInputPacket(IMFMediaBuffer* mediaBuffer, void* userData)
								{
									// Setup
									CAACAudioCodecInternals&	internals = *((CAACAudioCodecInternals*) userData);

									// Check packet index
									if (internals.mNextPacketIndex < internals.mPacketLocations->getCount()) {
										// Read next packet
										CAudioCodec::PacketLocation&	packetLocation =
																				internals.mPacketLocations->getAt(
																						internals.mNextPacketIndex);

										// Read
										OI<SError>	error =
															CMediaFoundationServices::readSample(
																	*internals.mByteParceller, packetLocation.mPos,
																	packetLocation.mPacket.mSize, mediaBuffer);
										ReturnErrorIfError(error);

										// Update
										internals.mNextPacketIndex++;

										return OI<SError>();
									} else
										// End of data
										return OI<SError>(SError::mEndOfData);
								}


		OSType										mCodecID;
		OI<CByteParceller>							mByteParceller;
		OI<SAudioProcessingFormat>					mAudioProcessingFormat;
		OI<TArray<CAudioCodec::PacketLocation> >	mPacketLocations;

		OCI<IMFTransform>							mAudioDecoder;
		OCI<IMFSample>								mInputSample;
		OCI<IMFSample>								mOutputSample;
		UInt32										mNextPacketIndex;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAACAudioCodec

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAACAudioCodec::CAACAudioCodec(OSType codecID)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CAACAudioCodecInternals(codecID);
}

//----------------------------------------------------------------------------------------------------------------------
CAACAudioCodec::~CAACAudioCodec()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CAudioCodec methods - Decoding

//----------------------------------------------------------------------------------------------------------------------
void CAACAudioCodec::setupForDecode(const SAudioProcessingFormat& audioProcessingFormat, CByteParceller& byteParceller,
		const I<CAudioCodec::DecodeInfo>& decodeInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	PacketsWithMagicCookieDecodeInfo&	packetsWithMagicCookieDecodeInfo =
														*((PacketsWithMagicCookieDecodeInfo*) &*decodeInfo);

	// Store
	mInternals->mByteParceller = OI<CByteParceller>(byteParceller);
	mInternals->mAudioProcessingFormat = OI<SAudioProcessingFormat>(audioProcessingFormat);
	mInternals->mPacketLocations = OI<TArray<CAudioCodec::PacketLocation> >(
			packetsWithMagicCookieDecodeInfo.getPacketLocations());

	// Setup
// TODO: Get audio specific config from actual AAC info
#pragma pack(push,1)
	struct UserData {
		WORD	mPayloadType;
		WORD	mAudioProfileLevelIndication;
		WORD	mStructType;
		WORD	mReserved1;
		DWORD	mReserved2;
		WORD	mAudioSpecificConfig;
	} userData = {0};
#pragma pack(pop)
	userData.mAudioSpecificConfig = 0x1012;

	mInternals->mAudioDecoder =
			CMediaFoundationServices::createAudioDecoder(MFAudioFormat_AAC, audioProcessingFormat,
					OI<CData>(new CData(&userData, sizeof(UserData), false)));
	if (!mInternals->mAudioDecoder.hasInstance())
		return;

	// Create input sample
	mInternals->mInputSample = CMediaFoundationServices::createSample(10 * 1024);
	if (!mInternals->mInputSample.hasInstance())
		return;

	// Create output sample
	MFT_OUTPUT_STREAM_INFO	outputStreamInfo;
	HRESULT					result = mInternals->mAudioDecoder->GetOutputStreamInfo(0, &outputStreamInfo);
	if (FAILED(result)) {
		// Failed
		LogHRESULT(result, "GetOutputStreamInfo");

		return;
	}

	mInternals->mOutputSample = CMediaFoundationServices::createSample(outputStreamInfo.cbSize);
	if (!mInternals->mOutputSample.hasInstance())
		return;

	// Begin streaming!
	result = mInternals->mAudioDecoder->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
	LogHRESULTIfFailed(result, "ProcessMessage to begin streaming");
}

//----------------------------------------------------------------------------------------------------------------------
SAudioReadStatus CAACAudioCodec::decode(const SMediaPosition& mediaPosition, CAudioData& audioData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	if (!mInternals->mAudioDecoder.hasInstance() || !mInternals->mInputSample.hasInstance() ||
			!mInternals->mOutputSample.hasInstance())
		// Can't decode
		return SAudioReadStatus(sSetupDidNotCompleteError);

	// Setup
	OI<SError>	error;

	// Update read position if needed
	if ((mediaPosition.getMode() != SMediaPosition::kFromCurrent) && (mInternals->mNextPacketIndex != 0)) {
		// Flush audio decoder
		error = CMediaFoundationServices::flush(*mInternals->mAudioDecoder);
		if (error.hasInstance()) return SAudioReadStatus(*error);

		// Update next packet index
		mInternals->mNextPacketIndex =
				getPacketIndex(mediaPosition, *mInternals->mAudioProcessingFormat, *mInternals->mPacketLocations);
	}

	// Process output
	Float32	sourceProcessed =
					(Float32) mInternals->mNextPacketIndex / (Float32) mInternals->mPacketLocations->getCount();
	error =
			CMediaFoundationServices::processOutput(*mInternals->mAudioDecoder, *mInternals->mInputSample,
					*mInternals->mOutputSample, CAACAudioCodecInternals::readInputPacket, mInternals);
	if (error.hasInstance()) return SAudioReadStatus(*error);

	error =
			CMediaFoundationServices::copySample(*mInternals->mOutputSample, audioData,
					*mInternals->mAudioProcessingFormat);
	if (error.hasInstance()) return SAudioReadStatus(*error);

	return SAudioReadStatus(sourceProcessed);
}
