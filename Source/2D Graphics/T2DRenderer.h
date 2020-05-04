//----------------------------------------------------------------------------------------------------------------------
//	T2DRenderer.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CColor.h"
#include "C2DGeometry.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: E2DStrokeCapStyle

enum E2DStrokeCapStyle {
	k2DStrokeCapStyleRound,
	k2DStrokeCapStyleNone,
	k2DStrokeCapStyleSquare,
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - E2DStrokeJoinStyle

enum E2DStrokeJoinStyle {
	k2DStrokeJoinStyleRound,
	k2DStrokeJoinStyleBevel,
	k2DStrokeJoinStyleMiter,
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: T2DRenderer

template <typename T> class T2DRenderer {
	// Methods
	public:
						// Lifecycle methods
		virtual			~T2DRenderer() {}

						// Instance methods
		virtual	void	setFillColor(const CColor& color) = 0;
		virtual	void	setStrokeColor(const CColor& color) = 0;

		virtual	void	strokeLine(const T2DPoint<T>& startPoint, const T2DPoint<T>& endPoint, bool antiAlias = true,
								T lineWidth = 1.0) = 0;
		virtual	void	strokeLines(const T2DPoint<T>* points, UInt32 count, bool antiAlias = true,
								T lineWidth = 1.0) = 0;
};
