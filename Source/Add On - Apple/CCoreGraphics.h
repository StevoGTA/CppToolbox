//----------------------------------------------------------------------------------------------------------------------
//	CCoreGraphics.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "C2DPath.h"
#include "CBitmap.h"

#include <CoreGraphics/CoreGraphics.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreGraphics

class CCoreGraphics {
	// Methods
	public:
									// Class methods
		static	CGPoint				cgPointFor(const S2DPointF32& point)
										{ return CGPointMake(point.mX, point.mY); }
		static	S2DPointF32			pointFor(const CGPoint& point)
										{ return S2DPointF32(point.x, point.y); }

		static	CGRect				cgRectFor(const S2DRectF32& rect)
										{ return CGRectMake(rect.mOrigin.mX, rect.mOrigin.mY, rect.mSize.mWidth,
												rect.mSize.mHeight); }
		static	S2DRectF32			rectFor(const CGRect& rect)
										{ return S2DRectF32(rect.origin.x, rect.origin.y, rect.size.width,
												rect.size.height); }

		static	CGImageRef			newImageRef(const CBitmap& bitmap);

		static	CGPathRef			newPathRef(const S2DPath32& path,
											const S2DAffineTransformF32& affineTransform = S2DAffineTransformF32());

		static	CGAffineTransform	cgAffineTransformFor(const S2DAffineTransformF32& affineTransform)
										{ return CGAffineTransformMake(affineTransform.mA, affineTransform.mB,
												affineTransform.mC, affineTransform.mD, affineTransform.mTX,
												affineTransform.mTY); }
};
