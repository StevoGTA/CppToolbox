//----------------------------------------------------------------------------------------------------------------------
//	C2DRenderer.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CColor.h"
#include "C2DPath.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: C2DRenderer

class C2DRenderer {
	// Font
	public:
		struct Font {
			// Methods
									// Lifecycle methods
									Font(const CString& name, Float32 size) : mName(name), mSize(size) {}
									Font(const Font& other) : mName(other.mName), mSize(other.mSize) {}

									// Instance methods
				const	CString&	getName() const
										{ return mName; }
						Float32		getSize() const
										{ return mSize; }

			// Properties
			private:
				CString	mName;
				Float32	mSize;
		};

	// StrokeCapStyle
	public:
		enum StrokeCapStyle {
			kStrokeCapStyleRound,
			kStrokeCapStyleNone,
			kStrokeCapStyleSquare,
		};

	// StrokeJoinStyle
	public:
		enum StrokeJoinStyle {
			kStrokeJoinStyleRound,
			kStrokeJoinStyleBevel,
			kStrokeJoinStyleMiter,
		};

	// TextPositioning
	public:
		enum TextPositioning {
			kTextPositioningTowardTrailingAbove,
			kTextPositioningTowardTrailingCenter,
			kTextPositioningTowardTrailingBelow,
			kTextPositioningCenterAbove,
			kTextPositioningCenter,
			kTextPositioningCenterBelow,
			kTextPositioningTowardLeadingAbove,
			kTextPositioningTowardLeadingCenter,
			kTextPositioningTowardLeadingBelow,
		};

	// Methods
	public:
						// Lifecycle methods
		virtual			~C2DRenderer() {}

						// Instance methods
		virtual	void	setFillColor(const CColor& color) = 0;
		virtual	void	setStrokeColor(const CColor& color) = 0;

		virtual	void	strokeLine(const S2DPointF32& startPoint, const S2DPointF32& endPoint, bool antiAlias = true,
								Float32 lineWidth = 1.0) const = 0;
		virtual	void	strokeLines(const S2DPointF32* points, UInt32 count, bool antiAlias = true,
								Float32 lineWidth = 1.0) const = 0;

		virtual	void	strokeRect(const S2DRectF32& rect) const = 0;
		virtual	void	fillRect(const S2DRectF32& rect) const = 0;
		virtual	void	shadeRect(const S2DRectF32& rect, const S2DPointF32& shadeStartPoint,
								const S2DPointF32& shadeEndPoint, const CColor& startColor, const CColor& endColor)
								const = 0;

		virtual	void	strokePath(const S2DPath32& path,
								const S2DAffineTransformF32& affineTransform = S2DAffineTransformF32()) const = 0;

		virtual	void	fillText(const CString& string, const Font& font, const S2DPointF32& point,
								TextPositioning textPositioning = kTextPositioningCenter) const = 0;

	protected:
						// Lifecycle methods
						C2DRenderer() {}
};
