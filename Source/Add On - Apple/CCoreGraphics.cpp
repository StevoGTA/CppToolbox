//----------------------------------------------------------------------------------------------------------------------
//	CCoreGraphics.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CCoreGraphics.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreGraphics

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CGImageRef CCoreGraphics::newImageRef(const CBitmap& bitmap)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	S2DSizeS32&	size = bitmap.getSize();
	const	CData&		data = bitmap.getPixelData();

	CGBitmapInfo	bitmapInfo;
	switch (bitmap.getFormat()) {
		case CBitmap::kFormatRGB888:	bitmapInfo = kCGImageAlphaNone;					break;
		case CBitmap::kFormatRGBA8888:	bitmapInfo = kCGImageAlphaPremultipliedLast;	break;
		case CBitmap::kFormatARGB8888:	bitmapInfo = kCGImageAlphaPremultipliedFirst;	break;
		default:						bitmapInfo = 0;									break;
	}

	CGColorSpaceRef		colorSpaceRef = ::CGColorSpaceCreateDeviceRGB();
	CGDataProviderRef	dataProviderRef =
								::CGDataProviderCreateWithData(nil, data.getBytePtr(),
										bitmap.getBytesPerRow() * size.mHeight, nil);

	CGImageRef	imageRef =
						::CGImageCreate(size.mWidth, size.mHeight, 8, bitmap.getBytesPerPixel() * 8,
								bitmap.getBytesPerRow(), colorSpaceRef, bitmapInfo, dataProviderRef, nil, 1,
								kCGRenderingIntentDefault);
	::CGDataProviderRelease(dataProviderRef);
	::CGColorSpaceRelease(colorSpaceRef);

	return imageRef;
}
