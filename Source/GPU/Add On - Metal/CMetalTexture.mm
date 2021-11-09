//----------------------------------------------------------------------------------------------------------------------
//	CMetalTexture.mm			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#import "CMetalTexture.h"

#include "CLogServices.h"

#if defined(TARGET_OS_MACOS)
	#define kCVPixelFormatType_Lossless_420YpCbCr8BiPlanarVideoRange '&8v0'
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMetalTextureInternals

class CMetalTextureInternals : public TReferenceCountable<CMetalTextureInternals> {
	public:
		CMetalTextureInternals(id<MTLDevice> device, const CData& data, CGPUTexture::DataFormat dataFormat,
				const S2DSizeU16& size) :
mUsedPixelsSize(size),
			TReferenceCountable(), mTotalPixelsSize(S2DSizeU16(SNumber::getNextPowerOf2(size.mWidth),
					SNumber::getNextPowerOf2(size.mHeight)))
			{
				// Setup
				MTLPixelFormat	pixelFormat;
				NSUInteger		bytesPerRow;
				switch (dataFormat) {
//					case CGPUTexture::kDataFormatRGB565:	pixelFormat = MTLPixelFormatB5G6R5Unorm;
//					case CGPUTexture::kDataFormatRGBA4444:	pixelFormat = MTLPixelFormatABGR4Unorm;
//					case CGPUTexture::kDataFormatRGBA5551:	pixelFormat = MTLPixelFormatBGR5A1Unorm;

					case CGPUTexture::kDataFormatRGBA8888:
						// RGBA8888
						mHasTransparency = true;
						pixelFormat = MTLPixelFormatRGBA8Unorm;
						bytesPerRow = 4 * size.mWidth;
						break;
				}

				MTLTextureDescriptor*	textureDescriptor = [[MTLTextureDescriptor alloc] init];
				textureDescriptor.pixelFormat = pixelFormat;
				textureDescriptor.width = mTotalPixelsSize.mWidth;
				textureDescriptor.height = mTotalPixelsSize.mHeight;
#if defined(TARGET_OS_IOS)
				textureDescriptor.storageMode = MTLStorageModeShared;
#endif
#if defined(TARGET_OS_MACOS)
				textureDescriptor.storageMode = MTLStorageModeManaged;
#endif
				// Create texture
				mTexture = [device newTextureWithDescriptor:textureDescriptor];

				// Load image data
				MTLRegion	region = {{0, 0, 0}, {size.mWidth, size.mHeight, 1}};
				[mTexture replaceRegion:region mipmapLevel:0 withBytes:data.getBytePtr() bytesPerRow:bytesPerRow];
			}
		CMetalTextureInternals(CVMetalTextureCacheRef metalTextureCacheRef, CVImageBufferRef imageBufferRef,
				UInt32 planeIndex) :
			TReferenceCountable(), mHasTransparency(false)
			{
				// Setup
				size_t	width = ::CVPixelBufferGetWidthOfPlane(imageBufferRef, planeIndex);
				size_t	height = ::CVPixelBufferGetHeightOfPlane(imageBufferRef, planeIndex);
				OSType	formatType = ::CVPixelBufferGetPixelFormatType(imageBufferRef);

				// Set pixel format
				MTLPixelFormat	pixelFormat;
				switch (formatType) {
					case kCVPixelFormatType_32ARGB:
					case kCVPixelFormatType_32BGRA:
						// 32 BGRA
						pixelFormat = MTLPixelFormatBGRA8Unorm;
						break;

					case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange:
					case kCVPixelFormatType_Lossless_420YpCbCr8BiPlanarVideoRange:
						// 420 YUV
						pixelFormat = (planeIndex == 0) ? MTLPixelFormatR8Unorm : MTLPixelFormatRG8Unorm;
						break;

					default:
						// Not yet implemented
						pixelFormat = MTLPixelFormatBGRA8Unorm;
						AssertFailUnimplemented();
				}

				// Create CoreVideo Metal texture
				CVMetalTextureRef	metalTextureRef;
				CVReturn			result =
				               			 ::CVMetalTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
				               			 		metalTextureCacheRef, imageBufferRef, nil, pixelFormat, width, height,
				               			 		planeIndex, &metalTextureRef);
				if (result == kCVReturnSuccess) {
					// Success
					mMetalTextureRef = OI<CVMetalTextureRef>(metalTextureRef);
					
					mTexture = ::CVMetalTextureGetTexture(metalTextureRef);
					mUsedPixelsSize = S2DSizeU16(width, height);
					mTotalPixelsSize = S2DSizeU16(mTexture.width, mTexture.height);
				} else
					// Log error
					CLogServices::logError(
							CString(OSSTR("CMetalTexture - error when creating texture from image: ")) +
									CString(result));
			}
		~CMetalTextureInternals()
			{
				// Cleanup
				if (mMetalTextureRef.hasInstance())
					// Release
					::CFRelease(*mMetalTextureRef);
			}

		S2DSizeU16				mUsedPixelsSize;
		S2DSizeU16				mTotalPixelsSize;
		bool					mHasTransparency;
		OI<CVMetalTextureRef>	mMetalTextureRef;
		id<MTLTexture>			mTexture;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMetalTexture

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CMetalTexture::CMetalTexture(id<MTLDevice> device, const CData& data, CGPUTexture::DataFormat dataFormat,
		const S2DSizeU16& size) : CGPUTexture()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CMetalTextureInternals(device, data, dataFormat, size);
}

//----------------------------------------------------------------------------------------------------------------------
CMetalTexture::CMetalTexture(CVMetalTextureCacheRef metalTextureCacheRef, CVImageBufferRef imageBufferRef,
		UInt32 planeIndex) : CGPUTexture()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CMetalTextureInternals(metalTextureCacheRef, imageBufferRef, planeIndex);
}

//----------------------------------------------------------------------------------------------------------------------
CMetalTexture::CMetalTexture(const CMetalTexture& other) : CGPUTexture()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CMetalTexture::~CMetalTexture()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: CGPUTexture methods

//----------------------------------------------------------------------------------------------------------------------
const S2DSizeU16& CMetalTexture::getSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mTotalPixelsSize;
}

// MARK: Temporary methods - will be removed in the future

//----------------------------------------------------------------------------------------------------------------------
const S2DSizeU16& CMetalTexture::getUsedSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mUsedPixelsSize;
}

//----------------------------------------------------------------------------------------------------------------------
id<MTLTexture> CMetalTexture::getMetalTexture() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mTexture;
}

//----------------------------------------------------------------------------------------------------------------------
bool CMetalTexture::hasTransparency() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mHasTransparency;
}
