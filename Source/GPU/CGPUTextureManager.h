//----------------------------------------------------------------------------------------------------------------------
//	CGPUTextureManager.h			©2019 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CBitmap.h"
#include "CByteParceller.h"
#include "CGPURenderEngine.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: EGPUTextureReferenceOptions

enum EGPUTextureReferenceOptions {
	kGPUTextureReferenceOptionsNone				= 0,
	kGPUTextureReferenceOptionsLoadImmediately	= 1 << 0,
};

////----------------------------------------------------------------------------------------------------------------------
//// MARK: - SGPUTextureManagerProcs
//
//class CGPUTextureReference;
//
//typedef void*	(*CGPUTextureManagerRegisterTextureProc)(const CGPUTexture& gpuTexture);
//typedef	void	(*CGPUTextureManagerUnregisterTextureProc)(void* reference);
//
//struct SGPUTextureManagerProcs {
//	// Lifecycle methods
//	SGPUTextureManagerProcs(CGPUTextureManagerRegisterTextureProc registerTextureProc,
//			CGPUTextureManagerUnregisterTextureProc unregisterTextureProc) :
//		mRegisterTextureProc(registerTextureProc), mUnregisterTextureProc(unregisterTextureProc)
//		{}
//
//	// Properties
//	CGPUTextureManagerRegisterTextureProc	mRegisterTextureProc;
//	CGPUTextureManagerUnregisterTextureProc	mUnregisterTextureProc;
//};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUTextureReference

class CGPUTextureReferenceInternals;
class CGPUTextureReference {
	// Methods
	public:
												// Lifecycle methods
												CGPUTextureReference(CGPUTextureReferenceInternals* internals);
												CGPUTextureReference(const CGPUTextureReference& other);
												~CGPUTextureReference();

												// Instance methods
				bool							getIsLoaded() const;
				void							load() const;
				void							finishLoading() const;

		const	SGPURenderEngineTextureInfo&	getGPURenderingEngineTextureInfo() const;

	// Properties
	private:
		CGPUTextureReferenceInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUTextureManager

class CGPUTextureManagerInternals;
class CGPUTextureManager {
	// Methods
	public:
								// Lifecycle methods
								CGPUTextureManager(CGPURenderEngine& gpuRenderEngine);
								CGPUTextureManager(const CGPUTextureManager& other);
								~CGPUTextureManager();

								// Instance methods
		CGPUTextureReference	gpuTextureReference(const CBitmap& bitmap,
												OR<const CString> reference = OR<const CString>(),
												EGPUTextureReferenceOptions
														gpuTextureReferenceOptions = kGPUTextureReferenceOptionsNone);
		CGPUTextureReference	gpuTextureReference(const CBitmap& bitmap, EGPUTextureFormat gpuTextureFormat,
												OR<const CString> reference = OR<const CString>(),
												EGPUTextureReferenceOptions
														gpuTextureReferenceOptions = kGPUTextureReferenceOptionsNone);
		CGPUTextureReference	gpuTextureReference(const CByteParceller& imageByteParceller,
												OR<const CString> reference = OR<const CString>(),
												EGPUTextureReferenceOptions
														gpuTextureReferenceOptions = kGPUTextureReferenceOptionsNone);
		CGPUTextureReference	gpuTextureReference(const CByteParceller& imageByteParceller,
												EGPUTextureFormat gpuTextureFormat,
												OR<const CString> reference = OR<const CString>(),
												EGPUTextureReferenceOptions
														gpuTextureReferenceOptions = kGPUTextureReferenceOptionsNone);
		CGPUTextureReference	gpuTextureReference(const CByteParceller& textureByteParceller,
												EGPUTextureFormat gpuTextureFormat, SGPUTextureSize gpuTextureSize,
												OR<const CString> reference = OR<const CString>(),
												EGPUTextureReferenceOptions
														gpuTextureReferenceOptions = kGPUTextureReferenceOptionsNone);

		void					loadAll();
		void					pauseLoading();
		void					resumeLoading();
		void					unloadAll();

	// Properties
	private:
		CGPUTextureManagerInternals*	mInternals;
};
