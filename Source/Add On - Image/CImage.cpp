//----------------------------------------------------------------------------------------------------------------------
//	CImage.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CImage.h"

#include "CppToolboxAssert.h"

// See https://libjpeg-turbo.org
#include "jpeglib.h"
#include "jerror.h"

// See http://www.libpng.org/pub/png/libpng.html
// See https://sourceforge.net/projects/libpng/files/
#include "png.h"

// See https://github.com/andrechen/yuv2rgb

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

struct SJPEGErrorInfo : public jpeg_error_mgr {
	// Properties
	jmp_buf	mSetjmpBuffer;
	char	mErrorMessage[JMSG_LENGTH_MAX];
};

struct SJPEGSourceInfo : public jpeg_source_mgr {
	// Properties
	void*	mUserInfo;
};

struct SPNGDecodeInfo {
	// Properties
	const	UInt8*	mCurrentDataPtr;
};

static	CString	sErrorDomain(OSSTR("CImage"));
static	SError	sErrorUnknownType(sErrorDomain, 1, CString(OSSTR("Unknown type")));
static	SError	sErrorMissingSize(sErrorDomain, 2, CString(OSSTR("Missing size")));
static	SError	sErrorUnableToDecode(sErrorDomain, 3, CString(OSSTR("Unable to decode")));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

static	TIResult<CBitmap>	sDecodeJPEGData(const CData& data);
static	void				sJPEGSourceInit(j_decompress_ptr jpegInfoPtr);
static	void				sJPEGSourceTerminate(j_decompress_ptr jpegInfoPtr);
static	boolean				sJPEGFillInputBuffer(j_decompress_ptr jpegInfoPtr);
static	void				sJPEGSkipInputData(j_decompress_ptr jpegInfoPtr, long byteCount);
static	void				sJPEGErrorExit(j_common_ptr jpegInfoPtr);

static	TIResult<CBitmap>	sDecodeNV12Data(const CData& data, const S2DSizeS32& size);

static	TIResult<CBitmap>	sDecodePNGData(const CData& data);
static	void				sPNGReadWriteProc(png_structp pngPtr, png_bytep dataPtr, png_size_t length);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CImageInternals

class CImageInternals : public TReferenceCountable<CImageInternals> {
	public:
		CImageInternals(const CData& data, const OV<CImage::Type>& type, const OV<S2DSizeS32>& size) :
			TReferenceCountable(), mData(data), mType(type), mSize(size)
			{}

		CData				mData;
		OV<CImage::Type>	mType;
		OV<S2DSizeS32>		mSize;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CImage

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CImage::CImage(const CData& data, const OV<Type>& type, const OV<S2DSizeS32>& size)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CImageInternals(data, type, size);
}

