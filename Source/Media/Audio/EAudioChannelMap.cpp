//----------------------------------------------------------------------------------------------------------------------
//	EAudioChannelMap.cpp			©2023 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SAudioFormats.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: EAudioChannelMap

//----------------------------------------------------------------------------------------------------------------------
const CString eAudioChannelMapGetDescription(EAudioChannelMap audioChannelMap)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check channel map
	switch (audioChannelMap) {
		// 1 Channel
		case kAudioChannelMap_1_0:			return CString(OSSTR("C"));

		// 2 Channels
		case kAudioChannelMap_2_0_Option1:	return CString(OSSTR("L R"));
		case kAudioChannelMap_1_1_Option1:	return CString(OSSTR("C LFE"));
		case kAudioChannelMap_2_0_Option2:	return CString(OSSTR("Ls Rs"));
		case kAudioChannelMap_2_0_Option3:	return CString(OSSTR("Lt Rt (Matrix encoded stereo stream)"));
//		case kAudioChannelMap_2_0_Option4:	return CString(OSSTR("Mide/Side"));
//		case kAudioChannelMap_2_0_Option5:	return CString(OSSTR("Coincident Mic Pair (often 2 figure 8s)"));
//		case kAudioChannelMap_2_0_Option6:	return CString(OSSTR("Binaural Stereo (L R)"));

		// 3 Channels
		case kAudioChannelMap_3_0_Unknown:	return CString(OSSTR("Unknown 3 channel"));
		case kAudioChannelMap_2_1_Option1:	return CString(OSSTR("L R LFE"));
		case kAudioChannelMap_3_0_Option1:	return CString(OSSTR("L R C"));
		case kAudioChannelMap_3_0_Option2:	return CString(OSSTR("C L R"));
		case kAudioChannelMap_3_0_Option3:	return CString(OSSTR("L R Cs"));
		case kAudioChannelMap_3_0_Option4:	return CString(OSSTR("L C R"));

		// 4 Channels
		case kAudioChannelMap_4_0_Unknown:	return CString(OSSTR("Unknown 4 channel"));
		case kAudioChannelMap_3_1_Option1:	return CString(OSSTR("L R LFE Cs"));
		case kAudioChannelMap_3_1_Option2:	return CString(OSSTR("L R C LFE"));
		case kAudioChannelMap_3_1_Option3:	return CString(OSSTR("L C R LFE"));
		case kAudioChannelMap_3_1_Option4:	return CString(OSSTR("L R Cs LFE"));
		case kAudioChannelMap_4_0_Option1:	return CString(OSSTR("L R Ls Rs (Quadraphonic)"));
		case kAudioChannelMap_4_0_Option2:	return CString(OSSTR("L R C Cs"));
		case kAudioChannelMap_4_0_Option3:	return CString(OSSTR("C L R Cs"));
		case kAudioChannelMap_4_0_Option4:	return CString(OSSTR("L C R Cs"));
//		case kAudioChannelMap_4_0_Option5:	return CString(OSSTR("Ambisonic B (W X Y Z)"));

		// 5 Channels
		case kAudioChannelMap_5_0_Unknown:	return CString(OSSTR("Unknown 5 channel"));
		case kAudioChannelMap_4_1_Option1:	return CString(OSSTR("L R LFE Ls Rs"));
		case kAudioChannelMap_4_1_Option2:	return CString(OSSTR("L R C LFE Cs"));
		case kAudioChannelMap_4_1_Option3:	return CString(OSSTR("L R Ls Rs LFE"));
		case kAudioChannelMap_4_1_Option4:	return CString(OSSTR("L C R Cs LFE"));
		case kAudioChannelMap_5_0_Option1:	return CString(OSSTR("L R C Ls Rs"));
		case kAudioChannelMap_5_0_Option2:	return CString(OSSTR("L R Ls Rs C (Pentagonal)"));
		case kAudioChannelMap_5_0_Option3:	return CString(OSSTR("L C R Ls Rs"));
		case kAudioChannelMap_5_0_Option4:	return CString(OSSTR("C L R Ls Rs"));

		// 6 Channels
		case kAudioChannelMap_6_0_Unknown:	return CString(OSSTR("Unknown 6 channel"));
		case kAudioChannelMap_5_1_Option1:	return CString(OSSTR("L R C LFE Ls Rs"));
		case kAudioChannelMap_5_1_Option2:	return CString(OSSTR("L R Ls Rs C LFE"));
		case kAudioChannelMap_5_1_Option3:	return CString(OSSTR("L C R Ls Rs LFE"));
		case kAudioChannelMap_5_1_Option4:	return CString(OSSTR("C L R Ls Rs LFE"));
		case kAudioChannelMap_6_0_Option1:	return CString(OSSTR("L R Ls Rs C Cs (Hexagonal)"));
		case kAudioChannelMap_6_0_Option2:	return CString(OSSTR("C L R Ls Rs Cs"));

		// 7 Channels
		case kAudioChannelMap_7_0_Unknown:	return CString(OSSTR("Unknown 7 channel"));
		case kAudioChannelMap_6_1_Option1:	return CString(OSSTR("L R C LFE Ls Rs Cs"));
		case kAudioChannelMap_6_1_Option2:	return CString(OSSTR("C L R Ls Rs Cs LFE"));
		case kAudioChannelMap_6_1_Option3:	return CString(OSSTR("L C R Ls Cs Rs LFE"));
		case kAudioChannelMap_7_0_Option1:	return CString(OSSTR("L R Ls Rs C Rls Rrs"));
		case kAudioChannelMap_7_0_Option2:	return CString(OSSTR("C L R Ls Rs Rls Rrs"));
		case kAudioChannelMap_7_0_Option3:	return CString(OSSTR("L R Ls Rs C Lc Rc"));

		// 8 Channels
		case kAudioChannelMap_8_0_Unknown:	return CString(OSSTR("Unknown 8 channel"));
		case kAudioChannelMap_7_1_Option1:	return CString(OSSTR("L R C LFE Ls Rs Lc Rc"));
		case kAudioChannelMap_7_1_Option2:	return CString(OSSTR("C Lc Rc L R Ls Rs LFE (doc: IS-13818-7 MPEG2-AAC Table 3.1)"));
		case kAudioChannelMap_7_1_Option3:	return CString(OSSTR("L R C LFE Ls Rs Rls Rrs"));
		case kAudioChannelMap_7_1_Option4:	return CString(OSSTR("L R Ls Rs C LFE Lc Rc (Emagic Default 7.1)"));
		case kAudioChannelMap_7_1_Option5:	return CString(OSSTR("L R C LFE Ls Rs Lt Rt (SMPTE DTV)"));
		case kAudioChannelMap_7_1_Option6:	return CString(OSSTR("L Lc C Rc R Ls Rs LFE"));
		case kAudioChannelMap_8_0_Option1:	return CString(OSSTR("L R Ls Rs C Cs Lw Rw (Octagonal)"));
		case kAudioChannelMap_8_0_Option2:	return CString(OSSTR("C L R Ls Rs Rls Rrs Cs"));
//		case kAudioChannelMap_8_0_Option2:	return CString(OSSTR("L R Ls Rs topLeft topRight topRearLeft topRearRight (Cube)"));
		case kAudioChannelMap_7_1_Option7:	return CString(OSSTR("C L R Ls Rs LFE Lt Rt (From Fotokem)"));

		// 9 Channels
		case kAudioChannelMap_9_0_Unknown:	return CString(OSSTR("Unknown 9 channel"));

		// 10 Channels
		case kAudioChannelMap_10_0_Unknown:	return CString(OSSTR("Unknown 10 channel"));

#if defined(TARGET_OS_WINDOWS)
		default:							return CString::mEmpty;
#endif
	}
}
