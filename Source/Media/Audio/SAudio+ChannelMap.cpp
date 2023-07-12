//----------------------------------------------------------------------------------------------------------------------
//	SAudio+ChannelMap.cpp			©2023 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SAudio.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SAudio::ChannelMap

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CString SAudio::ChannelMap::getDisplayString() const
//----------------------------------------------------------------------------------------------------------------------
{
	// 1 Channel
	if (*this == m1_0)					return CString(OSSTR("C"));

	// 2 Channels
	else if (*this == m2_0_Option1)		return CString(OSSTR("L R"));
	else if (*this == m1_1_Option1)		return CString(OSSTR("C LFE"));
	else if (*this == m2_0_Option2)		return CString(OSSTR("Ls Rs"));
	else if (*this == m2_0_Option3)		return CString(OSSTR("Lt Rt (Matrix encoded stereo stream)"));
//	else if (*this == m2_0_Option4)		return CString(OSSTR("Mide/Side"));
//	else if (*this == m2_0_Option5)		return CString(OSSTR("Coincident Mic Pair (often 2 figure 8s)"));
//	else if (*this == m2_0_Option6)		return CString(OSSTR("Binaural Stereo (L R)"));

	// 3 Channels
	else if (*this == m3_0_Unknown)		return CString(OSSTR("Unknown 3 channel"));
	else if (*this == m2_1_Option1)		return CString(OSSTR("L R LFE"));
	else if (*this == m3_0_Option1)		return CString(OSSTR("L R C"));
	else if (*this == m3_0_Option2)		return CString(OSSTR("C L R"));
	else if (*this == m3_0_Option3)		return CString(OSSTR("L R Cs"));
	else if (*this == m3_0_Option4)		return CString(OSSTR("L C R"));

	// 4 Channels
	else if (*this == m4_0_Unknown)		return CString(OSSTR("Unknown 4 channel"));
	else if (*this == m3_1_Option1)		return CString(OSSTR("L R LFE Cs"));
	else if (*this == m3_1_Option2)		return CString(OSSTR("L R C LFE"));
	else if (*this == m3_1_Option3)		return CString(OSSTR("L C R LFE"));
	else if (*this == m3_1_Option4)		return CString(OSSTR("L R Cs LFE"));
	else if (*this == m4_0_Option1)		return CString(OSSTR("L R Ls Rs (Quadraphonic)"));
	else if (*this == m4_0_Option2)		return CString(OSSTR("L R C Cs"));
	else if (*this == m4_0_Option3)		return CString(OSSTR("C L R Cs"));
	else if (*this == m4_0_Option4)		return CString(OSSTR("L C R Cs"));
//	else if (*this == m4_0_Option5)		return CString(OSSTR("Ambisonic B (W X Y Z)"));

	// 5 Channels
	else if (*this == m5_0_Unknown)		return CString(OSSTR("Unknown 5 channel"));
	else if (*this == m4_1_Option1)		return CString(OSSTR("L R LFE Ls Rs"));
	else if (*this == m4_1_Option2)		return CString(OSSTR("L R C LFE Cs"));
	else if (*this == m4_1_Option3)		return CString(OSSTR("L R Ls Rs LFE"));
	else if (*this == m4_1_Option4)		return CString(OSSTR("L C R Cs LFE"));
	else if (*this == m5_0_Option1)		return CString(OSSTR("L R C Ls Rs"));
	else if (*this == m5_0_Option2)		return CString(OSSTR("L R Ls Rs C (Pentagonal)"));
	else if (*this == m5_0_Option3)		return CString(OSSTR("L C R Ls Rs"));
	else if (*this == m5_0_Option4)		return CString(OSSTR("C L R Ls Rs"));

	// 6 Channels
	else if (*this == m6_0_Unknown)		return CString(OSSTR("Unknown 6 channel"));
	else if (*this == m5_1_Option1)		return CString(OSSTR("L R C LFE Ls Rs"));
	else if (*this == m5_1_Option2)		return CString(OSSTR("L R Ls Rs C LFE"));
	else if (*this == m5_1_Option3)		return CString(OSSTR("L C R Ls Rs LFE"));
	else if (*this == m5_1_Option4)		return CString(OSSTR("C L R Ls Rs LFE"));
	else if (*this == m6_0_Option1)		return CString(OSSTR("L R Ls Rs C Cs (Hexagonal)"));
	else if (*this == m6_0_Option2)		return CString(OSSTR("C L R Ls Rs Cs"));

	// 7 Channels
	else if (*this == m7_0_Unknown)		return CString(OSSTR("Unknown 7 channel"));
	else if (*this == m6_1_Option1)		return CString(OSSTR("L R C LFE Ls Rs Cs"));
	else if (*this == m6_1_Option2)		return CString(OSSTR("C L R Ls Rs Cs LFE"));
	else if (*this == m6_1_Option3)		return CString(OSSTR("L C R Ls Cs Rs LFE"));
	else if (*this == m7_0_Option1)		return CString(OSSTR("L R Ls Rs C Rls Rrs"));
	else if (*this == m7_0_Option2)		return CString(OSSTR("C L R Ls Rs Rls Rrs"));
	else if (*this == m7_0_Option3)		return CString(OSSTR("L R Ls Rs C Lc Rc"));

	// 8 Channels
	else if (*this == m8_0_Unknown)		return CString(OSSTR("Unknown 8 channel"));
	else if (*this == m7_1_Option1)		return CString(OSSTR("L R C LFE Ls Rs Lc Rc"));
	else if (*this == m7_1_Option2)		return CString(OSSTR("C Lc Rc L R Ls Rs LFE (doc: IS-13818-7 MPEG2-AAC Table 3.1)"));
	else if (*this == m7_1_Option3)		return CString(OSSTR("L R C LFE Ls Rs Rls Rrs"));
	else if (*this == m7_1_Option4)		return CString(OSSTR("L R Ls Rs C LFE Lc Rc (Emagic Default 7.1)"));
	else if (*this == m7_1_Option5)		return CString(OSSTR("L R C LFE Ls Rs Lt Rt (SMPTE DTV)"));
	else if (*this == m7_1_Option6)		return CString(OSSTR("L Lc C Rc R Ls Rs LFE"));
	else if (*this == m8_0_Option1)		return CString(OSSTR("L R Ls Rs C Cs Lw Rw (Octagonal)"));
	else if (*this == m8_0_Option2)		return CString(OSSTR("C L R Ls Rs Rls Rrs Cs"));
//	else if (*this == m8_0_Option2)		return CString(OSSTR("L R Ls Rs topLeft topRight topRearLeft topRearRight (Cube)"));
	else if (*this == m7_1_Option7)		return CString(OSSTR("C L R Ls Rs LFE Lt Rt (From Fotokem)"));

	// 9 Channels
	else if (*this == m9_0_Unknown)		return CString(OSSTR("Unknown 9 channel"));

	// 10 Channels
	else if (*this == m10_0_Unknown)	return CString(OSSTR("Unknown 10 channel"));

	// Unknown
	else								 return CString(OSSTR("Unknown"));
}
