//----------------------------------------------------------------------------------------------------------------------
//	SAudio.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SAudio.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SAudio::ChannelMap

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
#define CHANNELMAP(CM, F)								\
	const	SAudio::ChannelMap&	F						\
	{													\
		static	ChannelMap*	sValue = nil;				\
		if (sValue == nil) sValue = new ChannelMap(CM);	\
														\
		return *sValue;									\
	}

CHANNELMAP((UInt16) 0x0001, SAudio::ChannelMap::_1_0())

CHANNELMAP((UInt16) 0x0002, SAudio::ChannelMap::_2_0_Option1())
CHANNELMAP((UInt16) 0x0102, SAudio::ChannelMap::_1_1_Option1())
CHANNELMAP((UInt16) 0x0202, SAudio::ChannelMap::_2_0_Option2())
CHANNELMAP((UInt16) 0x0302, SAudio::ChannelMap::_2_0_Option3())
//CHANNELMAP((UInt16) 0x0402, SAudio::ChannelMap::_2_0_Option4())
//CHANNELMAP((UInt16) 0x0502, SAudio::ChannelMap::_2_0_Option5())
//CHANNELMAP((UInt16) 0x0602, SAudio::ChannelMap::_2_0_Option6())

CHANNELMAP((UInt16) 0x0003, SAudio::ChannelMap::_3_0_Unknown())
CHANNELMAP((UInt16) 0x0103, SAudio::ChannelMap::_2_1_Option1())
CHANNELMAP((UInt16) 0x0203, SAudio::ChannelMap::_3_0_Option1())
CHANNELMAP((UInt16) 0x0303, SAudio::ChannelMap::_3_0_Option2())
CHANNELMAP((UInt16) 0x0403, SAudio::ChannelMap::_3_0_Option3())
CHANNELMAP((UInt16) 0x0503, SAudio::ChannelMap::_3_0_Option4())

CHANNELMAP((UInt16) 0x0004, SAudio::ChannelMap::_4_0_Unknown())
CHANNELMAP((UInt16) 0x0104, SAudio::ChannelMap::_3_1_Option1())
CHANNELMAP((UInt16) 0x0204, SAudio::ChannelMap::_3_1_Option2())
CHANNELMAP((UInt16) 0x0304, SAudio::ChannelMap::_3_1_Option3())
CHANNELMAP((UInt16) 0x0404, SAudio::ChannelMap::_3_1_Option4())
CHANNELMAP((UInt16) 0x0504, SAudio::ChannelMap::_4_0_Option1())
CHANNELMAP((UInt16) 0x0604, SAudio::ChannelMap::_4_0_Option2())
CHANNELMAP((UInt16) 0x0704, SAudio::ChannelMap::_4_0_Option3())
CHANNELMAP((UInt16) 0x0804, SAudio::ChannelMap::_4_0_Option4())
//CHANNELMAP((UInt16) 0x0904, SAudio::ChannelMap::_4_0_Option5())

CHANNELMAP((UInt16) 0x0005, SAudio::ChannelMap::_5_0_Unknown())
CHANNELMAP((UInt16) 0x0105, SAudio::ChannelMap::_4_1_Option1())
CHANNELMAP((UInt16) 0x0205, SAudio::ChannelMap::_4_1_Option2())
CHANNELMAP((UInt16) 0x0305, SAudio::ChannelMap::_4_1_Option3())
CHANNELMAP((UInt16) 0x0405, SAudio::ChannelMap::_4_1_Option4())
CHANNELMAP((UInt16) 0x0505, SAudio::ChannelMap::_5_0_Option1())
CHANNELMAP((UInt16) 0x0605, SAudio::ChannelMap::_5_0_Option2())
CHANNELMAP((UInt16) 0x0705, SAudio::ChannelMap::_5_0_Option3())
CHANNELMAP((UInt16) 0x0805, SAudio::ChannelMap::_5_0_Option4())

CHANNELMAP((UInt16) 0x0006, SAudio::ChannelMap::_6_0_Unknown())
CHANNELMAP((UInt16) 0x0106, SAudio::ChannelMap::_5_1_Option1())
CHANNELMAP((UInt16) 0x0206, SAudio::ChannelMap::_5_1_Option2())
CHANNELMAP((UInt16) 0x0306, SAudio::ChannelMap::_5_1_Option3())
CHANNELMAP((UInt16) 0x0406, SAudio::ChannelMap::_5_1_Option4())
CHANNELMAP((UInt16) 0x0506, SAudio::ChannelMap::_6_0_Option1())
CHANNELMAP((UInt16) 0x0606, SAudio::ChannelMap::_6_0_Option2())

