//----------------------------------------------------------------------------------------------------------------------
//	CCoreMediaVideoCodec+OpenGLES.cpp			©2023 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CCoreMediaVideoCodec.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreMediaDecodeVideoCodec

// MARK: Private methods

//----------------------------------------------------------------------------------------------------------------------
void CCoreMediaDecodeVideoCodec::setCompatibility(CFMutableDictionaryRef dictionaryRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update
	::CFDictionarySetValue(dictionaryRef, kCVPixelBufferOpenGLESCompatibilityKey, kCFBooleanTrue);
}
