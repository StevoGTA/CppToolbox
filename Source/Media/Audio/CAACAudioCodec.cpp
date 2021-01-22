//----------------------------------------------------------------------------------------------------------------------
//	CAACAudioCodec.cpp			©2021 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAACAudioCodec.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAACAudioCodec

// MARK: Properties

OSType	CAACAudioCodec::mAACLCID = MAKE_OSTYPE('m', 'p', '4', 'a');
OSType	CAACAudioCodec::mAACLDID = MAKE_OSTYPE('a', 'a', 'c', 'l');

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
OI<SAudioStorageFormat> CAACAudioCodec::composeAudioStorageFormat(UInt16 startCodes, UInt16 channels)
//----------------------------------------------------------------------------------------------------------------------
{
	// See https://wiki.multimedia.cx/index.php/MPEG-4_Audio
	// Codec ID
	OSType	codecID;
	switch (startCodes >> 11) {
		case 02:	codecID = CAACAudioCodec::mAACLCID;	break;
		case 23:	codecID = CAACAudioCodec::mAACLDID;	break;
		default:	return OI<SAudioStorageFormat>();
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
		default:	return OI<SAudioStorageFormat>();
	}

	EAudioChannelMap	channelMap;
	switch ((startCodes & 0x0078) >> 3) {
		case 0:		channelMap = AUDIOCHANNELMAP_FORUNKNOWN(channels);	break;
		case 1:		channelMap = kAudioChannelMap_1_0;					break;
		case 2:		channelMap = kAudioChannelMap_2_0_Option1;			break;
		case 3:		channelMap = kAudioChannelMap_3_0_Option2;			break;
		case 4:		channelMap = kAudioChannelMap_4_0_Option3;			break;
		case 5:		channelMap = kAudioChannelMap_5_0_Option4;			break;
		case 6:		channelMap = kAudioChannelMap_5_1_Option4;			break;
		case 7:		channelMap = kAudioChannelMap_7_1_Option2;			break;
		default:	return OI<SAudioStorageFormat>();
	}

	return OI<SAudioStorageFormat>(new SAudioStorageFormat(codecID, sampleRate, channelMap));
}

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local procs

static	TArray<SAudioProcessingSetup>	sGetAudioProcessingSetups(OSType id,
												const SAudioStorageFormat& audioStorageFormat)
											{
												return TNArray<SAudioProcessingSetup>(
														SAudioProcessingSetup(32, audioStorageFormat.getSampleRate(),
																audioStorageFormat.getChannelMap(),
																SAudioProcessingSetup::kSampleTypeFloat));
											}
static	I<CAudioCodec>					sInstantiate(OSType id)
											{ return I<CAudioCodec>(new CAACAudioCodec(id)); }

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Declare audio codecs

REGISTER_AUDIO_CODEC(AACLC,
		CAudioCodec::Info(CAACAudioCodec::mAACLCID, CString("AAC Low Complexity"), sGetAudioProcessingSetups,
				sInstantiate));
REGISTER_AUDIO_CODEC(AACLD,
		CAudioCodec::Info(CAACAudioCodec::mAACLDID, CString("AAC Low Delay"), sGetAudioProcessingSetups, sInstantiate));
