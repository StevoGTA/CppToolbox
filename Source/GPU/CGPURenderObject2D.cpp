//----------------------------------------------------------------------------------------------------------------------
//	CGPURenderObject2D.cpp			©2018 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CGPURenderObject2D.h"

#include "CGPURenderState.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPURenderObject2DInternals

class CGPURenderObject2DInternals : public TReferenceCountable<CGPURenderObject2DInternals> {
	public:
				CGPURenderObject2DInternals(CGPU& gpu, const TArray<SGPURenderObject2DItem>& items,
						const TArray<CGPUTextureReference>& gpuTextureReferences) :
					TReferenceCountable(),
							mGPU(gpu), mGPUTextureReferences(gpuTextureReferences),
							mGPUVertexBuffer(
									mGPU.allocateVertexBuffer(sizeof(SVertex2DMultitexture), vertexData(items))),
							mAngleRadians(0.0), mAlpha(1.0), mScale(1.0, 1.0)
					{}
				~CGPURenderObject2DInternals()
					{
						// Cleanup
						mGPU.disposeBuffer(mGPUVertexBuffer);
					}

		CData	vertexData(const TArray<SGPURenderObject2DItem>& items)
					{
						// Setup buffer
						CData					vertexData(
														(CDataSize) sizeof(SVertex2DMultitexture) * items.getCount() *
																6);
						SVertex2DMultitexture*	vertexInfoPtr = (SVertex2DMultitexture*) vertexData.getMutableBytePtr();
						for (TIteratorD<SGPURenderObject2DItem> iterator = items.getIterator(); iterator.hasValue();
								iterator.advance()) {
							// Setup
							const	SGPURenderObject2DItem&	item = iterator.getValue();
							const	S2DRectF32&				screenRect = item.mScreenRect;
							const	S2DRectF32&				textureRect = item.mTextureRect;

									Float32					minX = screenRect.mOrigin.mX;
									Float32					maxX = screenRect.mOrigin.mX + screenRect.getWidth();
									Float32					minY = screenRect.mOrigin.mY;
									Float32					maxY = screenRect.mOrigin.mY + screenRect.getHeight();

									Float32					minS = textureRect.getMinX();
									Float32					maxS = textureRect.getMaxX();
									Float32					minT = textureRect.getMinY();
									Float32					maxT = textureRect.getMaxY();

							// Store in buffer
							*(vertexInfoPtr++) =
									SVertex2DMultitexture(S2DPointF32(minX, maxY), S2DPointF32(minS, maxT),
											item.mTextureIndex);
							*(vertexInfoPtr++) =
									SVertex2DMultitexture(S2DPointF32(minX, maxY), S2DPointF32(minS, maxT),
											item.mTextureIndex);

							*(vertexInfoPtr++) =
									SVertex2DMultitexture(S2DPointF32(maxX, maxY), S2DPointF32(maxS, maxT),
											item.mTextureIndex);

							*(vertexInfoPtr++) =
									SVertex2DMultitexture(S2DPointF32(minX, minY), S2DPointF32(minS, minT),
											item.mTextureIndex);

							*(vertexInfoPtr++) =
									SVertex2DMultitexture(S2DPointF32(maxX, minY), S2DPointF32(maxS, minT),
											item.mTextureIndex);
							*(vertexInfoPtr++) =
									SVertex2DMultitexture(S2DPointF32(maxX, minY), S2DPointF32(maxS, minT),
											item.mTextureIndex);
						}

						return vertexData;
					}

		CGPU&							mGPU;
		TArray<CGPUTextureReference>	mGPUTextureReferences;
		SGPUVertexBuffer				mGPUVertexBuffer;

		S2DPointF32						mAnchorPoint;
		S2DPointF32						mScreenPositionPoint;
		Float32							mAngleRadians;
		Float32							mAlpha;
		S2DPointF32						mScale;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPURenderObject2D

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPURenderObject2D::CGPURenderObject2D(CGPU& gpu, const S2DRectF32& screenRect, const S2DRectF32& textureRect,
		const CGPUTextureReference& gpuTextureReference) : CGPURenderObject()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals =
			new CGPURenderObject2DInternals(gpu,
					TNArray<SGPURenderObject2DItem>(SGPURenderObject2DItem(screenRect, 0, textureRect)),
					TNArray<CGPUTextureReference>(gpuTextureReference));
}

