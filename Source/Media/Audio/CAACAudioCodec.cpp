//----------------------------------------------------------------------------------------------------------------------
//	CAACAudioCodec.cpp			©2021 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAACAudioCodec.h"

#include "CCodecRegistry.h"
#include "CAudioProcessor.h"
#include "CMediaPacketSource.h"

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
	#include "CCoreAudioDecodeAudioCodec.h"
	#include "SError-Apple.h"
#elif defined(TARGET_OS_WINDOWS)
	#include "CMediaFoundationDecodeAudioCodec.h"
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

#pragma pack(push, 1)

#if defined(TARGET_OS_WINDOWS)
	#pragma warning(disable:4200)
#endif

//enum EESDSDescriptorType {
//	kESDSDescriptorTypeES 					= 0x03,
//	kESDSDescriptorTypeDecoderConfig		= 0x04,
//	kESDSDescriptorTypeDecoderSpecificInfo	= 0x05,
//	kESDSDescriptorTypeSyncLayerConfig		= 0x06,
//};

struct SesdsDecoderSpecificDescriptor {
			// Methods
	CData	getStartCodes() const
				{
					// Check for minimal/extended
					if (_.mExtendedInfo.mExtendedDescriptorType[0] != 0x80)
						// Minimal
						return CData(_.mMinimalInfo.mStartCodes, _.mMinimalInfo.mDescriptorLength);
					else
						// Extended
						return CData(_.mExtendedInfo.mStartCodes, _.mExtendedInfo.mDescriptorLength);
				}

	// Properties (in storage endian)
	private:
		UInt8	mDescriptorType;					// 5
		union {
			struct MinimalInfo {
				UInt8	mDescriptorLength;			// size of 5

				UInt8	mStartCodes[];
			} mMinimalInfo;
			struct ExtendedInfo {
				UInt8	mExtendedDescriptorType[3];
				UInt8	mDescriptorLength;			// size of 5

				UInt8	mStartCodes[];
			} mExtendedInfo;
		} _;
};

struct SesdsDecoderConfigDescriptor {
											// Methods
			UInt32							getLength() const
												{
													// Check for minimal/extended
													if (_.mExtendedInfo.mExtendedDescriptorType[0] != 0x80)
														// Minimal
														return _.mMinimalInfo.mDescriptorLength;
													else
														// Extended
														return _.mExtendedInfo.mDescriptorLength;
												}
	const	SesdsDecoderSpecificDescriptor&	getDecoderSpecificDescriptor() const
												{
													// Check for minimal/extended
													if (_.mExtendedInfo.mExtendedDescriptorType[0] != 0x80)
														// Minimal
														return _.mMinimalInfo.mDecoderSpecificDescriptor;
													else
														// Extended
														return _.mExtendedInfo.mDecoderSpecificDescriptor;
												}

	// Properties (in storage endian)
	private:
		UInt8							mDescriptorType;					// 4
		union {
			struct MinimalInfo {
				UInt8							mDescriptorLength;			// size of 4 + 5

				UInt8							mObjectTypeID;
				UInt8							mStreamTypeAndFlags;
				UInt8							mBufferSize[3];
				UInt32							mMaximumBitrate;
				UInt32							mAverageBitrate;
				SesdsDecoderSpecificDescriptor	mDecoderSpecificDescriptor;
			} mMinimalInfo;
			struct ExtendedInfo {
				UInt8							mExtendedDescriptorType[3];
				UInt8							mDescriptorLength;			// size of 4 + 5

				UInt8							mObjectTypeID;
				UInt8							mStreamTypeAndFlags;
				UInt8							mBufferSize[3];
				UInt32							mMaximumBitrate;
				UInt32							mAverageBitrate;
				SesdsDecoderSpecificDescriptor	mDecoderSpecificDescriptor;
			} mExtendedInfo;
		} _;
};

struct SesdsSyncLayerDescriptor {
			// Methods

	// Properties (in storage endian)
	private:
		UInt8	mDescriptorType;					// 6
		union {
			struct MinimalInfo {
				UInt8	mDescriptorLength;			// size of 6

				UInt8	mSyncLayerValue;
			} mMinimalInfo;
			struct ExtendedInfo {
				UInt8	mExtendedDescriptorType[3];
				UInt8	mDescriptorLength;			// size of 6

				UInt8	mSyncLayerValue;
			} mExtendedInfo;
		} _;
};

