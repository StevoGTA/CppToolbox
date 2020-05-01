//----------------------------------------------------------------------------------------------------------------------
//	CGPUTextureManager.h			©2019 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CBitmap.h"
#include "CByteParceller.h"
#include "CGPU.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: EGPUTextureReferenceOptions

enum EGPUTextureReferenceOptions {
	kGPUTextureReferenceOptionsNone				= 0,
	kGPUTextureReferenceOptionsLoadImmediately	= 1 << 0,
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Procs

typedef	CBitmap	(*CGPUTextureManagerBitmapProc)(const CByteParceller& byteParceller);

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
				void			load() const;
				void			finishLoading() const;

		const	CGPUTexture&	getGPUTexture() const;

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
								CGPUTextureManager(CGPU& gpu);
								CGPUTextureManager(const CGPUTextureManager& other);
								~CGPUTextureManager();

								// Instance methods
		CGPUTextureReference	gpuTextureReference(const CBitmap& bitmap,
												const OR<const CString>& reference = OR<const CString>(),
												EGPUTextureReferenceOptions
														gpuTextureReferenceOptions = kGPUTextureReferenceOptionsNone);
		CGPUTextureReference	gpuTextureReference(const CBitmap& bitmap, EGPUTextureDataFormat gpuTextureDataFormat,
												const OR<const CString>& reference = OR<const CString>(),
												EGPUTextureReferenceOptions
														gpuTextureReferenceOptions = kGPUTextureReferenceOptionsNone);
		CGPUTextureReference	gpuTextureReference(const CByteParceller& byteParceller,
												CGPUTextureManagerBitmapProc bitmapProc,
												const OR<const CString>& reference = OR<const CString>(),
												EGPUTextureReferenceOptions
														gpuTextureReferenceOptions = kGPUTextureReferenceOptionsNone);
		CGPUTextureReference	gpuTextureReference(const CByteParceller& byteParceller,
												CGPUTextureManagerBitmapProc bitmapProc,
												EGPUTextureDataFormat gpuTextureDataFormat,
												const OR<const CString>& reference = OR<const CString>(),
												EGPUTextureReferenceOptions
														gpuTextureReferenceOptions = kGPUTextureReferenceOptionsNone);
		CGPUTextureReference	gpuTextureReference(const CByteParceller& byteParceller,
												EGPUTextureDataFormat gpuTextureDataFormat, S2DSizeU16 size,
												const OR<const CString>& reference = OR<const CString>(),
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
