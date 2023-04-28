//----------------------------------------------------------------------------------------------------------------------
//	CIMAADPCMAudioCodecUseDefault.cpp			©2023 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CCodecRegistry.h"
#include "CIMAADPCMAudioCodec.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Register audio codec

REGISTER_AUDIO_CODEC(dviIntelIMA,
		CCodec::Info(CDVIIntelIMAADPCMAudioCodec::mID, CString(OSSTR("DVI/Intel IMA ADPCM 4:1"))));
