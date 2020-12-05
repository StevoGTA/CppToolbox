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

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

struct SJPEGErrorInfo : public jpeg_error_mgr {
	// Properties
	jmp_buf	mSetjmpBuffer;
};

struct SJPEGSourceInfo : public jpeg_source_mgr {
	// Properties
	void*	mUserInfo;
};

struct SPNGDecodeInfo {
	// Properties
	const	UInt8*	mCurrentDataPtr;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

static	CBitmap	sDecodeJPEGData(const CData& data);
static	void	sJPEGSourceInit(j_decompress_ptr jpegInfoPtr);
static	void	sJPEGSourceTerminate(j_decompress_ptr jpegInfoPtr);
static	boolean	sJPEGFillInputBuffer(j_decompress_ptr jpegInfoPtr);
static	void	sJPEGSkipInputData(j_decompress_ptr jpegInfoPtr, long byteCount);
static	void	sJPEGErrorExit(j_common_ptr jpegInfoPtr);

static	CBitmap	sDecodePNGData(const CData& data);
static	void	sPNGReadWriteProc(png_structp pngPtr, png_bytep dataPtr, png_size_t length);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CImageInternals

class CImageInternals : public TReferenceCountable<CImageInternals> {
	public:
		CImageInternals(const CByteParceller& byteParceller, OV<CImage::Type> type) :
			TReferenceCountable(), mByteParceller(byteParceller), mType(type)
			{}

		const	CByteParceller		mByteParceller;
				OV<CImage::Type>	mType;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CImage

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CImage::CImage(const CByteParceller& byteParceller, OV<Type> type)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CImageInternals(byteParceller, type);
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
CBitmap CImage::getBitmap() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get data
	OI<SError>	error;
	CData		data = mInternals->mByteParceller.readData(error);
	mInternals->mByteParceller.reset();
	ReturnValueIfError(error, CBitmap());

	// Setup/Validate image type
	if (!mInternals->mType.hasValue())
		// Determine from data
		mInternals->mType = getTypeFromData(data);
	AssertFailIf(!mInternals->mType.hasValue());

	// Check image type
	switch (*mInternals->mType) {
		case kTypeJPEG:
			// JPEG
			return sDecodeJPEGData(data);

		case kTypePNG:
			// PNG
			return sDecodePNGData(data);

#if TARGET_OS_WINDOWS
		default:
			// Just to make compiler happy.  Will never get here.
			return CBitmap();
#endif
	}
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CBitmap CImage::getBitmap(const CByteParceller& byteParceller)
//----------------------------------------------------------------------------------------------------------------------
{
	return CImage(byteParceller).getBitmap();
}

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
	if ((data.getSize() >= 8) && (EndianU32_BtoN(*((const UInt32*) ((const UInt8*) bytePtr + 6))) == 'JFIF'))
		// JPEG
		return OV<Type>(kTypeJPEG);
	else if ((data.getSize() >= 4) && (EndianU32_BtoN(*((const UInt32*) bytePtr)) == 0x89504E47))
		// PNG
		return OV<Type>(kTypePNG);
	else
		// Unknown
		return OV<Type>();
}

//----------------------------------------------------------------------------------------------------------------------
CString CImage::getDefaultFilenameExtension(Type type)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check image type
	switch (type) {
		case kTypeJPEG:	return CString(OSSTR(".jpg"));
		case kTypePNG:	return CString(OSSTR(".png"));
		default:		return CString::mEmpty;
	}
}

//----------------------------------------------------------------------------------------------------------------------
CString CImage::getMIMEType(Type type)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check image type
	switch (type) {
		case kTypeJPEG:	return CString(OSSTR("image/jpeg"));
		case kTypePNG:	return CString(OSSTR("image/png"));
		default:		return CString::mEmpty;
	}
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
CBitmap sDecodeJPEGData(const CData& data)
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
		jpeg_destroy_decompress(&jpegInfo);

		return CBitmap();
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
					jpegInfo.output_width * jpegInfo.output_components);

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

	return bitmap;
}

//----------------------------------------------------------------------------------------------------------------------
void sJPEGSourceInit(j_decompress_ptr jpegInfoPtr)
//----------------------------------------------------------------------------------------------------------------------
{
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
		jpegInfoPtr->src->bytes_in_buffer = imageData->getSize();

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

	// Display error message
	(*jpegInfoPtr->err->output_message)(jpegInfoPtr);

	// Done
	longjmp(errorInfo->mSetjmpBuffer, 1);
}

//----------------------------------------------------------------------------------------------------------------------
CBitmap sDecodePNGData(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	png_struct*	pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nil, nil, nil);
	if (pngPtr == nil)
		return CBitmap();

	png_info*	pngInfoPtr = png_create_info_struct(pngPtr);
	if (pngInfoPtr == nil) {
		png_destroy_read_struct(&pngPtr, nil, nil);

		return CBitmap();
	}

	// Set error handling
	if (setjmp(png_jmpbuf(pngPtr))) {
		// Error handling
		png_destroy_read_struct(&pngPtr, &pngInfoPtr, nil);

		return CBitmap();
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

	return bitmap;
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
