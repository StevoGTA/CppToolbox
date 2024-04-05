//----------------------------------------------------------------------------------------------------------------------
//	CCoreGraphicsRenderer.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CCoreGraphicsRenderer.h"

#include <CoreText/CoreText.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

struct SRGBInterpolateColorInfo {
	CGFloat	mColor1Red;
	CGFloat	mColor1Green;
	CGFloat	mColor1Blue;
	CGFloat	mColor1Alpha;

	CGFloat	mColor2Red;
	CGFloat	mColor2Green;
	CGFloat	mColor2Blue;
	CGFloat	mColor2Alpha;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

static	void	sInterpolateSRGBInterpolateColorInfo(SRGBInterpolateColorInfo *info, const CGFloat* in,
						CGFloat* output);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CCoreGraphicsRenderer::Internals

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
		Internals(CGContextRef contextRef) : mContextRef(::CGContextRetain(contextRef)) {}
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
CCoreGraphicsRenderer::CCoreGraphicsRenderer(CGContextRef contextRef)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(contextRef);
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
	CColor::RGBValues	rgbValues = color.getRGBValues();
	::CGContextSetRGBFillColor(mInternals->mContextRef, rgbValues.getRed(), rgbValues.getGreen(), rgbValues.getBlue(),
			rgbValues.getAlpha());
}

//----------------------------------------------------------------------------------------------------------------------
void CCoreGraphicsRenderer::setStrokeColor(const CColor& color)
//----------------------------------------------------------------------------------------------------------------------
{
	CColor::RGBValues	rgbValues = color.getRGBValues();
	::CGContextSetRGBStrokeColor(mInternals->mContextRef, rgbValues.getRed(), rgbValues.getGreen(), rgbValues.getBlue(),
			rgbValues.getAlpha());
}

// MARK: T2DRenderer methods

