//----------------------------------------------------------------------------------------------------------------------
//	CCoreMediaVideoCodec+OpenGL.cpp			©2023 Stevo Brock	All rights reserved.
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
	::CFDictionarySetValue(dictionaryRef, kCVPixelBufferOpenGLCompatibilityKey, kCFBooleanTrue);
	::CFDictionarySetValue(dictionaryRef, kCVPixelBufferOpenGLTextureCacheCompatibilityKey, kCFBooleanTrue);

	OSType		pixelFormat = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
	CFNumberRef	numberRef = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &pixelFormat);
	::CFDictionarySetValue(dictionaryRef, kCVPixelBufferPixelFormatTypeKey, numberRef);
	::CFRelease(numberRef);
}
