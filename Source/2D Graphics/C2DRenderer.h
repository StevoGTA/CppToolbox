//----------------------------------------------------------------------------------------------------------------------
//	C2DRenderer.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CColor.h"
#include "C2DGeometry.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: C2DRenderer

class C2DRenderer {
	// Types
	public:
		enum StrokeCapStyle {
			kStrokeCapStyleRound,
			kStrokeCapStyleNone,
			kStrokeCapStyleSquare,
		};

		enum StrokeJoinStyle {
			kStrokeJoinStyleRound,
			kStrokeJoinStyleBevel,
			kStrokeJoinStyleMiter,
		};

	// Methods
	public:
						// Lifecycle methods
						C2DRenderer() {}
		virtual			~C2DRenderer() {}

						// Instance methods
		virtual	void	setFillColor(const CColor& color) = 0;
		virtual	void	setStrokeColor(const CColor& color) = 0;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - T2DRenderer

template <typename T> class T2DRenderer : public C2DRenderer {
	// Methods
	public:
						// Instance methods
		virtual	void	strokeLine(const T2DPoint<T>& startPoint, const T2DPoint<T>& endPoint, bool antiAlias = true,
								T lineWidth = 1.0) = 0;
		virtual	void	strokeLines(const T2DPoint<T>* points, UInt32 count, bool antiAlias = true,
								T lineWidth = 1.0) = 0;
};
