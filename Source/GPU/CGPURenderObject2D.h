//----------------------------------------------------------------------------------------------------------------------
//	CGPURenderObject2D.h			©2018 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPURenderObject.h"
#include "CGPUTextureManager.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPURenderObject2D

class CGPURenderObject2DInternals;
class CGPURenderObject2D : public CGPURenderObject {
	// Methods
	public:
								// Lifecycle methods
								CGPURenderObject2D(const CGPUTextureReference& gpuTextureReference);
								CGPURenderObject2D(const CGPURenderObject2D& other);
								~CGPURenderObject2D();

								// CGPURenderObject methods
		void					render(CGPU& gpu,
										const SGPURenderObjectRenderInfo& renderInfo = SGPURenderObjectRenderInfo())
										const;

								// Instance methods
		CGPUTextureReference&	getGPUTextureReference() const;

		S2DPoint32				getAnchorPoint() const;
		void					setAnchorPoint(const S2DPoint32& anchorPoint);

		S2DPoint32				getScreenPositionPoint() const;
		void					setScreenPositionPoint(const S2DPoint32& screenPositionPoint);

		Float32					getAngleAsRadians() const;
		void					setAngleAsRadians(Float32 angleRadians);
		Float32					getAngleAsDegrees() const
									{ return T2DUtilities<Float32>::toDegrees(getAngleAsRadians()); }
		void					setAngleAsDegrees(Float32 angleDegrees)
									{ setAngleAsRadians(T2DUtilities<Float32>::toRadians(angleDegrees)); }

		Float32					getAlpha() const;
		void					setAlpha(Float32 alpha);

		S2DPoint32				getScale() const;
		void					setScale(const S2DPoint32& scale);
		void					setScale(Float32 scale);

		void					render(CGPU& gpu, const S2DRect32& subrect,
										const SGPURenderObjectRenderInfo& renderInfo = SGPURenderObjectRenderInfo())
										const;

	// Properties
	private:
		CGPURenderObject2DInternals*	mInternals;
};
