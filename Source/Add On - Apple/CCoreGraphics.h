//----------------------------------------------------------------------------------------------------------------------
//	CCoreGraphics.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CBitmap.h"

#include <CoreGraphics/CoreGraphics.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreGraphics

class CCoreGraphics {
	// Types
	public:
		typedef	T2DPoint<CGFloat>	Point;
		typedef	T2DRect<CGFloat>	Rect;

	// Methods
	public:
							// Class methods
		static	Point		from(const CGPoint& point)
								{ return Point(point.x, point.y); }
		static	CGPoint		from(const Point& point)
								{ return CGPointMake(point.mX, point.mY); }

		static	Rect		from(const CGRect& rect)
								{ return Rect(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height); }
		static	CGRect		from(const Rect& rect)
								{ return CGRectMake(rect.mOrigin.mX, rect.mOrigin.mY, rect.mSize.mWidth,
										rect.mSize.mHeight); }

		static	CGImageRef	newImageRef(const CBitmap& bitmap);
};
