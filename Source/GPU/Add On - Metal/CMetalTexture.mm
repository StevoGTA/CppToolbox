//----------------------------------------------------------------------------------------------------------------------
//	CMetalTexture.mm			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#import "CMetalTexture.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMetalTextureInternals

class CMetalTextureInternals : public TReferenceCountable<CMetalTextureInternals> {
	public:
		CMetalTextureInternals(id<MTLDevice> device, const CData& data, EGPUTextureDataFormat gpuTextureDataFormat,
				const S2DSizeU16& size) :
mUsedPixelsSize(size),
			mTotalPixelsSize(S2DSizeU16(SNumber::getNextPowerOf2(size.mWidth), SNumber::getNextPowerOf2(size.mHeight)))
			{
				// Setup
				NSUInteger		bytesPerRow;
				switch (gpuTextureDataFormat) {
//					case kGPUTextureDataFormatRGB565:	pixelFormat = MTLPixelFormatB5G6R5Unorm;
//					case kGPUTextureDataFormatRGBA4444:	pixelFormat = MTLPixelFormatABGR4Unorm;
//					case kGPUTextureDataFormatRGBA5551:	pixelFormat = MTLPixelFormatBGR5A1Unorm;

					case kGPUTextureDataFormatRGBA8888:
						// RGBA8888
						mPixelFormat = MTLPixelFormatRGBA8Unorm;
						bytesPerRow = 4 * size.mWidth;
						break;
				}

				MTLTextureDescriptor*	textureDescriptor = [[MTLTextureDescriptor alloc] init];
				textureDescriptor.pixelFormat = mPixelFormat;
				textureDescriptor.width = mTotalPixelsSize.mWidth;
				textureDescriptor.height = mTotalPixelsSize.mHeight;
#if TARGET_OS_IOS
				textureDescriptor.storageMode = MTLStorageModeShared;
#endif
#if TARGET_OS_MACOS
				textureDescriptor.storageMode = MTLStorageModeManaged;
#endif
				// Create texture
				mTexture = [device newTextureWithDescriptor:textureDescriptor];

				// Load image data
				MTLRegion	region = {{0, 0, 0}, {size.mWidth, size.mHeight, 1}};
				[mTexture replaceRegion:region mipmapLevel:0 withBytes:data.getBytePtr() bytesPerRow:bytesPerRow];
			}

	S2DSizeU16		mUsedPixelsSize;
	S2DSizeU16		mTotalPixelsSize;
	MTLPixelFormat	mPixelFormat;
	id<MTLTexture>	mTexture;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMetalTexture

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CMetalTexture::CMetalTexture(id<MTLDevice> device, const CData& data, EGPUTextureDataFormat gpuTextureDataFormat,
		const S2DSizeU16& size)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CMetalTextureInternals(device, data, gpuTextureDataFormat, size);
}

//----------------------------------------------------------------------------------------------------------------------
CMetalTexture::CMetalTexture(const CMetalTexture& other)
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
	switch (mInternals->mPixelFormat) {
		case MTLPixelFormatRGBA8Unorm:
			// Have transparency
			return true;

		default:
			// No transparency
			return false;
	}
}
