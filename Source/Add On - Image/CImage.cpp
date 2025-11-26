//----------------------------------------------------------------------------------------------------------------------
//	CImage.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CImage.h"

#include "CppToolboxAssert.h"
#include "CReferenceCountable.h"

// See https://libjpeg-turbo.org
#include "jpeglib.h"
#include "jerror.h"

#if defined(TARGET_OS_WINDOWS)
	#pragma comment(lib, "turbojpeg-static.lib")
#endif


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

static	CString	sErrorDomain(OSSTR("CImage"));
static	SError	sErrorUnknownType(sErrorDomain, 1, CString(OSSTR("Unknown type")));
static	SError	sErrorMissingSize(sErrorDomain, 2, CString(OSSTR("Missing size")));
static	SError	sErrorUnableToDecode(sErrorDomain, 3, CString(OSSTR("Unable to decode")));
static	SError	sErrorUnableToEncode(sErrorDomain, 4, CString(OSSTR("Unable to encode")));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

static	TVResult<CBitmap>	sDecodeJPEGData(const CData& data);
static	TVResult<CData>		sEncodeJPEGData(const CBitmap& bitmap);
static	void				sJPEGDestinationInit(j_compress_ptr jpegInfoPtr);
static	boolean				sJPEGDestinationEmptyBuffer(j_compress_ptr jpegInfoPtr);
static	void				sJPEGDestinationTerminate(j_compress_ptr jpegInfoPtr);
static	void				sJPEGSourceInit(j_decompress_ptr jpegInfoPtr);
static	void				sJPEGSourceTerminate(j_decompress_ptr jpegInfoPtr);
static	boolean				sJPEGFillInputBuffer(j_decompress_ptr jpegInfoPtr);
static	void				sJPEGSkipInputData(j_decompress_ptr jpegInfoPtr, long byteCount);
static	void				sJPEGErrorExit(j_common_ptr jpegInfoPtr);

static	void				sDecodeNV12Data(const CData& data, const S2DSizeS32& size, CBitmap& bitmap,
									const S2DRectS32& rect);

static	TVResult<CBitmap>	sDecodePNGData(const CData& data);
static	TVResult<CData>		sEncodePNGData(const CBitmap& bitmap);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CImage::Internals

class CImage::Internals : public TReferenceCountableAutoDelete<Internals> {
	public:
		Internals(const CData& data, const OV<CImage::Type>& type, const OV<S2DSizeS32>& size) :
			TReferenceCountableAutoDelete(),
					mData(data), mType(type), mSize(size)
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
	mInternals = new Internals(data, type, size);
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
TVResult<CBitmap> CImage::getBitmap() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup/Validate image type
	if (!mInternals->mType.hasValue())
		// Determine from data
		mInternals->mType = getTypeFromData(mInternals->mData);
	if (!mInternals->mType.hasValue())
		// Unknown type
		return TVResult<CBitmap>(sErrorUnknownType);

	// Check image type
	switch (*mInternals->mType) {
		case kTypeJPEG:
			// JPEG
			return sDecodeJPEGData(mInternals->mData);

		case kTypePNG:
			// PNG
			return sDecodePNGData(mInternals->mData);

		case kTypeNV12: {
			// NV12
			if (!mInternals->mSize.hasValue())
				// Missing size
				return TVResult<CBitmap>(sErrorMissingSize);
			else if (((*mInternals->mSize).mWidth < 2) || (((*mInternals->mSize).mWidth & 1) != 0) ||
					((*mInternals->mSize).mHeight < 2) || (((*mInternals->mSize).mHeight & 1) != 0))
				// Width and height must be 2 or greater and even
				return TVResult<CBitmap>(sErrorUnableToDecode);

			// Setup
			CBitmap	bitmap(*mInternals->mSize, CBitmap::kFormatRGB888);

			// Decode
			sDecodeNV12Data(mInternals->mData, *mInternals->mSize, bitmap,
					S2DRectS32(0, 0, (*mInternals->mSize).mWidth, (*mInternals->mSize).mHeight));

			return TVResult<CBitmap>(bitmap);
		}

#if defined(TARGET_OS_WINDOWS)
		default:
			// Just to make compiler happy.  Will never get here.
			return TVResult<CBitmap>(CBitmap());
#endif
	}
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CImage::decodeInto(CBitmap& bitmap, const S2DRectS32& rect) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup/Validate image type
	if (!mInternals->mType.hasValue())
		// Determine from data
		mInternals->mType = getTypeFromData(mInternals->mData);
	if (!mInternals->mType.hasValue())
		// Unknown type
		return OV<SError>(sErrorUnknownType);

