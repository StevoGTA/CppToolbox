//----------------------------------------------------------------------------------------------------------------------
//	CGPURenderObject.h			©2018 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPU.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPURenderObject

class CGPURenderObject {
	// Structs
	public:
		struct RenderInfo {
						// Lifecycle methods
						RenderInfo() {}
						RenderInfo(const S2DOffsetF32& offset) : mOffset(offset) {}
						RenderInfo(SMatrix4x1_32 clipPlane) : mClipPlane(clipPlane) {}
						RenderInfo(const RenderInfo& other, const S2DOffsetF32& offset) :
							mOffset(offset), mClipPlane(other.mClipPlane)
							{}

						// Instance methods
			RenderInfo	offset(const S2DOffsetF32& offset) const
							{ return RenderInfo(*this, mOffset + offset); }

			// Properties
			S2DOffsetF32		mOffset;
			OV<SMatrix4x1_32>	mClipPlane;
		};

	// Methods
	public:
				// Lifecycle methods
				CGPURenderObject() {}
				CGPURenderObject(const CGPURenderObject& other) {}
		virtual	~CGPURenderObject() {}
};