CHANNELMAP((UInt16) 0x0007, SAudio::ChannelMap::_7_0_Unknown())
CHANNELMAP((UInt16) 0x0107, SAudio::ChannelMap::_6_1_Option1())
CHANNELMAP((UInt16) 0x0207, SAudio::ChannelMap::_6_1_Option2())
CHANNELMAP((UInt16) 0x0307, SAudio::ChannelMap::_6_1_Option3())
CHANNELMAP((UInt16) 0x0407, SAudio::ChannelMap::_7_0_Option1())
CHANNELMAP((UInt16) 0x0507, SAudio::ChannelMap::_7_0_Option2())
CHANNELMAP((UInt16) 0x0607, SAudio::ChannelMap::_7_0_Option3())

CHANNELMAP((UInt16) 0x0008, SAudio::ChannelMap::_8_0_Unknown())
CHANNELMAP((UInt16) 0x0108, SAudio::ChannelMap::_7_1_Option1())
CHANNELMAP((UInt16) 0x0208, SAudio::ChannelMap::_7_1_Option2())
CHANNELMAP((UInt16) 0x0308, SAudio::ChannelMap::_7_1_Option3())
CHANNELMAP((UInt16) 0x0408, SAudio::ChannelMap::_7_1_Option4())
CHANNELMAP((UInt16) 0x0508, SAudio::ChannelMap::_7_1_Option5())
CHANNELMAP((UInt16) 0x0608, SAudio::ChannelMap::_7_1_Option6())
CHANNELMAP((UInt16) 0x0708, SAudio::ChannelMap::_8_0_Option1())
CHANNELMAP((UInt16) 0x0808, SAudio::ChannelMap::_8_0_Option2())
//CHANNELMAP((UInt16) 0x0908, SAudio::ChannelMap::_8_0_Option2())
CHANNELMAP((UInt16) 0x0A08, SAudio::ChannelMap::_7_1_Option7())

CHANNELMAP((UInt16) 0x0009, SAudio::ChannelMap::_9_0_Unknown())

CHANNELMAP((UInt16) 0x000A, SAudio::ChannelMap::_10_0_Unknown())

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SAudio::ProcessingSetup::BitsInfo

const	SAudio::ProcessingSetup::BitsInfo	SAudio::ProcessingSetup::BitsInfo::mUnspecified(kUnspecified);
const	SAudio::ProcessingSetup::BitsInfo	SAudio::ProcessingSetup::BitsInfo::mUnchanged(kUnchanged);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SAudio::ProcessingSetup::SampleRateInfo

const	SAudio::ProcessingSetup::SampleRateInfo	SAudio::ProcessingSetup::SampleRateInfo::mUnspecified(kUnspecified);
const	SAudio::ProcessingSetup::SampleRateInfo	SAudio::ProcessingSetup::SampleRateInfo::mUnchanged(kUnchanged);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SAudio::ProcessingSetup::ChannelMapInfo

const	SAudio::ProcessingSetup::ChannelMapInfo	SAudio::ProcessingSetup::ChannelMapInfo::mUnspecified(kUnspecified);
const	SAudio::ProcessingSetup::ChannelMapInfo	SAudio::ProcessingSetup::ChannelMapInfo::mUnchanged(kUnchanged);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SAudio::ProcessingSetup

const	SAudio::ProcessingSetup	SAudio::ProcessingSetup::mUnspecified(BitsInfo::mUnspecified,
										SampleRateInfo::mUnspecified, ChannelMapInfo::mUnspecified,
										kSampleTypeUnspecified, kEndianUnspecified, kInterleavedUnspecified);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SAudio

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CString SAudio::getStringFromDB(Float32 db, Float32 muteDB)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value
	if (db == muteDB)
		// Silence
		return CString(OSSTR("-\u221EdB"));
	else if (db < 1.0)
		// Negative
		return CString(db, 0, 1) + CString(OSSTR("dB"));
	else
		// Positive
		return CString(OSSTR("+")) + CString(db, 0, 1) + CString(OSSTR("dB"));
}
