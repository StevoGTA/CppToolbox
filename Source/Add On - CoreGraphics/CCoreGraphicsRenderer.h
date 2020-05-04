//----------------------------------------------------------------------------------------------------------------------
//	CCoreGraphicsRenderer.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CCoreGraphics.h"
#include "T2DRenderer.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreGraphicsRenderer

class CCoreGraphicsRendererInternals;
class CCoreGraphicsRenderer : public T2DRenderer<CGFloat> {
	// Methods
	public:
				// Lifecycle methods
				CCoreGraphicsRenderer(CBitmap& bitmap);
				~CCoreGraphicsRenderer();

				// T2DRenderer methods
		void	setFillColor(const CColor& color);
		void	setStrokeColor(const CColor& color);

		void	strokeLine(const S2DPointCGF& startPoint, const S2DPointCGF& endPoint, bool antiAlias = true,
						CGFloat lineWidth = 1.0);
		void	strokeLines(const S2DPointCGF* points, UInt32 count, bool antiAlias = true, CGFloat lineWidth = 1.0);

	// Properties
	private:
		CCoreGraphicsRendererInternals*	mInternals;
};
