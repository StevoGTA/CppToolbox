//----------------------------------------------------------------------------------------------------------------------
//	CGPURenderObject2D.cpp			©2018 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CGPURenderObject2D.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPURenderObject2DInternals

class CGPURenderObject2DInternals {
	public:
		CGPURenderObject2DInternals(const CGPUTextureReference& gpuTextureReference) :
			mGPUTextureReference(gpuTextureReference), mAngleRadians(0.0), mAlpha(1.0),
			mScale(1.0, 1.0)
			{}
		~CGPURenderObject2DInternals() {}

		CGPUTextureReference	mGPUTextureReference;

		S2DPoint32				mAnchorPoint;
		S2DPoint32				mScreenPositionPoint;
		Float32					mAngleRadians;
		Float32					mAlpha;
		S2DPoint32				mScale;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPURenderObject2D

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPURenderObject2D::CGPURenderObject2D(const CGPUTextureReference& gpuTextureReference)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CGPURenderObject2DInternals(gpuTextureReference);
}

//----------------------------------------------------------------------------------------------------------------------
CGPURenderObject2D::~CGPURenderObject2D()
//----------------------------------------------------------------------------------------------------------------------
{
	// Cleanup
	DisposeOf(mInternals);
}

// MARK: CGPURenderObject

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureReference& CGPURenderObject2D::getGPUTextureReference() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mGPUTextureReference;
}

//----------------------------------------------------------------------------------------------------------------------
S2DPoint32 CGPURenderObject2D::getAnchorPoint() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mAnchorPoint;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderObject2D::setAnchorPoint(const S2DPoint32& anchorPoint)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mAnchorPoint = anchorPoint;
}

//----------------------------------------------------------------------------------------------------------------------
S2DPoint32 CGPURenderObject2D::getScreenPositionPoint() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mScreenPositionPoint;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderObject2D::setScreenPositionPoint(const S2DPoint32& screenPositionPoint)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mScreenPositionPoint = screenPositionPoint;
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CGPURenderObject2D::getAngleAsRadians() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mAngleRadians;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderObject2D::setAngleAsRadians(Float32 angleRadians)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mAngleRadians = angleRadians;
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CGPURenderObject2D::getAlpha() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mAlpha;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderObject2D::setAlpha(Float32 alpha)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mAlpha = alpha;
}

//----------------------------------------------------------------------------------------------------------------------
S2DPoint32 CGPURenderObject2D::getScale() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mScale;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderObject2D::setScale(const S2DPoint32& scale)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mScale = scale;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderObject2D::setScale(Float32 scale)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mScale = S2DPoint32(scale, scale);
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderObject2D::render(CGPU& gpu, const S2DPoint32& offset)
//----------------------------------------------------------------------------------------------------------------------
{
	// Ensure we are fully loaded
	mInternals->mGPUTextureReference.finishLoading();

	// Setup
			Float32				a = -mInternals->mAngleRadians;
			Float32				cosA = cosf(a);
			Float32				sinA = sinf(a);

	const	SGPUTextureInfo&	gpuTextureInfo = mInternals->mGPUTextureReference.getGPUTextureInfo();
			SGPUVertexBuffer	gpuVertexBuffer = gpu.allocateVertexBuffer(kGPUVertexBufferType2Vertex2Texture, 4);

			Float32				textureWidth = gpuTextureInfo.mGPUTextureSize.mWidth;
			Float32				textureHeight = gpuTextureInfo.mGPUTextureSize.mHeight;

			Float32				maxU = gpuTextureInfo.mMaxU;
			Float32				maxV = gpuTextureInfo.mMaxV;

	// Points are UL, UR, LL, LR
	Float32*	bufferPtr = (Float32*) gpuVertexBuffer.mData.getMutableBytePtr();

	// Rotate around anchor point, then scale, then position on screen
	Float32	dx, dy;
	dx = 0.0f - mInternals->mAnchorPoint.mX;
	dy = textureHeight - mInternals->mAnchorPoint.mY;
	bufferPtr[0] = (cosA * dx - sinA * dy) * mInternals->mScale.mX + mInternals->mScreenPositionPoint.mX + offset.mX;
	bufferPtr[1] = (sinA * dx + cosA * dy) * mInternals->mScale.mY + mInternals->mScreenPositionPoint.mY + offset.mY;
	bufferPtr[2] = 0.0;
	bufferPtr[3] = maxV;

	dx = textureWidth - mInternals->mAnchorPoint.mX;
	dy = textureHeight - mInternals->mAnchorPoint.mY;
	bufferPtr[4] = (cosA * dx - sinA * dy) * mInternals->mScale.mX + mInternals->mScreenPositionPoint.mX + offset.mX;
	bufferPtr[5] = (sinA * dx + cosA * dy) * mInternals->mScale.mY + mInternals->mScreenPositionPoint.mY + offset.mY;
	bufferPtr[6] = maxU;
	bufferPtr[7] = maxV;

	dx = 0.0f - mInternals->mAnchorPoint.mX;
	dy = 0.0f - mInternals->mAnchorPoint.mY;
	bufferPtr[8] = (cosA * dx - sinA * dy) * mInternals->mScale.mX + mInternals->mScreenPositionPoint.mX + offset.mX;
	bufferPtr[9] = (sinA * dx + cosA * dy) * mInternals->mScale.mY + mInternals->mScreenPositionPoint.mY + offset.mY;
	bufferPtr[10] = 0.0;
	bufferPtr[11] = 0.0;

	dx = textureWidth - mInternals->mAnchorPoint.mX;
	dy = 0.0f - mInternals->mAnchorPoint.mY;
	bufferPtr[12] = (cosA * dx - sinA * dy) * mInternals->mScale.mX + mInternals->mScreenPositionPoint.mX + offset.mX;
	bufferPtr[13] = (sinA * dx + cosA * dy) * mInternals->mScale.mY + mInternals->mScreenPositionPoint.mY + offset.mY;
	bufferPtr[14] = maxU;
	bufferPtr[15] = 0.0;

	// Draw
	gpu.renderTriangleStrip(gpuVertexBuffer, 4, gpuTextureInfo, OV<Float32>(mInternals->mAlpha));

	// Cleanup
	gpu.disposeBuffer(gpuVertexBuffer);
}
