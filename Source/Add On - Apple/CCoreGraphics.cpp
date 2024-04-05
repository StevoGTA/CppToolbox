//----------------------------------------------------------------------------------------------------------------------
//	CCoreGraphics.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CCoreGraphics.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local Data

struct S2DPathIterateInfo {
	CGMutablePathRef	mPathRef;
	CGAffineTransform	mAffineTransform;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

static	void	sPathSegmentMoveTo32(const S2DPointF32& point, S2DPathIterateInfo* pathIterateInfo);
static	void	sPathSegmentLineTo32(const S2DPointF32& point, S2DPathIterateInfo* pathIterateInfo);
static	void	sPathSegmentQuadCurveTo32(const S2DPointF32& controlPoint, const S2DPointF32& point,
						S2DPathIterateInfo* pathIterateInfo);
static	void	sPathSegmentCubicCurveTo32(const S2DPointF32& controlPoint1, const S2DPointF32& controlPoint2,
						const S2DPointF32& point, S2DPathIterateInfo* pathIterateInfo);
static	void	sPathSegmentArcTo32(Float32 radiusX, Float32 radiusY, Float32 rotationAngleRadians, bool useLargerArc,
						bool isClockwise, const S2DPointF32& point, S2DPathIterateInfo* pathIterateInfo);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CCoreGraphics

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

//----------------------------------------------------------------------------------------------------------------------
CGPathRef CCoreGraphics::newPathRef(const S2DPath32& path, const S2DAffineTransformF32& affineTransform)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	S2DPathIterateInfo	pathIterateInfo;
	pathIterateInfo.mPathRef = ::CGPathCreateMutable();
	pathIterateInfo.mAffineTransform = cgAffineTransformFor(affineTransform);

	// Iterate segments
	((S2DPath32*) &path)->iterateSegments((S2DPath32::MoveToProc) sPathSegmentMoveTo32,
			(S2DPath32::LineToProc) sPathSegmentLineTo32, (S2DPath32::QuadCurveToProc) sPathSegmentQuadCurveTo32,
			(S2DPath32::CubicCurveToProc) sPathSegmentCubicCurveTo32, (S2DPath32::ArcToProc) sPathSegmentArcTo32,
			&pathIterateInfo);

