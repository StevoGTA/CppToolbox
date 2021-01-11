//----------------------------------------------------------------------------------------------------------------------
//	CAACAudioCodec.cpp			©2021 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAACAudioCodec.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAACAudioCodec

// MARK: Properties

OSType	CAACAudioCodec::mAACLCID = MAKE_OSTYPE('m', 'p', '4', 'a');
OSType	CAACAudioCodec::mAACLDID = MAKE_OSTYPE('a', 'a', 'c', 'l');

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
