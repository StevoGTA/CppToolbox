//----------------------------------------------------------------------------------------------------------------------
//	CCoreGraphicsRenderer.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "C2DRenderer.h"
#include "CCoreGraphics.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreGraphicsRenderer

class CCoreGraphicsRendererInternals;
class CCoreGraphicsRenderer : public T2DRenderer<CGFloat> {
	// Methods
	public:
				// Lifecycle methods
				CCoreGraphicsRenderer(CBitmap& bitmap);
				~CCoreGraphicsRenderer();

				// C2DRenderer methods
		void	setFillColor(const CColor& color);
		void	setStrokeColor(const CColor& color);

				// T2DRenderer methods
		void	strokeLine(const CCoreGraphics::Point& startPoint, const CCoreGraphics::Point& endPoint,
						bool antiAlias = true, CGFloat lineWidth = 1.0);
		void	strokeLines(const CCoreGraphics::Point* points, UInt32 count, bool antiAlias = true,
						CGFloat lineWidth = 1.0);

	// Properties
	private:
		CCoreGraphicsRendererInternals*	mInternals;
};
