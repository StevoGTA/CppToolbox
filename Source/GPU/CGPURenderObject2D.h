//----------------------------------------------------------------------------------------------------------------------
//	CGPURenderObject2D.h			©2018 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPURenderObject.h"
#include "CGPUTextureManager.h"
#include "TIndexes.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPURenderObject2D

class CGPURenderObject2D : public CGPURenderObject {
	// Structs
	public:
		struct Item {
			// Methods
			Item(const S2DRectF32& screenRect, UInt8 textureIndex, const S2DRectF32& textureRect) :
				mScreenRect(screenRect), mTextureRect(textureRect), mTextureIndex(textureIndex)
				{}
			Item(const S2DRectF32& screenRect, const S2DRectF32& textureRect) :
				mScreenRect(screenRect), mTextureRect(textureRect), mTextureIndex(0)
				{}
			Item(const Item& other) :
				mScreenRect(other.mScreenRect), mTextureRect(other.mTextureRect), mTextureIndex(other.mTextureIndex)
				{}

			// Properties
			S2DRectF32	mScreenRect;
			S2DRectF32	mTextureRect;
			UInt8		mTextureIndex;
		};

	// Types
	public:
		typedef	TIndexes<UInt16>	Indexes;

	// Classes
	private:
		class Internals;

	// Methods
	public:
								// Lifecycle methods
								CGPURenderObject2D(CGPU& gpu, const Item& item,
										const CGPUTextureReference& gpuTextureReference,
										CGPUFragmentShader::Proc fragmentShaderProc =
												CGPUFragmentShader::getRGBAMultiTexture);
								CGPURenderObject2D(CGPU& gpu, const Item& item,
										const TArray<CGPUTextureReference>& gpuTextureReferences,
										CGPUFragmentShader::Proc fragmentShaderProc =
												CGPUFragmentShader::getRGBAMultiTexture);
								CGPURenderObject2D(CGPU& gpu, const TArray<Item>& items,
										const TArray<CGPUTextureReference>& gpuTextureReferences,
										CGPUFragmentShader::Proc fragmentShaderProc =
												CGPUFragmentShader::getRGBAMultiTexture);
								CGPURenderObject2D(const CGPURenderObject2D& other);
								~CGPURenderObject2D();

								// Instance methods
		const	S2DPointF32&	getAnchorPoint() const;
				void			setAnchorPoint(const S2DPointF32& anchorPoint);

		const	S2DPointF32&	getScreenPositionPoint() const;
				void			setScreenPositionPoint(const S2DPointF32& screenPositionPoint);

				Float32			getAngleAsRadians() const;
				void			setAngleAsRadians(Float32 angleRadians);
				Float32			getAngleAsDegrees() const
									{ return T2DUtilities<Float32>::toDegrees(getAngleAsRadians()); }
				void			setAngleAsDegrees(Float32 angleDegrees)
									{ setAngleAsRadians(T2DUtilities<Float32>::toRadians(angleDegrees)); }

				Float32			getAlpha() const;
				void			setAlpha(Float32 alpha);

		const	S2DPointF32&	getScale() const;
				void			setScale(const S2DPointF32& scale);
				void			setScale(Float32 scale);

				void			finishLoading() const;

				void			render(const Indexes& indexes,
										const RenderInfo& renderInfo =
												RenderInfo()) const;

								// Deprecated methods
		const	TArray<CGPUTextureReference>&	getGPUTextureReferences() const;

	// Properties
	private:
		Internals*	mInternals;
};
