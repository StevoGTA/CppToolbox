//----------------------------------------------------------------------------------------------------------------------
//	CCoreGraphicsContext.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CCoreGraphicsContext.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreGraphicsContextInternals

class CCoreGraphicsContextInternals {
	public:
		CCoreGraphicsContextInternals(CBitmap& bitmap)
			{
				// Setup
				CGBitmapInfo	bitmapInfo;
				switch (bitmap.getFormat()) {
					case kBitmapFormatRGB888:	bitmapInfo = kCGImageAlphaNone;					break;
					case kBitmapFormatRGBA8888:	bitmapInfo = kCGImageAlphaPremultipliedLast;	break;
					case kBitmapFormatARGB8888:	bitmapInfo = kCGImageAlphaPremultipliedFirst;	break;
					default:					bitmapInfo = 0;									break;
				}

				// Create contextRef
				const	S2DSizeS32&		size = bitmap.getSize();
						CGColorSpaceRef	colorSpaceRef = ::CGColorSpaceCreateDeviceRGB();

				mContextRef =
						::CGBitmapContextCreate(bitmap.getPixelData().getMutableBytePtr(), size.mWidth, size.mHeight, 8,
								bitmap.getBytesPerRow(), colorSpaceRef, bitmapInfo);
				::CGColorSpaceRelease(colorSpaceRef);
			}
		~CCoreGraphicsContextInternals()
			{
				::CGContextRelease(mContextRef);
			}

		CGContextRef	mContextRef;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CCoreGraphicsContext

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CCoreGraphicsContext::CCoreGraphicsContext(CBitmap& bitmap)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CCoreGraphicsContextInternals(bitmap);
}

//----------------------------------------------------------------------------------------------------------------------
CCoreGraphicsContext::~CCoreGraphicsContext()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CCoreGraphicsContext::setFillColor(const CColor& color)
//----------------------------------------------------------------------------------------------------------------------
{
	::CGContextSetRGBFillColor(mInternals->mContextRef, color.getRed(), color.getGreen(), color.getBlue(),
			color.getAlpha());
}

//----------------------------------------------------------------------------------------------------------------------
void CCoreGraphicsContext::setStrokeColor(const CColor& color)
//----------------------------------------------------------------------------------------------------------------------
{
	::CGContextSetRGBStrokeColor(mInternals->mContextRef, color.getRed(), color.getGreen(), color.getBlue(),
			color.getAlpha());
}

//----------------------------------------------------------------------------------------------------------------------
void CCoreGraphicsContext::strokeLine(const S2DPointCGF& startPoint, const S2DPointCGF& endPoint, bool antiAlias,
CGFloat lineWidth)
//----------------------------------------------------------------------------------------------------------------------
{
	::CGContextMoveToPoint(mInternals->mContextRef, startPoint.mX, startPoint.mY);
	::CGContextAddLineToPoint(mInternals->mContextRef, endPoint.mX, endPoint.mY);
	::CGContextSetAllowsAntialiasing(mInternals->mContextRef, antiAlias);
	::CGContextSetLineWidth(mInternals->mContextRef, lineWidth);
	::CGContextStrokePath(mInternals->mContextRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CCoreGraphicsContext::strokeLines(const S2DPointCGF* points, UInt32 count, bool antiAlias,
CGFloat lineWidth)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertNotNil(points);
	if (points == nil)
		return;

	AssertFailIf(count <= 1);
	if (count <= 1)
		return;

	::CGContextAddLines(mInternals->mContextRef, (CGPoint*) points, count);
	::CGContextSetAllowsAntialiasing(mInternals->mContextRef, antiAlias);
	::CGContextSetLineWidth(mInternals->mContextRef, lineWidth);
	::CGContextStrokePath(mInternals->mContextRef);
}
