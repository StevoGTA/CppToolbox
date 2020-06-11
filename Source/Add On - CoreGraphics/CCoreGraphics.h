//----------------------------------------------------------------------------------------------------------------------
//	CCoreGraphics.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CBitmap.h"

#include <CoreGraphics/CoreGraphics.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Types

typedef	T2DPoint<CGFloat>	S2DPointCGF;
typedef	T2DRect<CGFloat>	S2DRectCGF;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CCoreGraphics

class CCoreGraphics {
	// Methods
	public:
							// Class methods
		static	S2DPointCGF	from(const CGPoint& point)
								{ return S2DPointCGF(point.x, point.y); }
		static	CGPoint		from(const S2DPointCGF& point)
								{ return CGPointMake(point.mX, point.mY); }

		static	S2DRectCGF	from(const CGRect& rect)
								{ return S2DRectCGF(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height); }
		static	CGRect		from(const S2DRectCGF& rect)
								{ return CGRectMake(rect.mOrigin.mX, rect.mOrigin.mY, rect.mSize.mWidth,
										rect.mSize.mHeight); }

		static	CGImageRef	newImageRef(const CBitmap& bitmap);
};