//----------------------------------------------------------------------------------------------------------------------
CImage::CImage(const CImage& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CImage::~CImage()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
TIResult<CBitmap> CImage::getBitmap() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup/Validate image type
	if (!mInternals->mType.hasValue())
		// Determine from data
		mInternals->mType = getTypeFromData(mInternals->mData);
	if (!mInternals->mType.hasValue())
		// Unknown type
		return TIResult<CBitmap>(sErrorUnknownType);

	// Check image type
	switch (*mInternals->mType) {
		case kTypeJPEG:
			// JPEG
			return sDecodeJPEGData(mInternals->mData);

		case kTypePNG:
			// PNG
			return sDecodePNGData(mInternals->mData);

		case kTypeNV12:
			// NV12
			if (!mInternals->mSize.hasValue())
				// Missing size
				return TIResult<CBitmap>(sErrorMissingSize);

			return sDecodeNV12Data(mInternals->mData, *mInternals->mSize);

#if defined(TARGET_OS_WINDOWS)
		default:
			// Just to make compiler happy.  Will never get here.
			return TIResult<CBitmap>(CBitmap());
#endif
	}
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
OV<CImage::Type> CImage::getTypeFromResourceName(const CString& resourceName)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CString	resourceNameUse = resourceName.lowercased();

	// Check name
	if (resourceNameUse.hasSuffix(CString(OSSTR("jpg"))) ||
			resourceNameUse.hasSuffix(CString(OSSTR("jpeg"))) ||
			resourceNameUse.hasSuffix(CString(OSSTR("jpe"))) ||
			resourceNameUse.hasSuffix(CString(OSSTR("jif"))) ||
			resourceNameUse.hasSuffix(CString(OSSTR("jfif"))) ||
			resourceNameUse.hasSuffix(CString(OSSTR("jfi"))))
		// JPEG
		return OV<Type>(kTypeJPEG);
	else if (resourceNameUse.hasSuffix(CString(OSSTR("png"))))
		// PNG
		return OV<Type>(kTypePNG);
	else
		// Unknown
		return OV<Type>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<CImage::Type> CImage::getTypeFromMIMEType(const CString& MIMEType)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CString	MIMETypeUse = MIMEType.lowercased();
	if (!MIMETypeUse.hasPrefix(CString(OSSTR("image/"))))
		MIMETypeUse = CString(OSSTR("image/")) + MIMETypeUse;

	// Check MIME type
	if ((MIMETypeUse == CString(OSSTR("image/jpeg"))) || (MIMETypeUse == CString(OSSTR("image/jpg"))))
		// JPEG
		return OV<Type>(kTypeJPEG);
	else if (MIMETypeUse == CString(OSSTR("image/png")))
		// PNG
		return OV<Type>(kTypePNG);
	else
		// Unknown
		return OV<Type>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<CImage::Type> CImage::getTypeFromData(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	const	void*	bytePtr = data.getBytePtr();
	if ((data.getByteCount() >= 8) && (EndianU32_BtoN(*((const UInt32*) ((const UInt8*) bytePtr + 6))) == 'JFIF'))
		// JPEG
		return OV<Type>(kTypeJPEG);
	else if ((data.getByteCount() >= 4) && (EndianU32_BtoN(*((const UInt32*) bytePtr)) == 0x89504E47))
		// PNG
		return OV<Type>(kTypePNG);
	else
		// Unknown
		return OV<Type>();
}

//----------------------------------------------------------------------------------------------------------------------
OI<CString> CImage::getDefaultFilenameExtension(Type type)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check image type
	switch (type) {
		case kTypeJPEG:	return OI<CString>(CString(OSSTR("jpg")));
		case kTypePNG:	return OI<CString>(CString(OSSTR("png")));
		default:		return OI<CString>();
	}
}

//----------------------------------------------------------------------------------------------------------------------
OI<CString> CImage::getMIMEType(Type type)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check image type
	switch (type) {
		case kTypeJPEG:	return OI<CString>(CString(OSSTR("image/jpeg")));
		case kTypePNG:	return OI<CString>(CString(OSSTR("image/png")));
		default:		return OI<CString>();
	}
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
TIResult<CBitmap> sDecodeJPEGData(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	// Step 1: allocate and initialize JPEG decompression object
	jpeg_decompress_struct	jpegInfo;
	SJPEGErrorInfo			jerr;
	jpegInfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = sJPEGErrorExit;

	// Establish setjmp return context
	if (setjmp(jerr.mSetjmpBuffer)) {
		// If we get here, the JPEG code has signaled an error.
		SError	error(sErrorDomain, -1, CString(jerr.mErrorMessage));
		jpeg_destroy_decompress(&jpegInfo);

		return TIResult<CBitmap>(error);
	}

	// Initialize the decompression object
	jpeg_create_decompress(&jpegInfo);

	// Step 2: Specify data source
	SJPEGSourceInfo	jpegSourceInfo;
	jpegSourceInfo.next_input_byte = nil;
	jpegSourceInfo.bytes_in_buffer = 0;
	jpegSourceInfo.init_source = sJPEGSourceInit;
	jpegSourceInfo.fill_input_buffer = sJPEGFillInputBuffer;
	jpegSourceInfo.skip_input_data = sJPEGSkipInputData;
	jpegSourceInfo.resync_to_restart = jpeg_resync_to_restart;
	jpegSourceInfo.term_source = sJPEGSourceTerminate;
	jpegSourceInfo.mUserInfo = (void*) &data;

	jpegInfo.src = &jpegSourceInfo;

	// Step 3: read parameters
	jpeg_read_header(&jpegInfo, TRUE);

	// Step 4: set decompression parameters

	// Step 5: Start the decompressor
	jpeg_start_decompress(&jpegInfo);

	// Setup bitmap
	CBitmap	bitmap(S2DSizeS32(jpegInfo.output_width, jpegInfo.output_height), CBitmap::kFormatRGB888,
					OV<UInt16>((UInt16) jpegInfo.output_width * (UInt16) jpegInfo.output_components));

	// Step 6: Decompress the image
	JSAMPLE*	bytePtr = (JSAMPLE*) bitmap.getPixelData().getMutableBytePtr();
	UInt16		bytesPerRow = bitmap.getBytesPerRow();
	while (jpegInfo.output_scanline < jpegInfo.output_height) {
//		JSAMPLE*	samplePtr = internals.mBytePtr + jpegInfo.output_scanline * internals.mRowBytes;
		JSAMPLE*	samplePtr = bytePtr + jpegInfo.output_scanline * bytesPerRow;
		jpeg_read_scanlines(&jpegInfo, &samplePtr, 1);
	}

	// Step 7: Finish decompression
	jpeg_finish_decompress(&jpegInfo);

	// Step 8: Cleanup
	jpeg_destroy_decompress(&jpegInfo);

	return TIResult<CBitmap>(bitmap);
}