struct SesdsAtomPayload {
											// Methods
	const	SesdsDecoderConfigDescriptor&	getDecoderConfigDescriptor() const
												{
													// Check for minimal/extended
													if (_.mExtendedInfo.mExtendedDescriptorType[0] != 0x80)
														// Minimal
														return *((SesdsDecoderConfigDescriptor*)
																&_.mMinimalInfo.mChildDescriptorData);
													else
														// Extended
														return *((SesdsDecoderConfigDescriptor*)
																&_.mExtendedInfo.mChildDescriptorData);
												}
	const	SesdsSyncLayerDescriptor&		getSyncLayerDescriptor() const
												{
													// Setup
													UInt32	offset = getDecoderConfigDescriptor().getLength();

													// Check for minimal/extended
													if (_.mExtendedInfo.mExtendedDescriptorType[0] != 0x80)
														// Minimal
														return *((SesdsSyncLayerDescriptor*)
																&_.mMinimalInfo.mChildDescriptorData[offset]);
													else
														// Extended
														return *((SesdsSyncLayerDescriptor*)
																&_.mExtendedInfo.mChildDescriptorData[offset]);
												}

	// Properties (in storage endian)
	private:
		UInt8	mVersion;							// 0
		UInt8	mFlags[3];
		UInt8	mDescriptorType;					// 3
		union {
			struct MinimalInfo {
				UInt8	mDescriptorLength;			// size of 3 + 4 + 5 + 6

				UInt16	mESID;
				UInt8	mStreamPriority;
				UInt8	mChildDescriptorData[];
			} mMinimalInfo;

			struct ExtendedInfo {
				UInt8	mExtendedDescriptorType[3];
				UInt8	mDescriptorLength;			// size of 3 + 4 + 5 + 6

				UInt16	mESID;
				UInt8	mStreamPriority;
				UInt8	mChildDescriptorData[];
			} mExtendedInfo;
		} _;
};

#if defined(TARGET_OS_WINDOWS)
	#pragma warning(default:4200)
#endif

#pragma pack(pop)

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAACDecodeAudioCodec

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
class CAACDecodeAudioCodec : public CCoreAudioDecodeAudioCodec {
#elif defined(TARGET_OS_WINDOWS)
class CAACDecodeAudioCodec : public CMediaFoundationDecodeAudioCodec {
#endif
	public:
										// Lifecycle methods
										CAACDecodeAudioCodec(const CAACAudioCodec::Info& info,
												const I<CMediaPacketSource>& mediaPacketSource) :
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
											CCoreAudioDecodeAudioCodec(info.getCodecID(), mediaPacketSource),
													mInfo(info)
#elif defined(TARGET_OS_WINDOWS)
											CMediaFoundationDecodeAudioCodec(info.getCodecID(), mediaPacketSource),
													mInfo(info)
#endif
											{}

										// CDecodeAudioCodec methods
		TArray<SAudio::ProcessingSetup>	getAudioProcessingSetups(const SAudio::Format& audioFormat)
											{
												return TNArray<SAudio::ProcessingSetup>(
														SAudio::ProcessingSetup(32, audioFormat.getSampleRate(),
																audioFormat.getChannelMap(),
																SAudio::ProcessingSetup::kSampleTypeFloat));
											}
		CAudioFrames::Requirements		getRequirements() const
											{ return CAudioFrames::Requirements(1024, 1024 * 2); }

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
										// CCoreAudioDecodeAudioCodec methods
		AudioStreamBasicDescription		getSourceASBD(OSType codecID,
												const SAudio::ProcessingFormat& audioProcessingFormat)
											{
												// Setup
												AudioStreamBasicDescription	asbd = {0};
												asbd.mFormatID =
														(codecID == CAACAudioCodec::mLCID) ?
																kAudioFormatMPEG4AAC : kAudioFormatMPEG4AAC_LD;
												asbd.mSampleRate = audioProcessingFormat.getSampleRate();
												asbd.mChannelsPerFrame =
														audioProcessingFormat.getChannelMap().getChannelCount();

												return asbd;
											}
		OV<SError>						setMagicCookie(AudioConverterRef audioConverterRef)
											{
												// Set magic cookie
												OSStatus	status =
																	::AudioConverterSetProperty(audioConverterRef,
																			kAudioConverterDecompressionMagicCookie,
																			(UInt32)
																					mInfo.getMagicCookie()
																							.getByteCount(),
																			mInfo.getMagicCookie().getBytePtr());
												ReturnErrorIfFailed(status,
														OSSTR("AudioConverterSetProperty for magic cookie"));

												return OV<SError>();
											}
#elif defined(TARGET_OS_WINDOWS)
										// CMediaFoundationDecodeAudioCodec methods
		OR<const GUID>					getGUID(OSType codecID) const
											{ return OR<const GUID>(MFAudioFormat_AAC); }
		OV<CData>						getUserData() const
											{
												// Setup
												#pragma pack(push, 1)
													struct UserData {
														WORD	mPayloadType;
														WORD	mAudioProfileLevelIndication;
														WORD	mStructType;
														WORD	mReserved1;
														DWORD	mReserved2;
														WORD	mAudioSpecificConfig;
													} userData = {0};
												#pragma pack(pop)
												userData.mAudioSpecificConfig = EndianU16_NtoB(mInfo.getStartCodes());

												return OV<CData>(CData(&userData, sizeof(UserData)));
											}
#endif

