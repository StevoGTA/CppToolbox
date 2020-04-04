//----------------------------------------------------------------------------------------------------------------------
//	CGPURenderObject2D.cpp			©2018 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CGPURenderObject2D.h"

#include "CGPUProgramBuiltins.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPURenderObject2DInternals

class CGPURenderObject2DInternals : public TReferenceCountable<CGPURenderObject2DInternals> {
	public:
		CGPURenderObject2DInternals(const CGPUTextureReference& gpuTextureReference) :
			TReferenceCountable(),
					mGPUTextureReference(gpuTextureReference), mAngleRadians(0.0), mAlpha(1.0), mScale(1.0, 1.0)
			{}

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
CGPURenderObject2D::CGPURenderObject2D(const CGPUTextureReference& gpuTextureReference) : CGPURenderObject()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CGPURenderObject2DInternals(gpuTextureReference);
}

//----------------------------------------------------------------------------------------------------------------------
CGPURenderObject2D::CGPURenderObject2D(const CGPURenderObject2D& other) : CGPURenderObject(other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CGPURenderObject2D::~CGPURenderObject2D()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: CGPURenderObject methods

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderObject2D::render(CGPU& gpu, const SGPURenderObjectRenderInfo& renderInfo) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	SGPUTextureInfo&	gpuTextureInfo = mInternals->mGPUTextureReference.getGPUTextureInfo();
			SGPUVertexBuffer	gpuVertexBuffer = gpu.allocateVertexBuffer(kGPUVertexBufferType2Vertex2Texture, 4);

			Float32				textureWidth = gpuTextureInfo.mGPUTextureSize.mWidth;
			Float32				textureHeight = gpuTextureInfo.mGPUTextureSize.mHeight;

			Float32				maxU = gpuTextureInfo.mMaxU;
			Float32				maxV = gpuTextureInfo.mMaxV;

	const	S2DOffset32&		offset = renderInfo.mOffset;

	// Points are UL, UR, LL, LR
	Float32*	bufferPtr = (Float32*) gpuVertexBuffer.mData.getMutableBytePtr();

	// Check angle
	Float32	dx, dy;
	if (mInternals->mAngleRadians == 0.0) {
		// No rotation
		dx = 0.0f - mInternals->mAnchorPoint.mX;
		dy = textureHeight - mInternals->mAnchorPoint.mY;
		bufferPtr[0] = dx * mInternals->mScale.mX + mInternals->mScreenPositionPoint.mX + offset.mDX;
		bufferPtr[1] = dy * mInternals->mScale.mY + mInternals->mScreenPositionPoint.mY + offset.mDY;
		bufferPtr[2] = 0.0;
		bufferPtr[3] = maxV;

		dx = textureWidth - mInternals->mAnchorPoint.mX;
		dy = textureHeight - mInternals->mAnchorPoint.mY;
		bufferPtr[4] = dx * mInternals->mScale.mX + mInternals->mScreenPositionPoint.mX + offset.mDX;
		bufferPtr[5] = dy * mInternals->mScale.mY + mInternals->mScreenPositionPoint.mY + offset.mDY;
		bufferPtr[6] = maxU;
		bufferPtr[7] = maxV;

		dx = 0.0f - mInternals->mAnchorPoint.mX;
		dy = 0.0f - mInternals->mAnchorPoint.mY;
		bufferPtr[8] = dx * mInternals->mScale.mX + mInternals->mScreenPositionPoint.mX + offset.mDX;
		bufferPtr[9] = dy * mInternals->mScale.mY + mInternals->mScreenPositionPoint.mY + offset.mDY;
		bufferPtr[10] = 0.0;
		bufferPtr[11] = 0.0;

		dx = textureWidth - mInternals->mAnchorPoint.mX;
		dy = 0.0f - mInternals->mAnchorPoint.mY;
		bufferPtr[12] = dx * mInternals->mScale.mX + mInternals->mScreenPositionPoint.mX + offset.mDX;
		bufferPtr[13] = dy * mInternals->mScale.mY + mInternals->mScreenPositionPoint.mY + offset.mDY;
		bufferPtr[14] = maxU;
		bufferPtr[15] = 0.0;
	} else {
		// Rotate around anchor point, then scale, then position on screen
		Float32	cosA = cosf(-mInternals->mAngleRadians);
		Float32	sinA = sinf(-mInternals->mAngleRadians);

		dx = 0.0f - mInternals->mAnchorPoint.mX;
		dy = textureHeight - mInternals->mAnchorPoint.mY;
		bufferPtr[0] =
				(cosA * dx - sinA * dy) * mInternals->mScale.mX + mInternals->mScreenPositionPoint.mX + offset.mDX;
		bufferPtr[1] =
				(sinA * dx + cosA * dy) * mInternals->mScale.mY + mInternals->mScreenPositionPoint.mY + offset.mDY;
		bufferPtr[2] = 0.0;
		bufferPtr[3] = maxV;

		dx = textureWidth - mInternals->mAnchorPoint.mX;
		dy = textureHeight - mInternals->mAnchorPoint.mY;
		bufferPtr[4] =
				(cosA * dx - sinA * dy) * mInternals->mScale.mX + mInternals->mScreenPositionPoint.mX + offset.mDX;
		bufferPtr[5] =
				(sinA * dx + cosA * dy) * mInternals->mScale.mY + mInternals->mScreenPositionPoint.mY + offset.mDY;
		bufferPtr[6] = maxU;
		bufferPtr[7] = maxV;

		dx = 0.0f - mInternals->mAnchorPoint.mX;
		dy = 0.0f - mInternals->mAnchorPoint.mY;
		bufferPtr[8] =
				(cosA * dx - sinA * dy) * mInternals->mScale.mX + mInternals->mScreenPositionPoint.mX + offset.mDX;
		bufferPtr[9] =
				(sinA * dx + cosA * dy) * mInternals->mScale.mY + mInternals->mScreenPositionPoint.mY + offset.mDY;
		bufferPtr[10] = 0.0;
		bufferPtr[11] = 0.0;

		dx = textureWidth - mInternals->mAnchorPoint.mX;
		dy = 0.0f - mInternals->mAnchorPoint.mY;
		bufferPtr[12] =
				(cosA * dx - sinA * dy) * mInternals->mScale.mX + mInternals->mScreenPositionPoint.mX + offset.mDX;
		bufferPtr[13] =
				(sinA * dx + cosA * dy) * mInternals->mScale.mY + mInternals->mScreenPositionPoint.mY + offset.mDY;
		bufferPtr[14] = maxU;
		bufferPtr[15] = 0.0;
	}

	// Draw
	if (renderInfo.mClipPlane.hasValue()) {
		// Clip plane
		CGPUClipOpacityProgram&	program = CGPUClipOpacityProgram::getProgram();
		program.willUse();
		program.setupVertexTextureInfo(gpuVertexBuffer, 2, gpuTextureInfo, mInternals->mAlpha);
		program.setClipPlane(*renderInfo.mClipPlane);
		gpu.renderTriangleStrip(program, SMatrix4x4_32(), 2);
	} else if (mInternals->mAlpha == 1.0) {
		// Opaque
		CGPUOpaqueProgram&	program = CGPUOpaqueProgram::getProgram();
		program.willUse();
		program.setupVertexTextureInfo(gpuVertexBuffer, 2, gpuTextureInfo);
		gpu.renderTriangleStrip(program, SMatrix4x4_32(), 2);
	} else {
		// Have alpha
		CGPUOpacityProgram&	program = CGPUOpacityProgram::getProgram();
		program.willUse();
		program.setupVertexTextureInfo(gpuVertexBuffer, 2, gpuTextureInfo, mInternals->mAlpha);
		gpu.renderTriangleStrip(program, SMatrix4x4_32(), 2);
	}

	// Cleanup
	gpu.disposeBuffer(gpuVertexBuffer);
}

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
void CGPURenderObject2D::render(CGPU& gpu, const S2DRect32& rect, const SGPURenderObjectRenderInfo& renderInfo) const
//----------------------------------------------------------------------------------------------------------------------
{
// Does not support angle, scale, nor anchor point
	// Setup
	const	SGPUTextureInfo&	gpuTextureInfo = mInternals->mGPUTextureReference.getGPUTextureInfo();
			SGPUVertexBuffer	gpuVertexBuffer = gpu.allocateVertexBuffer(kGPUVertexBufferType2Vertex2Texture, 4);

			Float32				textureWidth = gpuTextureInfo.mGPUTextureSize.mWidth;
			Float32				textureHeight = gpuTextureInfo.mGPUTextureSize.mHeight;

			Float32				maxU = gpuTextureInfo.mMaxU;
			Float32				maxV = gpuTextureInfo.mMaxV;

	const	S2DOffset32&		offset = renderInfo.mOffset;

	// Points are UL, UR, LL, LR
	Float32*	bufferPtr = (Float32*) gpuVertexBuffer.mData.getMutableBytePtr();

	// Check angle
//	Float32	dx, dy;
	if (mInternals->mAngleRadians == 0.0) {
		// No rotation
//		dx = 0.0f - mInternals->mAnchorPoint.mX;
//		dy = textureHeight - mInternals->mAnchorPoint.mY;
		bufferPtr[0] = mInternals->mScreenPositionPoint.mX + offset.mDX;
		bufferPtr[1] = mInternals->mScreenPositionPoint.mY + rect.mSize.mHeight + offset.mDY;
		bufferPtr[2] = rect.mOrigin.mX / textureWidth * maxU;
		bufferPtr[3] = (rect.mOrigin.mY + rect.mSize.mHeight) / textureHeight * maxV;

//		dx = textureWidth - mInternals->mAnchorPoint.mX;
//		dy = textureHeight - mInternals->mAnchorPoint.mY;
		bufferPtr[4] = mInternals->mScreenPositionPoint.mX + rect.mSize.mWidth + offset.mDX;
		bufferPtr[5] = mInternals->mScreenPositionPoint.mY + rect.mSize.mHeight + offset.mDY;
		bufferPtr[6] = (rect.mOrigin.mX + rect.mSize.mWidth) / textureWidth * maxU;
		bufferPtr[7] = (rect.mOrigin.mY + rect.mSize.mHeight) / textureHeight * maxV;

//		dx = 0.0f - mInternals->mAnchorPoint.mX;
//		dy = 0.0f - mInternals->mAnchorPoint.mY;
		bufferPtr[8] = mInternals->mScreenPositionPoint.mX + offset.mDX;
		bufferPtr[9] = mInternals->mScreenPositionPoint.mY + offset.mDY;
		bufferPtr[10] = rect.mOrigin.mX / textureWidth * maxU;
		bufferPtr[11] = rect.mOrigin.mY / textureHeight * maxV;

//		dx = textureWidth - mInternals->mAnchorPoint.mX;
//		dy = 0.0f - mInternals->mAnchorPoint.mY;
		bufferPtr[12] = mInternals->mScreenPositionPoint.mX + rect.mSize.mWidth + offset.mDX;
		bufferPtr[13] = mInternals->mScreenPositionPoint.mY + offset.mDY;
		bufferPtr[14] = (rect.mOrigin.mX + rect.mSize.mWidth) / textureWidth * maxU;
		bufferPtr[15] = rect.mOrigin.mY / textureHeight * maxV;
	} else {
		// Rotate around anchor point, then scale, then position on screen
//		Float32	cosA = cosf(-mInternals->mAngleRadians);
//		Float32	sinA = sinf(-mInternals->mAngleRadians);
//
//		dx = 0.0f - mInternals->mAnchorPoint.mX;
//		dy = textureHeight - mInternals->mAnchorPoint.mY;
//		bufferPtr[0] =
//				(cosA * dx - sinA * dy) * mInternals->mScale.mX + mInternals->mScreenPositionPoint.mX + offset.mX;
//		bufferPtr[1] =
//				(sinA * dx + cosA * dy) * mInternals->mScale.mY + mInternals->mScreenPositionPoint.mY + offset.mY;
//		bufferPtr[2] = 0.0;
//		bufferPtr[3] = maxV;
//
//		dx = textureWidth - mInternals->mAnchorPoint.mX;
//		dy = textureHeight - mInternals->mAnchorPoint.mY;
//		bufferPtr[4] =
//				(cosA * dx - sinA * dy) * mInternals->mScale.mX + mInternals->mScreenPositionPoint.mX + offset.mX;
//		bufferPtr[5] =
//				(sinA * dx + cosA * dy) * mInternals->mScale.mY + mInternals->mScreenPositionPoint.mY + offset.mY;
//		bufferPtr[6] = maxU;
//		bufferPtr[7] = maxV;
//
//		dx = 0.0f - mInternals->mAnchorPoint.mX;
//		dy = 0.0f - mInternals->mAnchorPoint.mY;
//		bufferPtr[8] =
//				(cosA * dx - sinA * dy) * mInternals->mScale.mX + mInternals->mScreenPositionPoint.mX + offset.mX;
//		bufferPtr[9] =
//				(sinA * dx + cosA * dy) * mInternals->mScale.mY + mInternals->mScreenPositionPoint.mY + offset.mY;
//		bufferPtr[10] = 0.0;
//		bufferPtr[11] = 0.0;
//
//		dx = textureWidth - mInternals->mAnchorPoint.mX;
//		dy = 0.0f - mInternals->mAnchorPoint.mY;
//		bufferPtr[12] =
//				(cosA * dx - sinA * dy) * mInternals->mScale.mX + mInternals->mScreenPositionPoint.mX + offset.mX;
//		bufferPtr[13] =
//				(sinA * dx + cosA * dy) * mInternals->mScale.mY + mInternals->mScreenPositionPoint.mY + offset.mY;
//		bufferPtr[14] = maxU;
//		bufferPtr[15] = 0.0;
	}

	// Draw
	if (renderInfo.mClipPlane.hasValue()) {
		// Clip plane
		CGPUClipOpacityProgram&	program = CGPUClipOpacityProgram::getProgram();
		program.willUse();
		program.setupVertexTextureInfo(gpuVertexBuffer, 2, gpuTextureInfo, mInternals->mAlpha);
		program.setClipPlane(*renderInfo.mClipPlane);
		gpu.renderTriangleStrip(program, SMatrix4x4_32(), 2);
	} else if (mInternals->mAlpha == 1.0) {
		// Opaque
		CGPUOpaqueProgram&	program = CGPUOpaqueProgram::getProgram();
		program.willUse();
		program.setupVertexTextureInfo(gpuVertexBuffer, 2, gpuTextureInfo);
		gpu.renderTriangleStrip(program, SMatrix4x4_32(), 2);
	} else {
		// Have alpha
		CGPUOpacityProgram&	program = CGPUOpacityProgram::getProgram();
		program.willUse();
		program.setupVertexTextureInfo(gpuVertexBuffer, 2, gpuTextureInfo, mInternals->mAlpha);
		gpu.renderTriangleStrip(program, SMatrix4x4_32(), 2);
	}

	// Cleanup
	gpu.disposeBuffer(gpuVertexBuffer);
}
