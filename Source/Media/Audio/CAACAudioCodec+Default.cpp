//----------------------------------------------------------------------------------------------------------------------
//	CAACAudioCodec+Default.cpp			©2023 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAACAudioCodec.h"
#include "CCodecRegistry.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Register audio codecs

REGISTER_AUDIO_CODEC(aacLC, CCodec::Info(CAACAudioCodec::mLCID, CString(OSSTR("AAC Low Complexity"))));
REGISTER_AUDIO_CODEC(aacLD, CCodec::Info(CAACAudioCodec::mLDID, CString(OSSTR("AAC Low Delay"))));
