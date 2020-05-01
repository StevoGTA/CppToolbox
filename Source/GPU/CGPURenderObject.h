//----------------------------------------------------------------------------------------------------------------------
//	CGPURenderObject.h			©2018 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPU.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SGPURenderObjectRenderInfo

struct SGPURenderObjectRenderInfo {
										// Lifecycle methods
										SGPURenderObjectRenderInfo() {}
										SGPURenderObjectRenderInfo(const S2DOffsetF32& offset) : mOffset(offset) {}
										SGPURenderObjectRenderInfo(SMatrix4x1_32 clipPlane) : mClipPlane(clipPlane) {}

										// Instance methods
	inline	SGPURenderObjectRenderInfo	offset(const S2DOffsetF32& offset) const
											{
												// Copy and update offset
												SGPURenderObjectRenderInfo	renderInfo(*this);
												renderInfo.mOffset = renderInfo.mOffset + offset;

												return renderInfo;
											}

	// Properties
	S2DOffsetF32		mOffset;
	OV<SMatrix4x1_32>	mClipPlane;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPURenderObject

class CGPURenderObject {
	// Methods
	public:
				// Lifecycle methods
				CGPURenderObject() {}
				CGPURenderObject(const CGPURenderObject& other) {}
		virtual	~CGPURenderObject() {}
};
