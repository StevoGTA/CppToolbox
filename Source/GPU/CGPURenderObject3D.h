//----------------------------------------------------------------------------------------------------------------------
//	CGPURenderObject3D.h			©2018 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPURenderObject.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPURenderObject3D

class CGPURenderObject3D : public CGPURenderObject {
	// Classes
	private:
		class Internals;

	// Methods
	public:
							// Lifecycle methods
							CGPURenderObject3D(CGPU& gpu, const CData& vertexData, UInt32 indexCount,
									const CData& indexData, CGPUVertexShader& vertexShader,
									CGPUFragmentShader& fragmentShader);
							CGPURenderObject3D(const CGPURenderObject3D& other);
							~CGPURenderObject3D();

							// Instance methods
		S3DPointF32			getRotationAsRadians() const;
		void				setRotationAsRadians(S3DPointF32 rotationRadians);
		S3DPointF32			getRotationAsDegrees() const
								{
									// Setup
									S3DPointF32	rotationAsRadians = getRotationAsRadians();

									return S3DPointF32(T2DUtilities<Float32>::toDegrees(rotationAsRadians.mX),
											T2DUtilities<Float32>::toDegrees(rotationAsRadians.mY),
											T2DUtilities<Float32>::toDegrees(rotationAsRadians.mZ));
								}
		void				setRotationAsDegrees(S3DPointF32 rotationDegrees)
								{
									setRotationAsRadians(
											S3DPointF32(T2DUtilities<Float32>::toRadians(rotationDegrees.mX),
											T2DUtilities<Float32>::toRadians(rotationDegrees.mY),
											T2DUtilities<Float32>::toRadians(rotationDegrees.mZ)));
								}

		void				finishLoading() const;

		void				render() const;

		CGPURenderObject3D&	operator=(const CGPURenderObject3D& other);

	// Properties
	private:
		Internals*	mInternals;
};
