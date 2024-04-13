//----------------------------------------------------------------------------------------------------------------------
//	CCoreGraphicsRenderer.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CCoreGraphicsRenderer.h"

#include <CoreText/CoreText.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

struct SRGBInterpolateColorInfo {
	public:
						SRGBInterpolateColorInfo(const CColor& color1, const CColor& color2) :
							mInitialRGBValues(color1.getRGBValues()),
									mDeltaRGBValues(color2.getRGBValues() - color1.getRGBValues())
							{}

		static	void	interpolate(SRGBInterpolateColorInfo *info, const CGFloat* mix, CGFloat* result)
							{
								result[0] =
										info->mInitialRGBValues.getRed() + info->mDeltaRGBValues.getRed() * mix[0];
								result[1] =
										info->mInitialRGBValues.getGreen() + info->mDeltaRGBValues.getGreen() * mix[0];
								result[2] =
										info->mInitialRGBValues.getBlue() + info->mDeltaRGBValues.getBlue() * mix[0];
								result[3] =
										info->mInitialRGBValues.getAlpha() + info->mDeltaRGBValues.getAlpha() * mix[0];
							}
		static	void	cleanup(SRGBInterpolateColorInfo* info)
							{ Delete(info); }

	private:
		CColor::RGBValues	mInitialRGBValues;
		CColor::RGBValues	mDeltaRGBValues;
};

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
		OV<CColor>		mFillColor;
		OV<CColor>		mStrokeColor;
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
	// Store
	mInternals->mFillColor.setValue(color);

	// Update CGContextRef
	CColor::RGBValues	rgbValues = color.getRGBValues();
	::CGContextSetRGBFillColor(mInternals->mContextRef, rgbValues.getRed(), rgbValues.getGreen(), rgbValues.getBlue(),
			rgbValues.getAlpha());
}

//----------------------------------------------------------------------------------------------------------------------
void CCoreGraphicsRenderer::setStrokeColor(const CColor& color)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mStrokeColor.setValue(color);

	// Update CGContextRef
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
	CGFloat						domain[2] = {0.0, 1.0};
	CGFloat						range[8] = {0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0};
	CGFunctionCallbacks			callbacks =
										{0, (CGFunctionEvaluateCallback) SRGBInterpolateColorInfo::interpolate,
												(CGFunctionReleaseInfoCallback) SRGBInterpolateColorInfo::cleanup};

	SRGBInterpolateColorInfo*	rgbInterpolateColorInfo = new SRGBInterpolateColorInfo(startColor, endColor);

	CGColorSpaceRef				colorSpaceRef = ::CGColorSpaceCreateDeviceRGB();
	CGFunctionRef				functionRef =
										::CGFunctionCreate(rgbInterpolateColorInfo, 1, domain, 4, range, &callbacks);
	CGShadingRef				shadingRef =
										::CGShadingCreateAxial(colorSpaceRef,
												CCoreGraphics::cgPointFor(shadeStartPoint),
												CCoreGraphics::cgPointFor(shadeEndPoint), functionRef, false, false);
	::CFRelease(colorSpaceRef);
	::CFRelease(functionRef);

	// Do it
	::CGContextDrawShading(mInternals->mContextRef, shadingRef);
	::CFRelease(shadingRef);

	// Unclip
	::CGContextRestoreGState(mInternals->mContextRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CCoreGraphicsRenderer::strokeRect(const S2DRectF32& rect) const
//----------------------------------------------------------------------------------------------------------------------
{
	::CGContextStrokeRect(mInternals->mContextRef, CCoreGraphics::cgRectFor(rect));
}

//----------------------------------------------------------------------------------------------------------------------
void CCoreGraphicsRenderer::strokePath(const S2DPath32& path, Float32 lineWidth,
		const S2DAffineTransformF32& affineTransform) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CGPathRef	pathRef = CCoreGraphics::newPathRef(path, affineTransform);

	// Stroke
	::CGContextSetLineWidth(mInternals->mContextRef, lineWidth);
	::CGContextAddPath(mInternals->mContextRef, pathRef);
	::CGContextStrokePath(mInternals->mContextRef);

	// Cleanup
	::CGPathRelease(pathRef);
}

//----------------------------------------------------------------------------------------------------------------------
S2DSizeF32 CCoreGraphicsRenderer::getTextSize(const CString& string, const Font& font) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CTFontRef		fontRef = ::CTFontCreateWithName(font.getName().getOSString(), font.getSize(), nil);
	CFStringRef		keys[] = { kCTFontAttributeName };
	CFTypeRef		values[] = { fontRef };
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

	return S2DSizeF32(imageBounds.size.width, imageBounds.size.height);
}

//----------------------------------------------------------------------------------------------------------------------
void CCoreGraphicsRenderer::strokeText(const CString& string, const Font& font, const S2DPointF32& point,
		TextPositioning textPositioning) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CTFontRef			fontRef = ::CTFontCreateWithName(font.getName().getOSString(), font.getSize(), nil);

	CColor::RGBValues	rgbValues = mInternals->mStrokeColor.getValue(CColor::mBlack).getRGBValues();
	CGColorRef			colorRef =
								::CGColorCreateGenericRGB(rgbValues.getRed(), rgbValues.getGreen(), rgbValues.getBlue(),
										rgbValues.getAlpha());

	CFStringRef			keys[] =
							{
								kCTFontAttributeName,
								kCTForegroundColorAttributeName,
							};
	CFTypeRef			values[] =
							{
								fontRef,
								colorRef,
							};
	CFDictionaryRef	attributesDictionaryRef =
							::CFDictionaryCreate(kCFAllocatorDefault, (const void**) &keys,
									(const void**) &values, sizeof(keys) / sizeof(keys[0]),
									&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	::CFRelease(fontRef);
	::CGColorRelease(colorRef);

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