	return pathIterateInfo.mPathRef;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
void sPathSegmentMoveTo32(const S2DPointF32& point, S2DPathIterateInfo* pathIterateInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update path
	::CGPathMoveToPoint(pathIterateInfo->mPathRef, &pathIterateInfo->mAffineTransform, point.mX, point.mY);
}

//----------------------------------------------------------------------------------------------------------------------
void sPathSegmentLineTo32(const S2DPointF32& point, S2DPathIterateInfo* pathIterateInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update path
	::CGPathAddLineToPoint(pathIterateInfo->mPathRef, &pathIterateInfo->mAffineTransform, point.mX, point.mY);
}

//----------------------------------------------------------------------------------------------------------------------
void sPathSegmentQuadCurveTo32(const S2DPointF32& controlPoint, const S2DPointF32& point,
		S2DPathIterateInfo* pathIterateInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update path
	::CGPathAddQuadCurveToPoint(pathIterateInfo->mPathRef, &pathIterateInfo->mAffineTransform, controlPoint.mX,
			controlPoint.mY, point.mX, point.mY);
}

//----------------------------------------------------------------------------------------------------------------------
void sPathSegmentCubicCurveTo32(const S2DPointF32& controlPoint1, const S2DPointF32& controlPoint2,
		const S2DPointF32& point, S2DPathIterateInfo* pathIterateInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update path
	::CGPathAddCurveToPoint(pathIterateInfo->mPathRef, &pathIterateInfo->mAffineTransform, controlPoint1.mX,
			controlPoint1.mY, controlPoint2.mX, controlPoint2.mY, point.mX, point.mY);
}

//----------------------------------------------------------------------------------------------------------------------
void sPathSegmentArcTo32(Float32 radiusX, Float32 radiusY, Float32 rotationAngleRadians, bool useLargerArc,
		bool isClockwise, const S2DPointF32& point, S2DPathIterateInfo* pathIterateInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Algorithm from http://www.w3.org/TR/SVG/implnote.html#ArcConversionEndpointToCenter
	CGPoint	currentPoint = ::CGPathGetCurrentPoint(pathIterateInfo->mPathRef);
	CGFloat	x1 = currentPoint.x;
	CGFloat	y1 = currentPoint.y;
	CGFloat	x2 = point.mX;
	CGFloat	y2 = point.mY;

	// Step 1: Compute (x1', y1')
	TMatrix2x2<CGFloat>	S1a = TMatrix2x2<CGFloat>(
													cos(rotationAngleRadians),
													-sin(rotationAngleRadians),
													sin(rotationAngleRadians),
													cos(rotationAngleRadians)
												 );
	TMatrix2x1<CGFloat>	S1b = TMatrix2x1<CGFloat>((x1 - x2) / 2.0, (y1 - y2) / 2.0);
	TMatrix2x1<CGFloat>	pp = S1a * S1b;
	CGFloat	x1p = pp.m1_1;
	CGFloat	y1p = pp.m2_1;

	// Update rx, ry if needed
	CGFloat	A = (x1p * x1p) / (radiusX * radiusX) + (y1p * y1p) / (radiusY * radiusY);
	if (A > 1.0) {
		radiusX = sqrt(A) * radiusX;
		radiusY = sqrt(A) * radiusY;
	}

	// Step 2: Compute (cx', cy')
	CGFloat	numerator =
					radiusX * radiusX * radiusY * radiusY - radiusX * radiusX * y1p * y1p -
							radiusY * radiusY * x1p * x1p;
	CGFloat	denominator = radiusX * radiusX * y1p * y1p + radiusY * radiusY * x1p * x1p;
	CGFloat	value = fabs(numerator / denominator);
	CGFloat	sqrtValue = (useLargerArc != isClockwise) ? sqrt(value) : -sqrt(value);
	CGFloat	cxp = sqrtValue * radiusX * y1p / radiusY;
	CGFloat	cyp = sqrtValue * -radiusY * x1p / radiusX;

	// Step 3: Compute (cx, cy) from (cx', cy')
	TMatrix2x2<CGFloat>	S3a = TMatrix2x2<CGFloat>(
													cos(rotationAngleRadians),
													sin(rotationAngleRadians),
													-sin(rotationAngleRadians),
													cos(rotationAngleRadians)
												 );
	TMatrix2x1<CGFloat>	S3b = TMatrix2x1<CGFloat>(cxp, cyp);
	TMatrix2x1<CGFloat>	S3c = TMatrix2x1<CGFloat>((x1 + x2) / 2.0, (y1 + y2) / 2.0);
	TMatrix2x1<CGFloat>	c = S3a * S3b + S3c;
	CGFloat				cx = c.m1_1;
	CGFloat				cy = c.m2_1;

	// Step 4: Compute theta1 and deltaTheta
	T2DVector<CGFloat>	u1 = T2DVector<CGFloat>(1.0, 0.0);
	T2DVector<CGFloat>	v1 = T2DVector<CGFloat>((x1p - cxp) / radiusX, (y1p - cyp) / radiusY);
	CGFloat				theta1 = u1.angle(v1);

	T2DVector<CGFloat>	u2 = T2DVector<CGFloat>((x1p - cxp) / radiusX, (y1p - cyp) / radiusY);
	T2DVector<CGFloat>	v2 = T2DVector<CGFloat>((-x1p - cxp) / radiusX, (-y1p - cyp) / radiusY);
	CGFloat				deltaTheta = u2.angle(v2);
	if (!isClockwise && (deltaTheta > 0.0))
		deltaTheta -= 2.0f * M_PI;
	else if (isClockwise && (deltaTheta < 0.0))
		deltaTheta += 2.0f * M_PI;

	// Add path segment
	CGAffineTransform	affineTransform = CGAffineTransformIdentity;
	affineTransform = CGAffineTransformTranslate(affineTransform, cx, cy);
	affineTransform = CGAffineTransformRotate(affineTransform, rotationAngleRadians);
	affineTransform = CGAffineTransformScale(affineTransform, radiusX, radiusY);

	::CGPathAddArc(pathIterateInfo->mPathRef, &affineTransform, 0.0, 0.0, 1.0, theta1, theta1 + deltaTheta,
			!isClockwise);
}
