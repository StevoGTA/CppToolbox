//----------------------------------------------------------------------------------------------------------------------
//	COpenGLES11GPU.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------------------------------------------------
// MARK: SOpenGLES11GPUSetupInfo

struct SOpenGLES11GPUSetupInfo {
	// Lifecycle methods
	SOpenGLES11GPUSetupInfo(Float64 scale, void* renderBufferStorageContext) :
		mScale(scale), mRenderBufferStorageContext(renderBufferStorageContext)
		{}

	// Properties
	Float64	mScale;
	void*	mRenderBufferStorageContext;
};
