//----------------------------------------------------------------------------------------------------------------------
//	CCoreGraphicsRenderer.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CCoreGraphicsRenderer.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreGraphicsRenderer::Internals

class CCoreGraphicsRenderer::Internals {
	public:
		Internals(CBitmap& bitmap)
			{
				// Setup
				CGBitmapInfo	bitmapInfo;
				switch (bitmap.getFormat()) {
					case CBitmap::kFormatRGB888:	bitmapInfo = kCGImageAlphaNone;					break;
					case CBitmap::kFormatRGBA8888:	bitmapInfo = kCGImageAlphaPremultipliedLast;	break;
					case CBitmap::kFormatARGB8888:	bitmapInfo = kCGImageAlphaPremultipliedFirst;	break;
					default:						bitmapInfo = 0;									break;
				}

				// Create contextRef
				const	S2DSizeS32&		size = bitmap.getSize();
						CGColorSpaceRef	colorSpaceRef = ::CGColorSpaceCreateDeviceRGB();

				mContextRef =
						::CGBitmapContextCreate(bitmap.getPixelData().getMutableBytePtr(), size.mWidth, size.mHeight, 8,
								bitmap.getBytesPerRow(), colorSpaceRef, bitmapInfo);
				::CGColorSpaceRelease(colorSpaceRef);
			}
		~Internals()
			{
				::CGContextRelease(mContextRef);
			}

		CGContextRef	mContextRef;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CCoreGraphicsRenderer

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CCoreGraphicsRenderer::CCoreGraphicsRenderer(CBitmap& bitmap)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(bitmap);
}

//----------------------------------------------------------------------------------------------------------------------
CCoreGraphicsRenderer::~CCoreGraphicsRenderer()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: C2DRenderer methods

//----------------------------------------------------------------------------------------------------------------------
void CCoreGraphicsRenderer::setFillColor(const CColor& color)
//----------------------------------------------------------------------------------------------------------------------
{
	::CGContextSetRGBFillColor(mInternals->mContextRef, color.getRed(), color.getGreen(), color.getBlue(),
			color.getAlpha());
}

//----------------------------------------------------------------------------------------------------------------------
void CCoreGraphicsRenderer::setStrokeColor(const CColor& color)
//----------------------------------------------------------------------------------------------------------------------
{
	::CGContextSetRGBStrokeColor(mInternals->mContextRef, color.getRed(), color.getGreen(), color.getBlue(),
			color.getAlpha());
}

// MARK: T2DRenderer methods

//----------------------------------------------------------------------------------------------------------------------
void CCoreGraphicsRenderer::strokeLine(const CCoreGraphics::Point& startPoint, const CCoreGraphics::Point& endPoint,
		bool antiAlias, CGFloat lineWidth)
//----------------------------------------------------------------------------------------------------------------------
{
	::CGContextMoveToPoint(mInternals->mContextRef, startPoint.mX, startPoint.mY);
	::CGContextAddLineToPoint(mInternals->mContextRef, endPoint.mX, endPoint.mY);
	::CGContextSetAllowsAntialiasing(mInternals->mContextRef, antiAlias);
	::CGContextSetLineWidth(mInternals->mContextRef, lineWidth);
	::CGContextStrokePath(mInternals->mContextRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CCoreGraphicsRenderer::strokeLines(const CCoreGraphics::Point* points, UInt32 count, bool antiAlias,
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