//----------------------------------------------------------------------------------------------------------------------
CGPURenderObject2D::CGPURenderObject2D(CGPU& gpu, const TArray<SGPURenderObject2DItem>& items,
		const TArray<CGPUTextureReference>& gpuTextureReferences) : CGPURenderObject()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CGPURenderObject2DInternals(gpu, items, gpuTextureReferences);
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

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
S2DPointF32 CGPURenderObject2D::getAnchorPoint() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mAnchorPoint;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderObject2D::setAnchorPoint(const S2DPointF32& anchorPoint)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mAnchorPoint = anchorPoint;
}

//----------------------------------------------------------------------------------------------------------------------
S2DPointF32 CGPURenderObject2D::getScreenPositionPoint() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mScreenPositionPoint;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderObject2D::setScreenPositionPoint(const S2DPointF32& screenPositionPoint)
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
S2DPointF32 CGPURenderObject2D::getScale() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mScale;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderObject2D::setScale(const S2DPointF32& scale)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mScale = scale;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderObject2D::setScale(Float32 scale)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mScale = S2DPointF32(scale, scale);
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderObject2D::finishLoading() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Finish loading all textures
	for (TIteratorD<CGPUTextureReference> iterator = mInternals->mGPUTextureReferences.getIterator();
			iterator.hasValue(); iterator.advance())
		// Finish loading
		iterator.getValue().finishLoading();
}

//----------------------------------------------------------------------------------------------------------------------
const TArray<CGPUTextureReference>& CGPURenderObject2D::getGPUTextureReferences() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mGPUTextureReferences;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderObject2D::render(const CGPURenderObject2DIndexes& indexes,
		const SGPURenderObjectRenderInfo& renderInfo) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	S2DOffsetF32&	offset = renderInfo.mOffset;
			SMatrix4x4_32	modelMatrix =
									SMatrix4x4_32()
											.translated(
													S3DOffsetF32(
															mInternals->mScreenPositionPoint.mX + offset.mDX,
															mInternals->mScreenPositionPoint.mY + offset.mDY,
															0.0))
											.scaled(mInternals->mScale.mX, mInternals->mScale.mY, 1.0)
											.translated(
													S3DOffsetF32(
															-mInternals->mAnchorPoint.mX,
															-mInternals->mAnchorPoint.mY,
															0.0))

											.translated(
													S3DOffsetF32(
															mInternals->mAnchorPoint.mX,
															mInternals->mAnchorPoint.mY,
															0.0))
											.rotatedOnZ(-mInternals->mAngleRadians)
											.translated(
													S3DOffsetF32(
															-mInternals->mAnchorPoint.mX,
															-mInternals->mAnchorPoint.mY,
															0.0));

	TCArray<const CGPUTexture>	gpuTextures;
	for (CArray::ItemIndex i = 0; i < mInternals->mGPUTextureReferences.getCount(); i++)
		gpuTextures += mInternals->mGPUTextureReferences[i].getGPUTexture();

	// Draw
	TArray<TIndexRange<UInt16> >		indexRanges = indexes.getRanges();
	TIteratorD<TIndexRange<UInt16> >	iterator = indexRanges.getIterator();

	CGPUVertexShader&					vertexShader =
												renderInfo.mClipPlane.hasValue() ?
														CGPUVertexShader::getClip2DMultiTexture(*renderInfo.mClipPlane) :
														CGPUVertexShader::getBasic2DMultiTexture();
	CGPUFragmentShader&					fragmentShader =
												(renderInfo.mClipPlane.hasValue() || (mInternals->mAlpha != 1.0)) ?
														CGPUFragmentShader::getOpacityMultiTexture(mInternals->mAlpha) :
														CGPUFragmentShader::getBasicMultiTexture();
	CGPURenderState						renderState(kGPURenderMode2D, vertexShader, fragmentShader);

	// Iterate index ranges
	for (; iterator.hasValue(); iterator.advance()) {
		// Get index range
		const	TIndexRange<UInt16>&	indexRange = iterator.getValue();

		// Setup and render
		renderState.setModelMatrix(modelMatrix);
		renderState.setVertexBuffer(mInternals->mGPUVertexBuffer);
		renderState.setTextures(gpuTextures);
		mInternals->mGPU.render(renderState, kGPURenderTypeTriangleStrip,
				(indexRange.mEnd - indexRange.mStart) * 6 + 2 + 2, indexRange.mStart * 6 + 1);
	}
}