//----------------------------------------------------------------------------------------------------------------------
void CCoreGraphicsRenderer::strokeLine(const S2DPointF32& startPoint, const S2DPointF32& endPoint, bool antiAlias,
		Float32 lineWidth) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Stroke line
	::CGContextMoveToPoint(mInternals->mContextRef, startPoint.mX, startPoint.mY);
	::CGContextAddLineToPoint(mInternals->mContextRef, endPoint.mX, endPoint.mY);
	::CGContextSetAllowsAntialiasing(mInternals->mContextRef, antiAlias);
	::CGContextSetLineWidth(mInternals->mContextRef, lineWidth);
	::CGContextStrokePath(mInternals->mContextRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CCoreGraphicsRenderer::strokeLines(const S2DPointF32* points, UInt32 count, bool antiAlias, Float32 lineWidth)
		const
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertNotNil(points);
	if (points == nil)
		return;

	AssertFailIf(count <= 1);
	if (count <= 1)
		return;

	// Setup
	CGPoint	pointsUse[count];
	for (UInt32 i = 0; i < count; i++, pointsUse[i] = CGPointMake(points[i].mX, points[i].mY)) ;

	// Stroke lines
	::CGContextAddLines(mInternals->mContextRef, pointsUse, count);
	::CGContextSetAllowsAntialiasing(mInternals->mContextRef, antiAlias);
	::CGContextSetLineWidth(mInternals->mContextRef, lineWidth);
	::CGContextStrokePath(mInternals->mContextRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CCoreGraphicsRenderer::strokeRect(const S2DRectF32& rect) const
//----------------------------------------------------------------------------------------------------------------------
{
	::CGContextStrokeRect(mInternals->mContextRef, CCoreGraphics::cgRectFor(rect));
}

//----------------------------------------------------------------------------------------------------------------------
void CCoreGraphicsRenderer::fillRect(const S2DRectF32& rect) const
//----------------------------------------------------------------------------------------------------------------------
{
	::CGContextFillRect(mInternals->mContextRef, CCoreGraphics::cgRectFor(rect));
}

//----------------------------------------------------------------------------------------------------------------------
void CCoreGraphicsRenderer::shadeRect(const S2DRectF32& rect, const S2DPointF32& shadeStartPoint,
		const S2DPointF32& shadeEndPoint, const CColor& startColor, const CColor& endColor) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Clip
	::CGContextSaveGState(mInternals->mContextRef);
	::CGContextClipToRect(mInternals->mContextRef, CCoreGraphics::cgRectFor(rect));

	// Setup
	CColor::RGBValues	startColorRGBValues = startColor.getRGBValues();
	CColor::RGBValues	endColorRGBValues = endColor.getRGBValues();
	CGColorSpaceRef		colorSpaceRef = ::CGColorSpaceCreateDeviceRGB();

	CGFloat					domain[2] = {0.0, 1.0};
	CGFloat					range[8] = {0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0};
	CGFunctionCallbacks		callbacks = {0, (CGFunctionEvaluateCallback) sInterpolateSRGBInterpolateColorInfo, nil};

	SRGBInterpolateColorInfo	info;
	info.mColor1Red = startColorRGBValues.getRed();
	info.mColor1Green = startColorRGBValues.getGreen();
	info.mColor1Blue = startColorRGBValues.getBlue();
	info.mColor1Alpha = startColorRGBValues.getAlpha();

	info.mColor2Red = endColorRGBValues.getRed();
	info.mColor2Green = endColorRGBValues.getGreen();
	info.mColor2Blue = endColorRGBValues.getBlue();
	info.mColor2Alpha = endColorRGBValues.getAlpha();

	CGFunctionRef	functionRef = ::CGFunctionCreate(&info, 1, domain, 4, range, &callbacks);
	CGShadingRef	shadingRef =
							::CGShadingCreateAxial(colorSpaceRef, CCoreGraphics::cgPointFor(shadeStartPoint),
									CCoreGraphics::cgPointFor(shadeEndPoint), functionRef, true, true);

	// Do it
	::CGContextDrawShading(mInternals->mContextRef, shadingRef);

	// Unclip
	::CGContextRestoreGState(mInternals->mContextRef);

	// Cleanup
	::CFRelease(colorSpaceRef);
	::CFRelease(functionRef);
	::CFRelease(shadingRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CCoreGraphicsRenderer::strokePath(const S2DPath32& path, const S2DAffineTransformF32& affineTransform) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CGPathRef	pathRef = CCoreGraphics::newPathRef(path, affineTransform);

	// Stroke
	::CGContextAddPath(mInternals->mContextRef, pathRef);
	::CGContextStrokePath(mInternals->mContextRef);

	// Cleanup
	::CGPathRelease(pathRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CCoreGraphicsRenderer::fillText(const CString& string, const Font& font, const S2DPointF32& point,
		TextPositioning textPositioning) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CTFontRef		fontRef = ::CTFontCreateWithName(font.getName().getOSString(), font.getSize(), nil);
	CFStringRef		keys[] =
						{
							kCTFontAttributeName,
							kCTForegroundColorFromContextAttributeName,
						};
	CFTypeRef		values[] =
						{
							fontRef,
							kCFBooleanTrue,
						};
	CFDictionaryRef	attributesDictionaryRef =
							::CFDictionaryCreate(kCFAllocatorDefault, (const void**) &keys,
									(const void**) &values, sizeof(keys) / sizeof(keys[0]),
									&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	::CFRelease(fontRef);

	CFAttributedStringRef	attributedStringRef =
									::CFAttributedStringCreate(kCFAllocatorDefault, string.getOSString(),
											attributesDictionaryRef);
	::CFRelease(attributesDictionaryRef);

	CTLineRef	lineRef = ::CTLineCreateWithAttributedString(attributedStringRef);
	::CFRelease(attributedStringRef);

	CGRect	imageBounds = ::CTLineGetImageBounds(lineRef, mInternals->mContextRef);
	switch (textPositioning) {
		case kTextPositioningTowardTrailingAbove:
			// Toward trailing, above
			::CGContextSetTextPosition(mInternals->mContextRef, point.mX, point.mY);
			break;

		case kTextPositioningTowardTrailingCenter:
			// Toward trailing, center
			::CGContextSetTextPosition(mInternals->mContextRef, point.mX, point.mY + imageBounds.size.height / 2.0);
			break;

		case kTextPositioningTowardTrailingBelow:
			// Toward trailing, below
			::CGContextSetTextPosition(mInternals->mContextRef, point.mX, point.mY + imageBounds.size.height);
			break;

		case kTextPositioningCenterAbove:
			// Center, above
			::CGContextSetTextPosition(mInternals->mContextRef, point.mX - imageBounds.size.width / 2.0, point.mY);
			break;

		case kTextPositioningCenter:
			// Center, center
			::CGContextSetTextPosition(mInternals->mContextRef, point.mX - imageBounds.size.width / 2.0,
					point.mY + imageBounds.size.height / 2.0);
			break;

		case kTextPositioningCenterBelow:
			// Center, below
			::CGContextSetTextPosition(mInternals->mContextRef, point.mX - imageBounds.size.width / 2.0,
					point.mY + imageBounds.size.height);
			break;

		case kTextPositioningTowardLeadingAbove:
			// Toward leading, above
			::CGContextSetTextPosition(mInternals->mContextRef, point.mX - imageBounds.size.width, point.mY);
			break;

		case kTextPositioningTowardLeadingCenter:
			// Toward leading, center
			::CGContextSetTextPosition(mInternals->mContextRef, point.mX - imageBounds.size.width,
					point.mY + imageBounds.size.height / 2.0);
			break;

		case kTextPositioningTowardLeadingBelow:
			// Toward leading, below
			::CGContextSetTextPosition(mInternals->mContextRef, point.mX - imageBounds.size.width,
					point.mY + imageBounds.size.height);
			break;
	}

	// Draw
	::CTLineDraw(lineRef, mInternals->mContextRef);
	::CFRelease(lineRef);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
void sInterpolateSRGBInterpolateColorInfo(SRGBInterpolateColorInfo *info, const CGFloat* in, CGFloat* output)
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform
	output[0] = info->mColor1Red + in[0] * (info->mColor2Red - info->mColor1Red);
	output[1] = info->mColor1Green + in[0] * (info->mColor2Green - info->mColor1Green);
	output[2] = info->mColor1Blue + in[0] * (info->mColor2Blue - info->mColor1Blue);
	output[3] = info->mColor1Alpha + in[0] * (info->mColor2Alpha - info->mColor1Alpha);
}
