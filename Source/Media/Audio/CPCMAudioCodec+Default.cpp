//----------------------------------------------------------------------------------------------------------------------
//	CPCMAudioCodec+Default.cpp			©2023 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CCodecRegistry.h"
#include "CPCMAudioCodec.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Register audio codecs

REGISTER_AUDIO_CODEC(pcmFloat, CCodec::Info(CPCMAudioCodec::mFloatID, CString(OSSTR("None (Floating Point)"))));
REGISTER_AUDIO_CODEC(pcmInteger, CCodec::Info(CPCMAudioCodec::mIntegerID, CString(OSSTR("None (Integer)"))));
