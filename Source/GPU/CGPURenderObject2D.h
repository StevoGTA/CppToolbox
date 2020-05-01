//----------------------------------------------------------------------------------------------------------------------
//	CGPURenderObject2D.h			©2018 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPURenderObject.h"
#include "CGPUTextureManager.h"
#include "TIndexes.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SGPURenderObject2DItem

struct SGPURenderObject2DItem {
	// Methods
	SGPURenderObject2DItem(const S2DRectF32& screenRect, UInt8 textureIndex, const S2DRectF32& textureRect) :
		mScreenRect(screenRect), mTextureIndex(textureIndex), mTextureRect(textureRect)
		{}
	SGPURenderObject2DItem(const SGPURenderObject2DItem& other) :
		mScreenRect(other.mScreenRect), mTextureIndex(other.mTextureIndex), mTextureRect(other.mTextureRect)
		{}

	// Properties
	S2DRectF32	mScreenRect;
	UInt8		mTextureIndex;
	S2DRectF32	mTextureRect;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPURenderObject2D

typedef	TIndexes<UInt16>	CGPURenderObject2DIndexes;

class CGPURenderObject2DInternals;
class CGPURenderObject2D : public CGPURenderObject {
	// Methods
	public:
							// Lifecycle methods
							CGPURenderObject2D(CGPU& gpu, const S2DRectF32& screenRect, const S2DRectF32& textureRect,
									const CGPUTextureReference& gpuTextureReference);
							CGPURenderObject2D(CGPU& gpu, const TArray<SGPURenderObject2DItem>& items,
									const TArray<CGPUTextureReference>& gpuTextureReferences);
							CGPURenderObject2D(const CGPURenderObject2D& other);
							~CGPURenderObject2D();

							// Instance methods
				S2DPointF32	getAnchorPoint() const;
				void		setAnchorPoint(const S2DPointF32& anchorPoint);

				S2DPointF32	getScreenPositionPoint() const;
				void		setScreenPositionPoint(const S2DPointF32& screenPositionPoint);

				Float32		getAngleAsRadians() const;
				void		setAngleAsRadians(Float32 angleRadians);
				Float32		getAngleAsDegrees() const
								{ return T2DUtilities<Float32>::toDegrees(getAngleAsRadians()); }
				void		setAngleAsDegrees(Float32 angleDegrees)
								{ setAngleAsRadians(T2DUtilities<Float32>::toRadians(angleDegrees)); }

				Float32		getAlpha() const;
				void		setAlpha(Float32 alpha);

				S2DPointF32	getScale() const;
				void		setScale(const S2DPointF32& scale);
				void		setScale(Float32 scale);

				void		finishLoading() const;

				void		render(CGPU& gpu, const CGPURenderObject2DIndexes& indexes,
									const SGPURenderObjectRenderInfo& renderInfo = SGPURenderObjectRenderInfo()) const;

							// Deprecated methods
		const	TArray<CGPUTextureReference>&	getGPUTextureReferences() const;

	// Properties
	private:
		CGPURenderObject2DInternals*	mInternals;
};
