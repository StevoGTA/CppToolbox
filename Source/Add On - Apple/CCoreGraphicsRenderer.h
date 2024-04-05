//----------------------------------------------------------------------------------------------------------------------
//	CCoreGraphicsRenderer.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "C2DRenderer.h"
#include "CCoreGraphics.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreGraphicsRenderer

class CCoreGraphicsRenderer : public C2DRenderer {
	// Classes
	private:
		class Internals;

	// Methods
	public:
				// Lifecycle methods
				CCoreGraphicsRenderer(CBitmap& bitmap);
				CCoreGraphicsRenderer(CGContextRef contextRef);
				~CCoreGraphicsRenderer();

				// C2DRenderer methods
		void	setFillColor(const CColor& color);
		void	setStrokeColor(const CColor& color);

		void	strokeLine(const S2DPointF32& startPoint, const S2DPointF32& endPoint, bool antiAlias = true,
						Float32 lineWidth = 1.0) const;
		void	strokeLines(const S2DPointF32* points, UInt32 count, bool antiAlias = true, Float32 lineWidth = 1.0)
						const;

		void	strokeRect(const S2DRectF32& rect) const;
		void	fillRect(const S2DRectF32& rect) const;
		void	shadeRect(const S2DRectF32& rect, const S2DPointF32& shadeStartPoint, const S2DPointF32& shadeEndPoint,
						const CColor& startColor, const CColor& endColor) const;

		void	strokePath(const S2DPath32& path,
						const S2DAffineTransformF32& affineTransform = S2DAffineTransformF32()) const;

		void	fillText(const CString& string, const Font& font, const S2DPointF32& point,
						TextPositioning textPositioning = kTextPositioningCenter) const;

	// Properties
	private:
		Internals*	mInternals;
};