	private:
		CAACAudioCodec::Info	mInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAACAudioCodec

// MARK: Properties

const	OSType	CAACAudioCodec::mLCID = MAKE_OSTYPE('m', 'p', '4', 'a');
const	OSType	CAACAudioCodec::mLDID = MAKE_OSTYPE('a', 'a', 'c', 'l');

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
OV<CAACAudioCodec::Info> CAACAudioCodec::composeInfo(const CData& configurationData, UInt16 channels)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	SesdsAtomPayload&				esdsAtomPayload = *((SesdsAtomPayload*) configurationData.getBytePtr());
	const	SesdsDecoderConfigDescriptor&	esdsDecoderConfigDescriptor = esdsAtomPayload.getDecoderConfigDescriptor();
	const	SesdsDecoderSpecificDescriptor&	esdsDecoderSpecificDescriptor =
													esdsDecoderConfigDescriptor.getDecoderSpecificDescriptor();
			CData							startCodesData = esdsDecoderSpecificDescriptor.getStartCodes();
			UInt16							startCodes = EndianU16_BtoN(*((UInt16*) startCodesData.getBytePtr()));

	// See https://wiki.multimedia.cx/index.php/MPEG-4_Audio
	// Codec ID
	OSType	codecID;
	switch (startCodes >> 11) {
		case 1:
		case 2:
		case 3:
		case 4:
		case 6:
			// AAC variants
			codecID = CAACAudioCodec::mLCID;
			break;

		case 23:
			// AAC-LD
			codecID = CAACAudioCodec::mLDID;
			break;

		default:
			// Not yet supported
			return OV<Info>();
	}

	Float32	sampleRate;
	switch ((startCodes & 0x0780) >> 7) {
		case 0:		sampleRate = 96000.0;	break;
		case 1:		sampleRate = 88200.0;	break;
		case 2:		sampleRate = 64000.0;	break;
		case 3:		sampleRate = 48000.0;	break;
		case 4:		sampleRate = 44100.0;	break;
		case 5:		sampleRate = 32000.0;	break;
		case 6:		sampleRate = 24000.0;	break;
		case 7:		sampleRate = 22050.0;	break;
		case 8:		sampleRate = 16000.0;	break;
		case 9:		sampleRate = 12000.0;	break;
		case 10:	sampleRate = 11025.0;	break;
		case 11:	sampleRate = 8000.0;	break;
		case 12:	sampleRate = 7350.0;	break;
		default:	return OV<Info>();
	}

	OV<SAudio::ChannelMap>	audioChannelMap;
	switch ((startCodes & 0x0078) >> 3) {
		case 0:		audioChannelMap.setValue(SAudio::ChannelMap((UInt8) channels));	break;
		case 1:		audioChannelMap.setValue(SAudio::ChannelMap::_1_0());			break;
		case 2:		audioChannelMap.setValue(SAudio::ChannelMap::_2_0_Option1());	break;
		case 3:		audioChannelMap.setValue(SAudio::ChannelMap::_3_0_Option2());	break;
		case 4:		audioChannelMap.setValue(SAudio::ChannelMap::_4_0_Option3());	break;
		case 5:		audioChannelMap.setValue(SAudio::ChannelMap::_5_0_Option4());	break;
		case 6:		audioChannelMap.setValue(SAudio::ChannelMap::_5_1_Option4());	break;
		case 7:		audioChannelMap.setValue(SAudio::ChannelMap::_7_1_Option2());	break;
		default:	return OV<Info>();
	}

	return OV<Info>(
			Info(codecID, sampleRate, *audioChannelMap,
					CData((UInt8*) configurationData.getBytePtr() + 4, configurationData.getByteCount() - 4),
					EndianU16_BtoN(*((UInt16*) startCodesData.getBytePtr()))));
}

//----------------------------------------------------------------------------------------------------------------------
SAudio::Format CAACAudioCodec::composeAudioFormat(const Info& info)
//----------------------------------------------------------------------------------------------------------------------
{
	return SAudio::Format(info.getCodecID(), info.getSampleRate(), info.getAudioChannelMap());
}

//----------------------------------------------------------------------------------------------------------------------
I<CDecodeAudioCodec> CAACAudioCodec::create(const Info& info,
		const I<CRandomAccessDataSource>& randomAccessDataSource,
		const TArray<SMedia::PacketAndLocation>& packetAndLocations)
//----------------------------------------------------------------------------------------------------------------------
{
	return I<CDecodeAudioCodec>(
			new CAACDecodeAudioCodec(info,
					I<CMediaPacketSource>(
							new CSeekableVaryingMediaPacketSource(randomAccessDataSource, packetAndLocations))));
}
