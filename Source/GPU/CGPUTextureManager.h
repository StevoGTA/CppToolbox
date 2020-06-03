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
class CGPUTextureReference : public CEquatable {
	// Methods
	public:
										// Lifecycle methods
										CGPUTextureReference(CGPUTextureReferenceInternals& internals);
										CGPUTextureReference(const CGPUTextureReference& other);
										~CGPUTextureReference();

										// CEquatable methods
				bool					operator==(const CEquatable& other) const
											{ return mInternals == ((const CGPUTextureReference&) other).mInternals; }

										// Instance methods
		const	CString&				getReference() const;

				void					load() const;
				void					finishLoading() const;
				void					unload() const;

		const	CGPUTexture&			getGPUTexture() const;

				CGPUTextureReference&	operator=(const CGPUTextureReference& other);

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
