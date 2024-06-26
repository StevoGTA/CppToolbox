//----------------------------------------------------------------------------------------------------------------------
//	CBitmap-Apple.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CBitmap.h"

#include "CppToolboxAssert.h"

#include <Accelerate/Accelerate.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local proc declarations

static	UInt32	sGetPixelData32ForColor(CBitmap::Format bitmapFormat, const CColor& color);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBitmap

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CBitmap::clearPixels(const S2DRectS32& rect)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	S2DSizeS32	bitmapSize = getSize();

	// Parameter check
	AssertFailIf((rect.mOrigin.mX < 0) || (rect.mOrigin.mX >= bitmapSize.mWidth));
	if ((rect.mOrigin.mX < 0) || (rect.mOrigin.mX >= bitmapSize.mWidth))
		return;

	AssertFailIf((rect.mOrigin.mY < 0) || (rect.mOrigin.mY >= bitmapSize.mHeight));
	if ((rect.mOrigin.mY < 0) || (rect.mOrigin.mY >= bitmapSize.mHeight))
		return;

	AssertFailIf(((rect.mOrigin.mX + rect.getWidth()) < 0) ||
			((rect.mOrigin.mX + rect.getWidth()) > bitmapSize.mWidth));
	if (((rect.mOrigin.mX + rect.getWidth()) < 0) ||
			((rect.mOrigin.mX + rect.getWidth()) > bitmapSize.mWidth))
		return;

	AssertFailIf(((rect.mOrigin.mY + rect.getHeight()) < 0) ||
			((rect.mOrigin.mY + rect.getHeight()) > bitmapSize.mHeight));
	if (((rect.mOrigin.mY + rect.getHeight()) < 0) ||
			((rect.mOrigin.mY + rect.getHeight()) > bitmapSize.mHeight))
		return;

	AssertFailIf(rect.getWidth() < 0);
	if (rect.getWidth() < 0)
		return;

	AssertFailIf(rect.getHeight() < 0);
	if (rect.getHeight() < 0)
		return;

	// Are we really clearing any pixesl?
	if ((rect.getWidth() == 0) || (rect.getHeight() == 0))
		// No
		return;

	// Setup bitmap context
	CGColorSpaceRef	colorSpaceRef = ::CGColorSpaceCreateDeviceRGB();

	CGBitmapInfo	bitmapInfo;
	switch (getFormat()) {
		case CBitmap::kFormatRGB888:	bitmapInfo = kCGImageAlphaNone;					break;
		case CBitmap::kFormatRGBA8888:	bitmapInfo = kCGImageAlphaPremultipliedLast;	break;
		case CBitmap::kFormatARGB8888:	bitmapInfo = kCGImageAlphaPremultipliedFirst;	break;
		default:						bitmapInfo = 0;									break;
	}

	CGContextRef	bitmapContextRef =
							::CGBitmapContextCreate(getPixelData().getMutableBytePtr(), bitmapSize.mWidth,
									bitmapSize.mHeight, 8, getBytesPerRow(), colorSpaceRef, bitmapInfo);
	::CGColorSpaceRelease(colorSpaceRef);

	// Clear pixels
	::CGContextClearRect(bitmapContextRef,
			::CGRectMake(rect.mOrigin.mX, rect.mOrigin.mY, rect.getWidth(), rect.getHeight()));
	::CGContextRelease(bitmapContextRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CBitmap::setPixels(const S2DRectS32& rect, const CColor& color)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	S2DSizeS32	bitmapSize = getSize();

	// Parameter check
	AssertFailIf((rect.mOrigin.mX < 0) || (rect.mOrigin.mX >= bitmapSize.mWidth));
	if ((rect.mOrigin.mX < 0) || (rect.mOrigin.mX >= bitmapSize.mWidth))
		return;

	AssertFailIf((rect.mOrigin.mY < 0) || (rect.mOrigin.mY >= bitmapSize.mHeight));
	if ((rect.mOrigin.mY < 0) || (rect.mOrigin.mY >= bitmapSize.mHeight))
		return;

	AssertFailIf(((rect.mOrigin.mX + rect.getWidth()) < 0) ||
			((rect.mOrigin.mX + rect.getWidth()) > bitmapSize.mWidth));
	if (((rect.mOrigin.mX + rect.getWidth()) < 0) ||
			((rect.mOrigin.mX + rect.getWidth()) > bitmapSize.mWidth))
		return;

	AssertFailIf(((rect.mOrigin.mY + rect.getHeight()) < 0) ||
			((rect.mOrigin.mY + rect.getHeight()) > bitmapSize.mHeight));
	if (((rect.mOrigin.mY + rect.getHeight()) < 0) ||
			((rect.mOrigin.mY + rect.getHeight()) > bitmapSize.mHeight))
		return;

	AssertFailIf(rect.getWidth() < 0);
	if (rect.getWidth() < 0)
		return;

	AssertFailIf(rect.getHeight() < 0);
	if (rect.getHeight() < 0)
		return;

	// Are we really setting any pixesl?
	if ((rect.getWidth() == 0) || (rect.getHeight() == 0))
		// No
		return;

	// Do it
	if (getBytesPerPixel() == 4) {
		// Use Accelerate Framework
		vImage_Buffer	buffer;
		buffer.width = rect.getWidth();
		buffer.height = rect.getHeight();
		buffer.rowBytes = getBytesPerRow();
		buffer.data =
				(UInt8*) getPixelData().getMutableBytePtr() + rect.mOrigin.mY * getBytesPerRow() +
						rect.mOrigin.mX * getBytesPerPixel();

		UInt32		pixelData = sGetPixelData32ForColor(getFormat(), color);
		Pixel_8888* pixel = (Pixel_8888*) &pixelData;
 		vImageBufferFill_ARGB8888(&buffer, *pixel, 0);
	} else {
		// Setup bitmap context
		CGColorSpaceRef	colorSpaceRef = ::CGColorSpaceCreateDeviceRGB();

		CGBitmapInfo	bitmapInfo;
		switch (getFormat()) {
			case CBitmap::kFormatRGB888:	bitmapInfo = kCGImageAlphaNone;					break;
			case CBitmap::kFormatRGBA8888:	bitmapInfo = kCGImageAlphaPremultipliedLast;	break;
			case CBitmap::kFormatARGB8888:	bitmapInfo = kCGImageAlphaPremultipliedFirst;	break;
			default:						bitmapInfo = 0;									break;
		}

		CGContextRef	bitmapContextRef =
								::CGBitmapContextCreate(getPixelData().getMutableBytePtr(), bitmapSize.mWidth,
										bitmapSize.mHeight, 8, getBytesPerRow(), colorSpaceRef, bitmapInfo);
		::CGColorSpaceRelease(colorSpaceRef);

		// Set pixels
		CColor::RGBValues	rgbValues = color.getRGBValues();
		::CGContextSetRGBFillColor(bitmapContextRef, rgbValues.getRed(), rgbValues.getGreen(), rgbValues.getBlue(),
				rgbValues.getAlpha());
		::CGContextFillRect(bitmapContextRef,
				::CGRectMake(rect.mOrigin.mX, bitmapSize.mHeight - rect.mOrigin.mY, rect.getWidth(),
						-rect.getHeight()));
		::CGContextRelease(bitmapContextRef);
	}
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
UInt32 sGetPixelData32ForColor(CBitmap::Format bitmapFormat, const CColor& color)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check format
	CColor::RGBValues	rgbValues = color.getRGBValues();
	switch (bitmapFormat) {
		case CBitmap::kFormatRGBA8888:
			// RGBA 8888
			return EndianU32_BtoN(
					((UInt32) (rgbValues.getRed() * 255.0) << 24) |
					((UInt32) (rgbValues.getGreen() * 255.0) << 16) |
					((UInt32) (rgbValues.getBlue() * 255.0) << 8) |
					(UInt32) (rgbValues.getAlpha() * 255.0));

		case CBitmap::kFormatARGB8888:
			// ARGB 8888
			return EndianU32_BtoN(
					((UInt32) (rgbValues.getAlpha() * 255.0) << 24) |
					((UInt32) (rgbValues.getRed() * 255.0) << 16) |
					((UInt32) (rgbValues.getGreen() * 255.0) << 8) |
					(UInt32) (rgbValues.getBlue() * 255.0));

		default:
			return 0;
	}
}