//----------------------------------------------------------------------------------------------------------------------
void sJPEGSourceInit(j_decompress_ptr jpegInfoPtr)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update
	jpegInfoPtr->src->next_input_byte = nil;
	jpegInfoPtr->src->bytes_in_buffer = 0;
}

//----------------------------------------------------------------------------------------------------------------------
void sJPEGSourceTerminate(j_decompress_ptr jpegInfoPtr)
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
boolean sJPEGFillInputBuffer(j_decompress_ptr jpegInfoPtr)
//----------------------------------------------------------------------------------------------------------------------
{
	if (jpegInfoPtr->src->next_input_byte != nil) {
		// 2nd time, no more data
		(jpegInfoPtr)->err->msg_code = JERR_INPUT_EOF;
		(*(jpegInfoPtr)->err->error_exit)((j_common_ptr) (jpegInfoPtr));

		return FALSE;
	} else {
		// 1st time, point to data
		SJPEGSourceInfo*	sourceInfo = (SJPEGSourceInfo*) jpegInfoPtr->src;
		CData*				imageData = (CData*) sourceInfo->mUserInfo;

		jpegInfoPtr->src->next_input_byte = (const JOCTET*) imageData->getBytePtr();
		jpegInfoPtr->src->bytes_in_buffer = imageData->getByteCount();

		return TRUE;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void sJPEGSkipInputData(j_decompress_ptr jpegInfoPtr, long byteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	if (jpegInfoPtr->src->bytes_in_buffer > (size_t) byteCount) {
		// Advance the byte pointer
		jpegInfoPtr->src->next_input_byte += byteCount;
		jpegInfoPtr->src->bytes_in_buffer -= byteCount;
	} else {
		// Done
		(jpegInfoPtr)->err->msg_code = JERR_INPUT_EOF;
		(*(jpegInfoPtr)->err->error_exit)((j_common_ptr) (jpegInfoPtr));
	}
}

//----------------------------------------------------------------------------------------------------------------------
void sJPEGErrorExit(j_common_ptr jpegInfoPtr)
//----------------------------------------------------------------------------------------------------------------------
{
	SJPEGErrorInfo*	errorInfo = (SJPEGErrorInfo*) jpegInfoPtr->err;

	// Capture error message
	(*jpegInfoPtr->err->format_message)(jpegInfoPtr, errorInfo->mErrorMessage);

	// Done
	longjmp(errorInfo->mSetjmpBuffer, 1);
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CBitmap> sDecodeNV12Data(const CData& data, const S2DSizeS32& size)
//----------------------------------------------------------------------------------------------------------------------
{
	// Based on implementation by Andre Chen, see https://github.com/andrechen/yuv2rgb
	// Width and height must be 2 or greater and even
	if ((size.mWidth < 2) || ((size.mWidth & 1) != 0) || (size.mHeight < 2) || ((size.mHeight & 1) != 0))
		// Failed
		return TIResult<CBitmap>(sErrorUnableToDecode);

	// Setup
//    unsigned char const* y0 = yuv;
//    unsigned char const* uv = yuv + (width*height);
//    int const halfHeight = height>>1;
//    int const halfWidth = width>>1;
	//const	UInt8*	y0Ptr = (UInt8*) data.getBytePtr();
	//const	UInt8*	uvPtr = y0Ptr + size.mWidth * size.mHeight;
	const	SInt32	halfWidth = size.mWidth / 2;
	const	SInt32	halfHeight = size.mHeight / 2;

	CBitmap	bitmap(size, CBitmap::kFormatRGB888);
	UInt8*	bitmapPixelDataPtr = (UInt8*) bitmap.getPixelData().getMutableBytePtr();
	UInt16	bitmapBytesPerRow = bitmap.getBytesPerRow();
//    unsigned char* dst0 = out;
	//UInt8*	rgb0Ptr = (UInt8*) bitmap.getPixelData().getMutableBytePtr();

	// Loop vertially
//    for (int h=0; h<halfHeight; ++h) {
	//for (SInt32 y = 0; y < halfHeight; y++, y0Ptr += size.mWidth, rgb0Ptr += bitmapBytesPerRow) {
	const UInt8* y0Ptr = (UInt8*) data.getBytePtr();
	const UInt8* y1Ptr = y0Ptr + size.mWidth;
	const UInt8* uvPtr = y0Ptr + size.mWidth * size.mHeight;
	for (SInt32 y = 0; y < halfHeight; y++, y0Ptr += size.mWidth, y1Ptr += size.mWidth) {
//        unsigned char const* y1 = y0+width;
		// Setup
		//const	UInt8*	y1Ptr = y0Ptr + size.mWidth;
//        unsigned char* dst1 = dst0 + width*trait::bytes_per_pixel;
				UInt8*	rgb0Ptr = bitmapPixelDataPtr + bitmapBytesPerRow * 2 * y;
				UInt8*	rgb1Ptr = rgb0Ptr + bitmapBytesPerRow;
//        for (int w=0; w<halfWidth; ++w) {
		// Loop horizontally
		for (SInt32 x = 0; x < halfWidth; x++) {
//            // shift
//		    int Y00, Y01, Y10, Y11;
//            Y00 = (*y0++) - 16;  Y01 = (*y0++) - 16;
//            Y10 = (*y1++) - 16;  Y11 = (*y1++) - 16;
			// Get source ys
			SInt32	y00 = (*y0Ptr++) - 16;
			y00 = (y00 > 0) ? 298 * y00 : 0;

			SInt32	y01 = (*y0Ptr++) - 16;
			y01 = (y01 > 0) ? 298 * y01 : 0;

			SInt32	y10 = (*y1Ptr++) - 16;
			y10 = (y10 > 0) ? 298 * y10 : 0;

			SInt32	y11 = (*y1Ptr++) - 16;
			y11 = (y11 > 0) ? 298 * y11 : 0;

//            // U,V or V,U? our trait will make the right call
//		    int V, U;
//            trait::loadvu(U, V, uv);
			// Load U and V
			SInt32	u = (*uvPtr++) - 128;
			SInt32	v = (*uvPtr++) - 128;

			// Calculate RGB adjustments
			SInt32	dR = 128 + 409 * v;
			SInt32	dG = 128 - 100 * u - 208 * v;
			SInt32	dB = 128 + 516 * u;

//            // 2x2 pixels result
//            trait::store_pixel(dst0, Y00 + tR, Y00 + tG, Y00 + tB, alpha);
//            trait::store_pixel(dst0, Y01 + tR, Y01 + tG, Y01 + tB, alpha);
//            trait::store_pixel(dst1, Y10 + tR, Y10 + tG, Y10 + tB, alpha);
//            trait::store_pixel(dst1, Y11 + tR, Y11 + tG, Y11 + tB, alpha);
			// Store RGB
			SInt32	r, g, b;

			r = y00 + dR; g = y00 + dG; b = y00 + dB;
			*rgb0Ptr++ = (r > 0) ? (r < 65535 ? (UInt8) (r >> 8) : 0xff) : 0;
			*rgb0Ptr++ = (g > 0) ? (g < 65535 ? (UInt8) (g >> 8) : 0xff) : 0;
			*rgb0Ptr++ = (b > 0) ? (b < 65535 ? (UInt8) (b >> 8) : 0xff) : 0;

			r = y01 + dR; g = y01 + dG; b = y01 + dB;
			*rgb0Ptr++ = (r > 0) ? (r < 65535 ? (UInt8) (r >> 8) : 0xff) : 0;
			*rgb0Ptr++ = (g > 0) ? (g < 65535 ? (UInt8) (g >> 8) : 0xff) : 0;
			*rgb0Ptr++ = (b > 0) ? (b < 65535 ? (UInt8) (b >> 8) : 0xff) : 0;

			r = y10 + dR; g = y10 + dG; b = y10 + dB;
			*rgb1Ptr++ = (r > 0) ? (r < 65535 ? (UInt8) (r >> 8) : 0xff) : 0;
			*rgb1Ptr++ = (g > 0) ? (g < 65535 ? (UInt8) (g >> 8) : 0xff) : 0;
			*rgb1Ptr++ = (b > 0) ? (b < 65535 ? (UInt8) (b >> 8) : 0xff) : 0;

			r = y11 + dR; g = y11 + dG; b = y11 + dB;
			*rgb1Ptr++ = (r > 0) ? (r < 65535 ? (UInt8) (r >> 8) : 0xff) : 0;
			*rgb1Ptr++ = (g > 0) ? (g < 65535 ? (UInt8) (g >> 8) : 0xff) : 0;
			*rgb1Ptr++ = (b > 0) ? (b < 65535 ? (UInt8) (b >> 8) : 0xff) : 0;
        }
//        y0 = y1;
//        dst0 = dst1;
    }

	return TIResult<CBitmap>(bitmap);
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CBitmap> sDecodePNGData(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	png_struct*	pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nil, nil, nil);
	if (pngPtr == nil)
		return TIResult<CBitmap>(sErrorUnableToDecode);

	png_info*	pngInfoPtr = png_create_info_struct(pngPtr);
	if (pngInfoPtr == nil) {
		png_destroy_read_struct(&pngPtr, nil, nil);

		return TIResult<CBitmap>(sErrorUnableToDecode);
	}

	// Set error handling
	if (setjmp(png_jmpbuf(pngPtr))) {
		// Error handling
		png_destroy_read_struct(&pngPtr, &pngInfoPtr, nil);

		return TIResult<CBitmap>(sErrorUnableToDecode);
	}

	// Initialize
	SPNGDecodeInfo	pngDecodeInfo;
	pngDecodeInfo.mCurrentDataPtr = (const UInt8*) data.getBytePtr();

	png_set_read_fn(pngPtr, &pngDecodeInfo, sPNGReadWriteProc);

	/* if you are using replacement message functions, here you would call */
//	png_set_message_fn(pngPtr, (void *)msg_ptr, user_error_fn, user_warning_fn);
	/* where msg_ptr is a structure you want available to the callbacks */

	// Read info
	png_read_info(pngPtr, pngInfoPtr);

	int			bitDepth, colorType;
	png_uint_32	width, height;
	png_get_IHDR(pngPtr, pngInfoPtr, &width, &height, &bitDepth, &colorType, nil, nil, nil);

	// Activate options
	// Strip 16 bit files down to 8 bits
//	png_set_strip_16(pngPtr);

//	// Strip alpha bytes from the input data without combining with the background (not recommended)
//	png_set_strip_alpha(pngPtr);

//	png_set_invert_alpha(pngPtr);

//	png_set_invert_mono(pngPtr);

	if ((colorType == PNG_COLOR_TYPE_GRAY) || (colorType == PNG_COLOR_TYPE_GRAY_ALPHA))
		png_set_gray_to_rgb(pngPtr);

	if (colorType == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(pngPtr);

//	if (png_get_valid(pngPtr, pngInfoPtr, PNG_INFO_tRNS))
//		png_set_tRNS_to_alpha(pngPtr);

#if 0
	/* Set the background color to draw transparent and alpha images over */
	png_color_16 my_background;
	my_background.index = 0;
	my_background.red = 0;
	my_background.green = 0;
	my_background.blue = 0;
	my_background.gray = 0;

//	if (pngInfoPtr->valid & PNG_INFO_bKGD)
//		png_set_background(pngPtr, &(pngInfoPtr->background), PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
//	else
		png_set_background(pngPtr, &my_background, PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);
//		png_set_background(pngPtr, &my_background, PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
#endif

//	/* tell libpng to handle the gamma conversion for you */
//	if (pngInfoPtr->valid & PNG_INFO_gAMA)
//		png_set_gamma(pngPtr, screen_gamma, pngInfoPtr->gamma);
//	else
//		png_set_gamma(pngPtr, screen_gamma, 0.45);

//	/* shift the pixels down to their true bit depth */
//	if ((pngInfoPtr->valid & PNG_INFO_sBIT) && (pngInfoPtr->bit_depth > pngInfoPtr->sig_bit))
//		png_set_shift(pngPtr, &(pngInfoPtr->sig_bit));

	/* pack pixels into bytes */
	if (bitDepth < 8)
		png_set_packing(pngPtr);

//	/* flip the rgb pixels to bgr */
//	if ((pngInfoPtr->color_type == PNG_COLOR_TYPE_RGB) ||
//			(pngInfoPtr->color_type == PNG_COLOR_TYPE_RGB_ALPHA))
//		png_set_bgr(pngPtr);

//	/* swap bytes of 16 bit files to least significant bit first */
//	if (pngInfoPtr->bit_depth == 16)
//		png_set_swap(pngPtr);

//	/* add a filler byte to rgb files */
//	if ((pngInfoPtr->bit_depth == 8) && (pngInfoPtr->color_type == PNG_COLOR_TYPE_RGB))
//		png_set_filler(pngPtr, 0xff, PNG_FILLER_AFTER);

	// Update info
	png_read_update_info(pngPtr, pngInfoPtr);

	// Setup bitmap
//	internals.setup(S2DSizeS32(width, height),
//			(colorType == PNG_COLOR_TYPE_RGB) ? kCBitmapFormatRGB888 : kCBitmapFormatRGBA8888,
//			png_get_rowbytes(pngPtr, pngInfoPtr));
	CBitmap	bitmap(S2DSizeS32(width, height),
					(colorType == PNG_COLOR_TYPE_RGB) ? CBitmap::kFormatRGB888 : CBitmap::kFormatRGBA8888,
					(UInt16) png_get_rowbytes(pngPtr, pngInfoPtr));

	// Read the image
	png_bytepp	rowPointers = (png_bytepp) ::malloc(height * sizeof(png_bytep));
	png_bytep	bytePtr = (png_bytep) bitmap.getPixelData().getMutableBytePtr();
	for (UInt32 i = 0; i < height; i++)
//		rowPointers[i] = internals.mBytePtr + i * internals.mRowBytes;
		rowPointers[i] = bytePtr + i * bitmap.getBytesPerRow();
	png_read_image(pngPtr, rowPointers);

//#if TARGET_UI_ANDROID
//	// Apply pre-multiplied alpha to match CoreGraphics on iOS
//	for (UInt32 y = 0; y < height; y++) {
//		UInt32*	bytePtr = (UInt32*) rowPointers[y];
//		for (UInt32 x = 0; x < bitmap.getBytesPerRow(); x += 4, bytePtr++) {
//			UInt32	alpha = (*bytePtr >> 24) & 0xFF;
//			UInt32	red = (*bytePtr >> 16) & 0xFF;
//			UInt32	green = (*bytePtr >> 8) & 0xFF;
//			UInt32	blue = (*bytePtr >> 0) & 0xFF;
//
//			red = (red * alpha) >> 8;
//			green = (green * alpha) >> 8;
//			blue = (blue * alpha) >> 8;
//
//			*bytePtr = (alpha << 24) | (red << 16) | (green << 8) | (blue << 0);
//		}
//	}
//#endif

	// Cleanup
	png_destroy_read_struct(&pngPtr, &pngInfoPtr, nil);
	free(rowPointers);

	return TIResult<CBitmap>(bitmap);
}

//----------------------------------------------------------------------------------------------------------------------
void sPNGReadWriteProc(png_structp pngPtr, png_bytep dataPtr, png_size_t length)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SPNGDecodeInfo*	pngDecodeInfo = (SPNGDecodeInfo*) png_get_io_ptr(pngPtr);

	// Just copy bytes
	memcpy(dataPtr, pngDecodeInfo->mCurrentDataPtr, length);
	pngDecodeInfo->mCurrentDataPtr += length;
}
