//----------------------------------------------------------------------------------------------------------------------
//	CCoreGraphics.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CBitmap.h"

#include <CoreGraphics/CoreGraphics.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Types

typedef	T2DPoint<CGFloat>	S2DPointCGF;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CCoreGraphics

class CCoreGraphics {
	// Methods
	public:
							// Class methods
		static	CGImageRef	newImageRef(const CBitmap& bitmap);
};