	// Check image type
	switch (*mInternals->mType) {
		case kTypeJPEG:
			// JPEG
			AssertFailUnimplemented();

			return OV<SError>(sErrorUnableToDecode);

		case kTypePNG:
			// PNG
			AssertFailUnimplemented();

			return OV<SError>(sErrorUnableToDecode);

		case kTypeNV12:
			// NV12
			AssertFailIf(!mInternals->mSize.hasValue())
			AssertFailIf((rect.getMaxX() > (*mInternals->mSize).mWidth) ||
					(rect.getMaxY() > (*mInternals->mSize).mHeight))

			// Decode
			sDecodeNV12Data(mInternals->mData, *mInternals->mSize, bitmap, rect);

			return OV<SError>();

#if defined(TARGET_OS_WINDOWS)
		default:
			// Just to make compiler happy.  Will never get here.
			return OV<SError>();
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
OV<CString> CImage::getDefaultFilenameExtension(Type type)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check image type
	switch (type) {
		case kTypeJPEG:	return OV<CString>(CString(OSSTR("jpg")));
		case kTypePNG:	return OV<CString>(CString(OSSTR("png")));
		default:		return OV<CString>();
	}
}

//----------------------------------------------------------------------------------------------------------------------
OV<CString> CImage::getMIMEType(Type type)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check image type
	switch (type) {
		case kTypeJPEG:	return OV<CString>(CString(OSSTR("image/jpeg")));
		case kTypePNG:	return OV<CString>(CString(OSSTR("image/png")));
		default:		return OV<CString>();
	}
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> CImage::getData(const CBitmap& bitmap, Type type)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check image type
	switch (type) {
		case kTypeJPEG:
			// JPEG
			return sEncodeJPEGData(bitmap);

		case kTypePNG:
			// PNG
			return sEncodePNGData(bitmap);

		case kTypeNV12: {
			// NV12
			AssertFailUnimplemented();

			return TVResult<CData>(CData::mEmpty);
		}

#if defined(TARGET_OS_WINDOWS)
		default:
			// Just to make compiler happy.  Will never get here.
			return TVResult<CData>(CData::mEmpty);
#endif
	}
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
TVResult<CBitmap> sDecodeJPEGData(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SJPEGErrorInfo			jpegErrorInfo;
	jpegErrorInfo.error_exit = sJPEGErrorExit;

	// Specify data source
	jpeg_source_mgr	jpegSourceInfo = {0};
	jpegSourceInfo.next_input_byte = nil;
	jpegSourceInfo.bytes_in_buffer = 0;
	jpegSourceInfo.init_source = sJPEGSourceInit;
	jpegSourceInfo.fill_input_buffer = sJPEGFillInputBuffer;
	jpegSourceInfo.skip_input_data = sJPEGSkipInputData;
	jpegSourceInfo.resync_to_restart = jpeg_resync_to_restart;
	jpegSourceInfo.term_source = sJPEGSourceTerminate;

	jpeg_decompress_struct	jpegDecompressInfo;
	jpeg_create_decompress(&jpegDecompressInfo);
	jpegDecompressInfo.err = jpeg_std_error(&jpegErrorInfo);
	jpegDecompressInfo.client_data = (void*) &data;
	jpegDecompressInfo.src = &jpegSourceInfo;

	// Establish setjmp return context
	if (setjmp(jpegErrorInfo.mSetjmpBuffer)) {
		// If we get here, the JPEG code has signaled an error.
		SError	error(sErrorDomain, -1,
						CString(jpegErrorInfo.mErrorMessage, sizeof(jpegErrorInfo.mErrorMessage),
								CString::kEncodingASCII));
		jpeg_destroy_decompress(&jpegDecompressInfo);

		return TVResult<CBitmap>(error);
	}

	// Read parameters
	jpeg_read_header(&jpegDecompressInfo, TRUE);

	// Start the decompressor
	jpeg_start_decompress(&jpegDecompressInfo);

	// Setup bitmap
	CBitmap	bitmap(S2DSizeS32(jpegDecompressInfo.output_width, jpegDecompressInfo.output_height),
					CBitmap::kFormatRGB888);

	// Decompress the image
	JSAMPLE*	bytePtr = (JSAMPLE*) bitmap.getPixelData().getMutableBytePtr();
	UInt16		bytesPerRow = bitmap.getBytesPerRow();
	while (jpegDecompressInfo.output_scanline < jpegDecompressInfo.output_height) {
		// Do the work
		JSAMPLE*	samplePtr = bytePtr + jpegDecompressInfo.output_scanline * bytesPerRow;
		jpeg_read_scanlines(&jpegDecompressInfo, &samplePtr, 1);
	}

	// Finish decompression
	jpeg_finish_decompress(&jpegDecompressInfo);

	// Cleanup
	jpeg_destroy_decompress(&jpegDecompressInfo);

	return TVResult<CBitmap>(bitmap);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> sEncodeJPEGData(const CBitmap& bitmap)
//----------------------------------------------------------------------------------------------------------------------
{
	// Specify data destination
	CData	data(bitmap.getPixelData().getByteCount());

	jpeg_destination_mgr	jpegDestinationManager = {0};
	jpegDestinationManager.next_output_byte = (JOCTET*) data.getMutableBytePtr();
	jpegDestinationManager.free_in_buffer = data.getByteCount();
	jpegDestinationManager.init_destination = sJPEGDestinationInit;
	jpegDestinationManager.empty_output_buffer = sJPEGDestinationEmptyBuffer;
	jpegDestinationManager.term_destination = sJPEGDestinationTerminate;

	// Setup JPEG compression object
	jpeg_error_mgr jerr;

	jpeg_compress_struct	jpegCompressInfo;
	jpeg_create_compress(&jpegCompressInfo);
	jpegCompressInfo.err = jpeg_std_error(&jerr);
	jpegCompressInfo.client_data = nil;
	jpegCompressInfo.dest = &jpegDestinationManager;

	// Set parameters for compression
	jpegCompressInfo.image_width = bitmap.getSize().mWidth;
	jpegCompressInfo.image_height = bitmap.getSize().mHeight;
	switch (bitmap.getFormat()) {
		case CBitmap::kFormatRGB888:
			// RGB 8888
			jpegCompressInfo.input_components = 3;
			jpegCompressInfo.in_color_space = JCS_RGB;
			break;

		case CBitmap::kFormatRGBA8888:
			// RGBA 8888
			jpegCompressInfo.input_components = 4;
			jpegCompressInfo.in_color_space = JCS_EXT_RGBA;
			break;

		case CBitmap::kFormatARGB8888:
			// ARGB 8888
			jpegCompressInfo.input_components = 4;
			jpegCompressInfo.in_color_space = JCS_EXT_ARGB;
			break;

		default:
			// Can't encode
			return TVResult<CData>(sErrorUnableToEncode);
	}
	jpeg_set_defaults(&jpegCompressInfo);
//	jpeg_set_quality(&jpegCompressInfo, 75, TRUE /* limit to baseline-JPEG values */);

	// Start compressor
	jpeg_start_compress(&jpegCompressInfo, TRUE);

	// Encode all scanlines
	int			row_stride = bitmap.getBytesPerRow();
	JSAMPLE*	image_buffer = (JSAMPLE*) bitmap.getPixelData().getBytePtr();
	while (jpegCompressInfo.next_scanline < jpegCompressInfo.image_height) {
		// Do the compression
		JSAMPROW 	row_pointer[1] = {0};
		row_pointer[0] = &image_buffer[jpegCompressInfo.next_scanline * row_stride];
		jpeg_write_scanlines(&jpegCompressInfo, row_pointer, 1);
	}

	// Finish compression
	jpeg_finish_compress(&jpegCompressInfo);
	data.setByteCount(data.getByteCount() - jpegDestinationManager.free_in_buffer);

	// Cleanup
	jpeg_destroy_compress(&jpegCompressInfo);

	return TVResult<CData>(data);
}

//----------------------------------------------------------------------------------------------------------------------
void sJPEGDestinationInit(j_compress_ptr jpegInfoPtr)
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
boolean sJPEGDestinationEmptyBuffer(j_compress_ptr jpegInfoPtr)
//----------------------------------------------------------------------------------------------------------------------
{
	return true;
}

//----------------------------------------------------------------------------------------------------------------------
void sJPEGDestinationTerminate(j_compress_ptr jpegInfoPtr)
//----------------------------------------------------------------------------------------------------------------------
{
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
		CData*	data = (CData*) jpegInfoPtr->client_data;

		jpegInfoPtr->src->next_input_byte = (const JOCTET*) data->getBytePtr();
		jpegInfoPtr->src->bytes_in_buffer = data->getByteCount();

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
void sDecodeNV12Data(const CData& data, const S2DSizeS32& size, CBitmap& bitmap, const S2DRectS32& rect)
//----------------------------------------------------------------------------------------------------------------------
{
	// Based on implementation by Andre Chen, see https://github.com/andrechen/yuv2rgb

	// Setup
	I<CBitmap::RowWriter>	rowWriter0 = bitmap.getRowWriter(0);
	I<CBitmap::RowWriter>	rowWriter1 = bitmap.getRowWriter(1);

	// Loop vertially
	const UInt8* y0Ptr = (UInt8*) data.getBytePtr();
	const UInt8* y1Ptr = y0Ptr + size.mWidth;
	const UInt8* uvPtr = y0Ptr + size.mWidth * size.mHeight;
	for (SInt32 y = 0; y < size.mHeight; y += 2, y0Ptr += size.mWidth, y1Ptr += size.mWidth) {
		// Setup
		bool	writeRow0;
		if ((y >= rect.getMinY() && (y < rect.getMaxY()))) {
			// Do this y
			rowWriter0 = bitmap.getRowWriter(y - rect.getMinY());
			writeRow0 = true;
		} else
			// Skip this y
			writeRow0 = false;

		bool	writeRow1;
		if (((y + 1) >= rect.getMinY() && ((y + 1) < rect.getMaxY()))) {
			// Do this y
			rowWriter1 = bitmap.getRowWriter((y + 1) - rect.getMinY());
			writeRow1 = true;
		} else
			// Skip this y
			writeRow1 = false;

		// Loop horizontally
		for (SInt32 x = 0; x < size.mWidth; x += 2) {
			// Get source ys
			SInt32	y00 = (*y0Ptr++) - 16;
			y00 = (y00 > 0) ? 298 * y00 : 0;

			SInt32	y01 = (*y0Ptr++) - 16;
			y01 = (y01 > 0) ? 298 * y01 : 0;

			SInt32	y10 = (*y1Ptr++) - 16;
			y10 = (y10 > 0) ? 298 * y10 : 0;

			SInt32	y11 = (*y1Ptr++) - 16;
			y11 = (y11 > 0) ? 298 * y11 : 0;

			// Load U and V
			SInt32	v = (*uvPtr++) - 128;
			SInt32	u = (*uvPtr++) - 128;

			// Calculate RGB adjustments
			SInt32	dR = 128 + 409 * v;
			SInt32	dG = 128 - 100 * u - 208 * v;
			SInt32	dB = 128 + 516 * u;

			// Store RGB
			SInt32	r, g, b;

			if (writeRow0 && (x >= rect.getMinX() && (x < rect.getMaxX()))) {
				// Store this pixel
				r = y00 + dR; g = y00 + dG; b = y00 + dB;
				rowWriter0->write(
						CBitmap::PixelDataRGB888((r > 0) ? (r < 65535 ? (UInt8) (r >> 8) : 0xff) : 0,
								(g > 0) ? (g < 65535 ? (UInt8) (g >> 8) : 0xff) : 0,
								(b > 0) ? (b < 65535 ? (UInt8) (b >> 8) : 0xff) : 0));
			}

			if (writeRow0 && ((x + 1) >= rect.getMinX() && ((x + 1) < rect.getMaxX()))) {
				// Store this pixel
				r = y01 + dR; g = y01 + dG; b = y01 + dB;
				rowWriter0->write(
						CBitmap::PixelDataRGB888((r > 0) ? (r < 65535 ? (UInt8) (r >> 8) : 0xff) : 0,
								(g > 0) ? (g < 65535 ? (UInt8) (g >> 8) : 0xff) : 0,
								(b > 0) ? (b < 65535 ? (UInt8) (b >> 8) : 0xff) : 0));
			}

			if (writeRow1 && (x >= rect.getMinX() && (x < rect.getMaxX()))) {
				// Store this pixel
				r = y10 + dR; g = y10 + dG; b = y10 + dB;
				rowWriter1->write(
						CBitmap::PixelDataRGB888((r > 0) ? (r < 65535 ? (UInt8) (r >> 8) : 0xff) : 0,
								(g > 0) ? (g < 65535 ? (UInt8) (g >> 8) : 0xff) : 0,
								(b > 0) ? (b < 65535 ? (UInt8) (b >> 8) : 0xff) : 0));
			}

			if (writeRow1 && ((x + 1) >= rect.getMinX() && ((x + 1) < rect.getMaxX()))) {
				// Store this pixel
				r = y11 + dR; g = y11 + dG; b = y11 + dB;
				rowWriter1->write(
						CBitmap::PixelDataRGB888((r > 0) ? (r < 65535 ? (UInt8) (r >> 8) : 0xff) : 0,
								(g > 0) ? (g < 65535 ? (UInt8) (g >> 8) : 0xff) : 0,
								(b > 0) ? (b < 65535 ? (UInt8) (b >> 8) : 0xff) : 0));
			}
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CBitmap> sDecodePNGData(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	png_image	pngImage;
	memset(&pngImage, 0, sizeof(png_image));
	pngImage.version = PNG_IMAGE_VERSION;
	if (png_image_begin_read_from_memory(&pngImage, data.getBytePtr(), data.getByteCount()) == 0)
		// Nope!
		return TVResult<CBitmap>(sErrorUnableToDecode);

	// Setup bitmap
	CBitmap::Format	bitmapFormat;
	if (pngImage.format & PNG_FORMAT_FLAG_ALPHA) {
		// Have alpha
		bitmapFormat = CBitmap::kFormatRGBA8888;
		pngImage.format = PNG_FORMAT_RGBA;
	} else {
		// No alpha
		bitmapFormat = CBitmap::kFormatRGB888;
		pngImage.format = PNG_FORMAT_RGB;
	}

	CBitmap	bitmap(S2DSizeS32(pngImage.width, pngImage.height), bitmapFormat);
	if (png_image_finish_read(&pngImage, nil, bitmap.getPixelData().getMutableBytePtr(), bitmap.getBytesPerRow(), nil)
			== 0)
		// Something went wrong
		return TVResult<CBitmap>(sErrorUnableToDecode);

	return TVResult<CBitmap>(bitmap);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> sEncodePNGData(const CBitmap& bitmap)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	png_image	pngImage;
	memset(&pngImage, 0, sizeof(png_image));
	pngImage.version = PNG_IMAGE_VERSION;
	pngImage.width = bitmap.getSize().mWidth;
	pngImage.height = bitmap.getSize().mHeight;

	switch (bitmap.getFormat()) {
		case CBitmap::kFormatRGB888:
			// RGB888
			pngImage.format = PNG_FORMAT_RGB;
			break;

		case CBitmap::kFormatRGBA8888:
			// RGBA8888
			pngImage.format = PNG_FORMAT_RGBA;
			break;

		case CBitmap::kFormatARGB8888:
			// ARGB8888
			pngImage.format = PNG_FORMAT_ARGB;
			break;

		default:
			// The rest - unsupported
			return TVResult<CData>(sErrorUnableToEncode);
	}

	CData	data(bitmap.getPixelData().getByteCount());

	// Write
	png_alloc_size_t	byteCount = data.getByteCount();
	png_image_write_to_memory(&pngImage, data.getMutableBytePtr(), &byteCount, 0, bitmap.getPixelData().getBytePtr(),
			bitmap.getBytesPerRow(), nil);

	// Finish up
	data.setByteCount(byteCount);

	return TVResult<CData>(data);
}
