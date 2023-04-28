//----------------------------------------------------------------------------------------------------------------------
//	CAACAudioCodecUseDefault.cpp			©2023 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAACAudioCodec.h"
#include "CCodecRegistry.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Register audio codecs

REGISTER_AUDIO_CODEC(aacLC, CCodec::Info(CAACAudioCodec::mAACLCID, CString(OSSTR("AAC Low Complexity"))));
REGISTER_AUDIO_CODEC(aacLD, CCodec::Info(CAACAudioCodec::mAACLDID, CString(OSSTR("AAC Low Delay"))));
