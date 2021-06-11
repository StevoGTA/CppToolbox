//----------------------------------------------------------------------------------------------------------------------
//	CGPUTextureManager.h			©2019 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CBitmap.h"
#include "CGPU.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPUTextureReference

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

		const	I<CGPUTexture>&			getGPUTexture() const;

				CGPUTextureReference&	operator=(const CGPUTextureReference& other);

	// Properties
	private:
		CGPUTextureReferenceInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUTextureManager

class CGPUTextureManagerInternals;
class CGPUTextureManager {
	// Enums
	public:
		enum ReferenceOptions {
			kReferenceOptionsNone				= 0,
			kReferenceOptionsLoadImmediately	= 1 << 0,
		};

	// Procs
	public:
		typedef	CBitmap	(*BitmapProc)(const CData& data);

	// Methods
	public:
								// Lifecycle methods
								CGPUTextureManager(CGPU& gpu);
								CGPUTextureManager(const CGPUTextureManager& other);
								~CGPUTextureManager();

								// Instance methods
		CGPUTextureReference	gpuTextureReference(const CBitmap& bitmap,
												const OR<const CString>& reference = OR<const CString>(),
												ReferenceOptions referenceOptions = kReferenceOptionsNone);
		CGPUTextureReference	gpuTextureReference(const CBitmap& bitmap, CGPUTexture::DataFormat dataFormat,
												const OR<const CString>& reference = OR<const CString>(),
												ReferenceOptions referenceOptions = kReferenceOptionsNone);
		CGPUTextureReference	gpuTextureReference(const I<CDataSource>& dataSource, BitmapProc bitmapProc,
												const OR<const CString>& reference = OR<const CString>(),
												ReferenceOptions referenceOptions = kReferenceOptionsNone);
		CGPUTextureReference	gpuTextureReference(const I<CDataSource>& dataSource, BitmapProc bitmapProc,
												CGPUTexture::DataFormat dataFormat,
												const OR<const CString>& reference = OR<const CString>(),
												ReferenceOptions referenceOptions = kReferenceOptionsNone);
		CGPUTextureReference	gpuTextureReference(const I<CDataSource>& dataSource,
												CGPUTexture::DataFormat dataFormat, S2DSizeU16 size,
												const OR<const CString>& reference = OR<const CString>(),
												ReferenceOptions referenceOptions = kReferenceOptionsNone);
		CGPUTextureReference	gpuTextureReference(const I<CGPUTexture>& gpuTexture);

		void					loadAll();
		void					pauseLoading();
		void					resumeLoading();
		void					unloadAll();

	// Properties
	private:
		CGPUTextureManagerInternals*	mInternals;
};
