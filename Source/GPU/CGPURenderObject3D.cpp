//----------------------------------------------------------------------------------------------------------------------
//	CGPURenderObject3D.cpp			©2018 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CGPURenderObject3D.h"

#include "CReferenceCountable.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPURenderObject3D::Internals

class CGPURenderObject3D::Internals : public TReferenceCountableAutoDelete<Internals> {
	public:
				Internals(CGPU& gpu, const CData& vertexData, UInt32 indexCount,
						const CData& indexData, CGPUVertexShader& vertexShader, CGPUFragmentShader& fragmentShader) :
					TReferenceCountableAutoDelete(), mGPU(gpu),
							mGPUVertexBuffer(
									mGPU.allocateVertexBuffer(vertexShader.getPerVertexByteCount(), vertexData)),
							mIndexCount(indexCount), mGPUIndexBuffer(mGPU.allocateIndexBuffer(indexData)),
							mVertexShader(vertexShader), mFragmentShader(fragmentShader)
					{}
				~Internals()
					{
						// Cleanup
						mGPU.disposeBuffer(mGPUVertexBuffer);
						mGPU.disposeBuffer(mGPUIndexBuffer);
					}

		CGPU&				mGPU;
		SGPUVertexBuffer	mGPUVertexBuffer;
		UInt32				mIndexCount;
		SGPUBuffer			mGPUIndexBuffer;
		CGPUVertexShader&	mVertexShader;
		CGPUFragmentShader& mFragmentShader;

		S3DPointF32			mRotation;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPURenderObject3D

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPURenderObject3D::CGPURenderObject3D(CGPU& gpu, const CData& vertexData, UInt32 indexCount, const CData& indexData,
		CGPUVertexShader& vertexShader, CGPUFragmentShader& fragmentShader) : CGPURenderObject()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(gpu, vertexData, indexCount, indexData, vertexShader, fragmentShader);
}

//----------------------------------------------------------------------------------------------------------------------
CGPURenderObject3D::CGPURenderObject3D(const CGPURenderObject3D& other) : CGPURenderObject(other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CGPURenderObject3D::~CGPURenderObject3D()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
S3DPointF32 CGPURenderObject3D::getRotationAsRadians() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mRotation;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderObject3D::setRotationAsRadians(S3DPointF32 rotationRadians)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mRotation = rotationRadians;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderObject3D::finishLoading() const
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderObject3D::render() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup render state
	CGPURenderState	renderState(CGPURenderState::kMode3D, mInternals->mVertexShader, mInternals->mFragmentShader);

	SMatrix4x4_32	modelMatrix =
							SMatrix4x4_32()
									.rotatedOnX(mInternals->mRotation.mX)
									.rotatedOnY(mInternals->mRotation.mY)
									.rotatedOnZ(mInternals->mRotation.mZ);
	renderState.setModelMatrix(modelMatrix);

	renderState.setVertexBuffer(mInternals->mGPUVertexBuffer);
	renderState.setIndexBuffer(mInternals->mGPUIndexBuffer);

	// Render
	mInternals->mGPU.renderIndexed(renderState, CGPU::kRenderTypeTriangleList, mInternals->mIndexCount, 0);
}
