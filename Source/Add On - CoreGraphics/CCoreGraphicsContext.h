//----------------------------------------------------------------------------------------------------------------------
//	CCoreGraphicsContext.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CCoreGraphics.h"

#include <CoreGraphics/CoreGraphics.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreGraphicsContext

class CCoreGraphicsContextInternals;
class CCoreGraphicsContext {
	// Methods
	public:
				// Lifecycle methods
				CCoreGraphicsContext(CBitmap& bitmap);
				~CCoreGraphicsContext();

				// Instance methods
		void	setFillColor(const CColor& color);
		void	setStrokeColor(const CColor& color);

		void	strokeLine(const S2DPointCGF& startPoint, const S2DPointCGF& endPoint, bool antiAlias = true,
						CGFloat lineWidth = 1.0);
		void	strokeLines(const S2DPointCGF* points, UInt32 count, bool antiAlias = true,
						CGFloat lineWidth = 1.0);

	// Properties
	private:
		CCoreGraphicsContextInternals*	mInternals;
};
