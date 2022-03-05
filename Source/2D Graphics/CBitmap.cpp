//----------------------------------------------------------------------------------------------------------------------
//	CBitmap.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CBitmap.h"

#include "CppToolboxAssert.h"
#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local proc declarations

static	void	sConvertRGB565ToRGB888(const CBitmapInternals& sourceBitmapInternals,
						CBitmapInternals& destinationBitmapInternals);
static	void	sConvertRGB565ToRGBA8888(const CBitmapInternals& sourceBitmapInternals,
						CBitmapInternals& destinationBitmapInternals);
static	void	sConvertRGB565ToARGB8888(const CBitmapInternals& sourceBitmapInternals,
						CBitmapInternals& destinationBitmapInternals);

static	void	sConvertRGBA4444ToRGBA8888(const CBitmapInternals& sourceBitmapInternals,
						CBitmapInternals& destinationBitmapInternals);
static	void	sConvertRGBA4444ToARGB8888(const CBitmapInternals& sourceBitmapInternals,
						CBitmapInternals& destinationBitmapInternals);

static	void	sConvertRGBA5551ToRGBA8888(const CBitmapInternals& sourceBitmapInternals,
						CBitmapInternals& destinationBitmapInternals);
static	void	sConvertRGBA5551ToARGB8888(const CBitmapInternals& sourceBitmapInternals,
						CBitmapInternals& destinationBitmapInternals);

static	void	sConvertRGB888ToRGB565(const CBitmapInternals& sourceBitmapInternals,
						CBitmapInternals& destinationBitmapInternals);
static	void	sConvertRGB888ToRGBA4444(const CBitmapInternals& sourceBitmapInternals,
						CBitmapInternals& destinationBitmapInternals);
static	void	sConvertRGB888ToRGBA5551(const CBitmapInternals& sourceBitmapInternals,
						CBitmapInternals& destinationBitmapInternals);
static	void	sConvertRGB888ToRGBA8888(const CBitmapInternals& sourceBitmapInternals,
						CBitmapInternals& destinationBitmapInternals);
static	void	sConvertRGB888ToARGB8888(const CBitmapInternals& sourceBitmapInternals,
						CBitmapInternals& destinationBitmapInternals);

static	void	sConvertRGBA8888ToRGB565(const CBitmapInternals& sourceBitmapInternals,
						CBitmapInternals& destinationBitmapInternals);
static	void	sConvertRGBA8888ToRGBA4444(const CBitmapInternals& sourceBitmapInternals,
						CBitmapInternals& destinationBitmapInternals);
static	void	sConvertRGBA8888ToRGBA5551(const CBitmapInternals& sourceBitmapInternals,
						CBitmapInternals& destinationBitmapInternals);
static	void	sConvertRGBA8888ToARGB8888(const CBitmapInternals& sourceBitmapInternals,
						CBitmapInternals& destinationBitmapInternals);

static	void	sConvertARGB8888ToRGBA4444(const CBitmapInternals& sourceBitmapInternals,
						CBitmapInternals& destinationBitmapInternals);
static	void	sConvertARGB8888ToRGBA5551(const CBitmapInternals& sourceBitmapInternals,
						CBitmapInternals& destinationBitmapInternals);
static	void	sConvertARGB8888ToRGBA8888(const CBitmapInternals& sourceBitmapInternals,
						CBitmapInternals& destinationBitmapInternals);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBitmapInternals

class CBitmapInternals : public TCopyOnWriteReferenceCountable<CBitmapInternals>{
	public:
		CBitmapInternals(const S2DSizeS32& size, CBitmap::Format format, const CData& pixelData = CData::mEmpty,
				const OV<UInt16>& bytesPerRow = OV<UInt16>()) :
			TCopyOnWriteReferenceCountable(),
					mSize(size), mFormat(format)
			{
				// Finish setup
				switch (mFormat) {
					case CBitmap::kFormatRGBA4444:
					case CBitmap::kFormatRGBA5551:
					case CBitmap::kFormatRGB565:
						mBytesPerPixel = 2;
						break;

					case CBitmap::kFormatRGB888:
						mBytesPerPixel = 3;
						break;

					case CBitmap::kFormatRGBA8888:
					case CBitmap::kFormatARGB8888:
						mBytesPerPixel = 4;
						break;
				}

				if (bytesPerRow.hasValue())
					// Have value
					mBytesPerRow = *bytesPerRow;
				else {
					// Calculate and round up to a 16 byte boundary
					mBytesPerRow = (UInt16) (mSize.mWidth * mBytesPerPixel);
					if ((mBytesPerRow % 0x10) != 0)
						mBytesPerRow += 0x10 - (mBytesPerRow % 0x0F);
				}
				mPixelData = !pixelData.isEmpty() ? pixelData : CData((CData::ByteCount) mBytesPerRow * mSize.mHeight);
			}
		CBitmapInternals(const CBitmapInternals& other) :
			TCopyOnWriteReferenceCountable(),
					mFormat(other.mFormat), mSize(other.mSize), mPixelData(other.mPixelData),
					mBytesPerPixel(other.mBytesPerPixel), mBytesPerRow(other.mBytesPerRow)
			{}

		CBitmap::Format	mFormat;
		S2DSizeS32		mSize;
		CData			mPixelData;
		UInt16			mBytesPerPixel;
		UInt16			mBytesPerRow;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBitmap

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CBitmap::CBitmap(const S2DSizeS32& size, Format format, const OV<UInt16>& bytesPerRow)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertFailIf(size.mWidth < 1);
	AssertFailIf(size.mHeight < 1);

	// Setup
	mInternals =
			new CBitmapInternals(S2DSizeS32(std::max(1, size.mWidth), std::max(1, size.mHeight)), format,
					CData::mEmpty, bytesPerRow);
}

//----------------------------------------------------------------------------------------------------------------------
CBitmap::CBitmap(const S2DSizeS32& size, Format format, const CData& pixelData, UInt16 bytesPerRow)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertFailIf(size.mWidth < 1);
	AssertFailIf(size.mHeight < 1);

	// Setup
	mInternals =
			new CBitmapInternals(S2DSizeS32(std::max(1, size.mWidth), std::max(1, size.mHeight)), format, pixelData,
					OV<UInt16>(bytesPerRow));
}

//----------------------------------------------------------------------------------------------------------------------
CBitmap::CBitmap(const CBitmap& other, Format format)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CBitmapInternals(other.mInternals->mSize, format);

	// Convert
	switch (other.mInternals->mFormat) {
		case kFormatRGB565:
			// RGB565 =>
			switch (format) {
				case kFormatRGB888:
					// => RGB888
					sConvertRGB565ToRGB888(*other.mInternals, *mInternals);
					break;

				case kFormatRGBA8888:
					// => RGBA8888
					sConvertRGB565ToRGBA8888(*other.mInternals, *mInternals);
					break;

				case kFormatARGB8888:
					// => ARGB8888
					sConvertRGB565ToARGB8888(*other.mInternals, *mInternals);
					break;

				default:
					// Would never do
					AssertFailUnimplemented();
			}
			break;

		case kFormatRGBA4444:
			// RGBA4444 =>
			switch (format) {
				case kFormatRGBA8888:
					// => RGBA8888
					sConvertRGBA4444ToRGBA8888(*other.mInternals, *mInternals);
					break;

				case kFormatARGB8888:
					// => ARGB8888
					sConvertRGBA4444ToARGB8888(*other.mInternals, *mInternals);
					break;

				default:
					// Would never do
					AssertFailUnimplemented();
			}
			break;

		case kFormatRGBA5551:
			// RGBA5551 =>
			switch (format) {
				case kFormatRGBA8888:
					// => RGBA8888
					sConvertRGBA5551ToRGBA8888(*other.mInternals, *mInternals);
					break;

				case kFormatARGB8888:
					// => ARGB8888
					sConvertRGBA5551ToARGB8888(*other.mInternals, *mInternals);
					break;

				default:
					// Would never do
					AssertFailUnimplemented();
			}
			break;

		case kFormatRGB888:
			// RGB888 =>
			switch (format) {
				case kFormatRGB565:
					// => RGB565
					sConvertRGB888ToRGB565(*other.mInternals, *mInternals);
					break;

				case kFormatRGBA4444:
					// => RGBA4444
					sConvertRGB888ToRGBA4444(*other.mInternals, *mInternals);
					break;

				case kFormatRGBA5551:
					// => RGBA5551
					sConvertRGB888ToRGBA5551(*other.mInternals, *mInternals);
					break;

				case kFormatRGBA8888:
					// => RGBA8888
					sConvertRGB888ToRGBA8888(*other.mInternals, *mInternals);
					break;

				case kFormatARGB8888:
					// => ARGB8888
					sConvertRGB888ToARGB8888(*other.mInternals, *mInternals);
					break;

				default:
					// Would never do
					AssertFailUnimplemented();
			}
			break;

		case kFormatRGBA8888:
			// RGBA8888 =>
			switch (format) {
				case kFormatRGB565:
					// => RGB565
					sConvertRGBA8888ToRGB565(*other.mInternals, *mInternals);
					break;

				case kFormatRGBA4444:
					// => RGBA4444
					sConvertRGBA8888ToRGBA4444(*other.mInternals, *mInternals);
					break;

				case kFormatRGBA5551:
					// => RGBA5551
					sConvertRGBA8888ToRGBA5551(*other.mInternals, *mInternals);
					break;

				case kFormatARGB8888:
					// => ARGB8888
					sConvertRGBA8888ToARGB8888(*other.mInternals, *mInternals);
					break;

				default:
					// Would never do
					AssertFailUnimplemented();
			}
			break;

		case kFormatARGB8888:
			// ARGB8888 =>
			switch (format) {
				case kFormatRGBA4444:
					// => RGBA4444
					sConvertARGB8888ToRGBA4444(*other.mInternals, *mInternals);
					break;

				case kFormatRGBA5551:
					// => RGBA5551
					sConvertARGB8888ToRGBA5551(*other.mInternals, *mInternals);
					break;

				case kFormatRGBA8888:
					// => RGBA8888
					sConvertARGB8888ToRGBA8888(*other.mInternals, *mInternals);
					break;

				default:
					// Would never do
					AssertFailUnimplemented();
			}
			break;
	}
}

//----------------------------------------------------------------------------------------------------------------------
CBitmap::CBitmap(const CBitmap& other, RotationOperation rotationOperation)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	S2DSizeS32	newSize;
	switch (rotationOperation & 0x03) {
		case kRotationOperationRotateNone:
		case kRotationOperationRotate180:
			// Flipping
			newSize = other.mInternals->mSize;
			break;

		case kRotationOperationRotate90:
		case kRotationOperationRotate270:
			// Rotating
			newSize = S2DSizeS32(other.mInternals->mSize.mHeight, other.mInternals->mSize.mWidth);
			break;
	}

	mInternals = new CBitmapInternals(newSize, other.mInternals->mFormat);

	// Rotate
	// bytePtr = A * y + B * x + C
	SInt32		A = 0;
	SInt32		B = 0;
	SInt32		C = 0;
	SInt32		bytesPerPixel = (SInt32) other.mInternals->mBytesPerPixel;
	SInt32		bytesPerRow = other.mInternals->mBytesPerRow;
	S2DSizeS32	size = other.mInternals->mSize;
	switch ((UInt32) rotationOperation) {
		case kRotationOperationRotateNone:
			// bytePtr = y * bytesPerRow + x * bytesPerPixel
			// bytePtr = bytesPerRow * y + bytesPerPixel * x + 0
			A = bytesPerRow;
			B = bytesPerPixel;
			C = 0;
			break;

		case kRotationOperationRotateNone | kRotationOperationFlipLR:
			// bytePtr = y * bytesPerRow + ((width - 1) - x) * bytesPerPixel
			// bytePtr = y * bytesPerRow + (width - 1) * bytesPerPixel - x * bytesPerPixel
			// bytePtr = bytesPerRow * y - bytesPerPixel * x + (width - 1) * bytesPerPixel
			A = bytesPerRow;
			B = -bytesPerPixel;
			C = (size.mWidth - 1) * bytesPerPixel;
			break;

		case kRotationOperationRotate90:
			// bytePtr = x * bytesPerRow + ((height - 1) - y) * bytesPerPixel
			// bytePtr = x * bytesPerRow + (height - 1) * bytesPerPixel - y * bytesPerPixel
			// bytePtr = -bytesPerPixel * y + bytesPerRow * x + (height - 1) * bytesPerPixel
			A = -bytesPerPixel;
			B = bytesPerRow;
			C = (size.mHeight - 1) * bytesPerPixel;
			break;

		case kRotationOperationRotate90 | kRotationOperationFlipLR:
			// bytePtr = x * bytesPerRow + y * bytesPerPixel
			// bytePtr = bytesPerPixel * y + bytesPerRow * x + 0
			A = bytesPerPixel;
			B = bytesPerRow;
			C = 0;
			break;

		case kRotationOperationRotate180:
			// bytePtr = ((height - 1) - y) * bytesPerRow + ((width - 1) - x) * bytesPerPixel
			// bytePtr = (height - 1) * bytesPerRow - y * bytesPerRow + (width - 1) * bytesPerPixel - x * bytesPerPixel
			// bytePtr = -bytesPerRow * y -bytesPerPixel * x + (height - 1) * bytesPerRow + (width - 1) * bytesPerPixel
			A = -bytesPerRow;
			B = -bytesPerPixel;
			C = (size.mHeight - 1) * bytesPerRow + (size.mWidth - 1) * bytesPerPixel;
			break;

		case kRotationOperationRotate180 | kRotationOperationFlipLR:
			// bytePtr = ((height - 1) - y) * bytesPerRow + x * bytesPerPixel
			// bytePtr = (height - 1) * bytesPerRow - y * bytesPerRow + x * bytesPerPixel
			// bytePtr = -bytesPerRow * y + bytesPerPixel * x + (height - 1) * bytesPerRow;
			A = -bytesPerRow;
			B = bytesPerPixel;
			C = (size.mHeight - 1) * bytesPerRow;
			break;

		case kRotationOperationRotate270:
			// bytePtr = ((width - 1) - x) * bytesPerRow + y * bytesPerPixel
			// bytePtr = (width - 1) * bytesPerRow - x * bytesPerRow + y * bytesPerPixel
			// bytePtr = bytesPerPixel * y - bytesPerRow * x + (width - 1) * bytesPerRow;
			A = bytesPerPixel;
			B = -bytesPerRow;
			C = (size.mWidth - 1) * bytesPerRow;
			break;

		case kRotationOperationRotate270 | kRotationOperationFlipLR:
			// bytePtr = ((width - 1) - x) * bytesPerRow + ((height - 1) - y) * bytesPerPixel
			// bytePtr = (width - 1) * bytesPerRow - x * bytesPerRow + (height - 1) *  bytesPerPixel - y * bytesPerPixel
			// bytePtr = -bytesPerPixel * y - bytesPerRow * x + (width - 1) * bytesPerRow + (height - 1) * bytesPerPixel
			A = -bytesPerPixel;
			B = -bytesPerRow;
			C = (size.mWidth - 1) * bytesPerRow + (size.mHeight - 1) * bytesPerPixel;
			break;
	}

	// Loop on vertical
	const	UInt8*	sourcePixelData = (const UInt8*) other.mInternals->mPixelData.getBytePtr();
			UInt8*	destinationPixelData = (UInt8*) mInternals->mPixelData.getMutableBytePtr();
	for (SInt32 y = 0; y < size.mHeight; y++) {
		// Get source ptr
		const	UInt8*	srcPtr = sourcePixelData + other.mInternals->mBytesPerRow * y;

		// Loop on horizontal
		for (SInt32 x = 0; x < size.mWidth; x++, srcPtr += bytesPerPixel) {
			// Calculate destination offset
			SInt32	destinationOffset = A * y + B * x + C;

			// Set pixel
			::memcpy(destinationPixelData + destinationOffset, srcPtr, bytesPerPixel);
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
CBitmap::CBitmap(const CBitmap& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CBitmap::~CBitmap()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const S2DSizeS32& CBitmap::getSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mSize;
}

//----------------------------------------------------------------------------------------------------------------------
CData& CBitmap::getPixelData() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mPixelData;
}

//----------------------------------------------------------------------------------------------------------------------
CBitmap::Format CBitmap::getFormat() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFormat;
}

//----------------------------------------------------------------------------------------------------------------------
UInt16 CBitmap::getBytesPerRow() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mBytesPerRow;
}

//----------------------------------------------------------------------------------------------------------------------
UInt16 CBitmap::getBytesPerPixel() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mBytesPerPixel;
}

//----------------------------------------------------------------------------------------------------------------------
void CBitmap::setPixel(const S2DPointS32& point, const CColor& color)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertFailIf((point.mX < 0) || (point.mX >= mInternals->mSize.mWidth));
	if ((point.mX < 0) || (point.mX >= mInternals->mSize.mWidth))
		return;

	AssertFailIf((point.mY < 0) || (point.mY >= mInternals->mSize.mHeight));
	if ((point.mY < 0) || (point.mY >= mInternals->mSize.mHeight))
		return;

	// Prepare for write
	mInternals = mInternals->prepareForWrite();

	// Update pixel data
	void*	pixelDataPtr =
					(UInt8*) mInternals->mPixelData.getMutableBytePtr() + point.mY * mInternals->mBytesPerRow +
							point.mX * mInternals->mBytesPerPixel;
	switch (mInternals->mFormat) {
		case kFormatRGBA4444: {
			// RGBA4444
			SPixelDataRGBA4444*	pixelData = (SPixelDataRGBA4444*) pixelDataPtr;
			pixelData->mComponents.mR = (UInt16) (color.getRed() * 15.0);
			pixelData->mComponents.mG = (UInt16) (color.getGreen() * 15.0);
			pixelData->mComponents.mB = (UInt16) (color.getBlue() * 15.0);
			pixelData->mComponents.mA = (UInt16) (color.getAlpha() * 15.0);
			break;
		}

		case kFormatRGBA5551: {
			// RGBA5551
			SPixelDataRGBA5551*	pixelData = (SPixelDataRGBA5551*) pixelDataPtr;
			pixelData->mComponents.mR = (UInt16) (color.getRed() * 31.0);
			pixelData->mComponents.mG = (UInt16) (color.getGreen() * 31.0);
			pixelData->mComponents.mB = (UInt16) (color.getBlue() * 31.0);
			pixelData->mComponents.mA = (UInt16) (color.getAlpha() * 1.0);
			break;
		}

		case kFormatRGB565: {
			// RGB565
			SPixelDataRGB565*	pixelData = (SPixelDataRGB565*) pixelDataPtr;
			pixelData->mComponents.mR = (UInt16) (color.getRed() * 31.0);
			pixelData->mComponents.mG = (UInt16) (color.getGreen() * 63.0);
			pixelData->mComponents.mB = (UInt16) (color.getBlue() * 31.0);
			break;
		}

		case kFormatRGB888: {
			// RGB888
			SPixelDataRGB888*	pixelData = (SPixelDataRGB888*) pixelDataPtr;
			pixelData->mR = (UInt8) (color.getRed() * 255.0);
			pixelData->mG = (UInt8) (color.getGreen() * 255.0);
			pixelData->mB = (UInt8) (color.getBlue() * 255.0);
			break;
		}

		case kFormatRGBA8888: {
			// RGBA8888
			SPixelDataRGBA8888*	pixelData = (SPixelDataRGBA8888*) pixelDataPtr;
			pixelData->mComponents.mR = (UInt8) (color.getRed() * 255.0);
			pixelData->mComponents.mG = (UInt8) (color.getGreen() * 255.0);
			pixelData->mComponents.mB = (UInt8) (color.getBlue() * 255.0);
			pixelData->mComponents.mA = (UInt8) (color.getAlpha() * 255.0);
			break;
		}

		case kFormatARGB8888: {
			// ARGB8888
			SPixelDataARGB8888*	pixelData = (SPixelDataARGB8888*) pixelDataPtr;
			pixelData->mComponents.mA = (UInt8) (color.getAlpha() * 255.0);
			pixelData->mComponents.mR = (UInt8) (color.getRed() * 255.0);
			pixelData->mComponents.mG = (UInt8) (color.getGreen() * 255.0);
			pixelData->mComponents.mB = (UInt8) (color.getBlue() * 255.0);
			break;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
void sConvertRGB565ToRGB888(const CBitmapInternals& sourceBitmapInternals, CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertRGB565ToRGBA8888(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	UInt8*	sourcePixelData = (const UInt8*) sourceBitmapInternals.mPixelData.getBytePtr();
			UInt32	sourceBytesPerRow = sourceBitmapInternals.mBytesPerRow;
			UInt8*	destinationPixelData = (UInt8*) destinationBitmapInternals.mPixelData.getMutableBytePtr();
			UInt32	destinationBytesPerRow = destinationBitmapInternals.mBytesPerRow;

	// Loop on vertical
	for (SInt32 h = 0; h < sourceBitmapInternals.mSize.mHeight; h++) {
		// Setup
		const	UInt16*	srcPtr = (const UInt16*) (sourcePixelData + h * sourceBytesPerRow);
				UInt32*	dstPtr = (UInt32*) (destinationPixelData + h * destinationBytesPerRow);

		// Loop on horizontal
		for (SInt32 w = 0; w < sourceBitmapInternals.mSize.mWidth; w++, srcPtr++, dstPtr++) {
			// Convert color
#if TARGET_RT_LITTLE_ENDIAN
			UInt32	red = (((*srcPtr >> 11) & 0x1F) * 255 + 15) / 31;
			UInt32	green = (((*srcPtr >> 5) & 0x3F) * 255 + 31) / 63;
			UInt32	blue = (((*srcPtr >> 0) & 0x1F) * 255 + 15) / 31;
			*dstPtr = (red << 0) | (green << 8) | (blue << 16) | 0xFF000000;
#else
#if defined(__clang__)
	#warning TODO - convertRGB565ToRGBA8888 for Big Endian
#endif
			AssertFailUnimplemented();
#endif
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertRGB565ToARGB8888(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertRGBA4444ToRGBA8888(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	UInt8*	sourcePixelData = (const UInt8*) sourceBitmapInternals.mPixelData.getBytePtr();
			UInt32	sourceBytesPerRow = sourceBitmapInternals.mBytesPerRow;
			UInt8*	destinationPixelData = (UInt8*) destinationBitmapInternals.mPixelData.getMutableBytePtr();
			UInt32	destinationBytesPerRow = destinationBitmapInternals.mBytesPerRow;

	// Loop on vertical
	for (SInt32 h = 0; h < sourceBitmapInternals.mSize.mHeight; h++) {
		// Setup
		const	UInt16*	srcPtr = (const UInt16*) (sourcePixelData + h * sourceBytesPerRow);
				UInt32*	dstPtr = (UInt32*) (destinationPixelData + h * destinationBytesPerRow);

		// Loop on horizontal
		for (SInt32 w = 0; w < sourceBitmapInternals.mSize.mWidth; w++, srcPtr++, dstPtr++) {
			// Convert color
#if TARGET_RT_LITTLE_ENDIAN
			UInt32	red = (((*srcPtr >> 12) & 0x0F) * 255 + 7) / 15;
			UInt32	green = (((*srcPtr >> 8) & 0x0F) * 255 + 7) / 15;
			UInt32	blue = (((*srcPtr >> 4) & 0x0F) * 255 + 7) / 15;
			UInt32	alpha = (((*srcPtr >> 0) & 0x0F) * 255 + 7) / 15;
			*dstPtr = (red << 0) | (green << 8) | (blue << 16) | (alpha << 24);
#else
#if defined(__clang__)
	#warning TODO - convertRGBA4444ToRGBA8888 for Big Endian
#endif
			AssertFailUnimplemented();
#endif
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertRGBA4444ToARGB8888(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertRGBA5551ToRGBA8888(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	UInt8*	sourcePixelData = (const UInt8*) sourceBitmapInternals.mPixelData.getBytePtr();
			UInt32	sourceBytesPerRow = sourceBitmapInternals.mBytesPerRow;
			UInt8*	destinationPixelData = (UInt8*) destinationBitmapInternals.mPixelData.getMutableBytePtr();
			UInt32	destinationBytesPerRow = destinationBitmapInternals.mBytesPerRow;

	// Loop on vertical
	for (SInt32 h = 0; h < sourceBitmapInternals.mSize.mHeight; h++) {
		// Setup
		const	UInt16*	srcPtr = (const UInt16*) (sourcePixelData + h * sourceBytesPerRow);
				UInt32*	dstPtr = (UInt32*) (destinationPixelData + h * destinationBytesPerRow);

		// Loop on horizontal
		for (SInt32 w = 0; w < sourceBitmapInternals.mSize.mWidth; w++, srcPtr++, dstPtr++) {
			// Convert color
#if TARGET_RT_LITTLE_ENDIAN
			UInt32	red = (((*srcPtr >> 11) & 0x1F) * 255 + 15) / 31;
			UInt32	green = (((*srcPtr >> 6) & 0x1F) * 255 + 15) / 31;
			UInt32	blue = (((*srcPtr >> 1) & 0x1F) * 255 + 15) / 31;
			UInt32	alpha = (((*srcPtr >> 0) & 0x01) * 255 + 0) / 1;
			*dstPtr = (red << 0) | (green << 8) | (blue << 16) | (alpha << 24);
#else
#if defined(__clang__)
	#warning TODO - convertRGBA5551ToRGBA8888 for Big Endian
#endif
			AssertFailUnimplemented();
#endif
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertRGBA5551ToARGB8888(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertRGB888ToRGB565(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertRGB888ToRGBA4444(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertRGB888ToRGBA5551(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertRGB888ToRGBA8888(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	UInt8*	sourcePixelData = (const UInt8*) sourceBitmapInternals.mPixelData.getBytePtr();
			UInt32	sourceBytesPerRow = sourceBitmapInternals.mBytesPerRow;
			UInt8*	destinationPixelData = (UInt8*) destinationBitmapInternals.mPixelData.getMutableBytePtr();
			UInt32	destinationBytesPerRow = destinationBitmapInternals.mBytesPerRow;

	// Loop on vertical
	for (SInt32 h = 0; h < sourceBitmapInternals.mSize.mHeight; h++) {
		// Setup
		const	UInt8*	srcPtr = (const UInt8*) (sourcePixelData + h * sourceBytesPerRow);
				UInt8*	dstPtr = destinationPixelData + h * destinationBytesPerRow;
		for (UInt32 w = 0; w < (UInt32) sourceBitmapInternals.mSize.mWidth; w++) {
			// Convert color
#if TARGET_RT_LITTLE_ENDIAN
			*(dstPtr++) = (*srcPtr++);
			*(dstPtr++) = (*srcPtr++);
			*(dstPtr++) = (*srcPtr++);
			*(dstPtr++) = 0xFF;
#else
#if defined(__clang__)
	#warning TODO - convertRGB888ToRGBA8888 for Big Endian
#endif
			AssertFailUnimplemented();
#endif
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertRGB888ToARGB8888(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	UInt8*	sourcePixelData = (const UInt8*) sourceBitmapInternals.mPixelData.getBytePtr();
			UInt32	sourceBytesPerRow = sourceBitmapInternals.mBytesPerRow;
			UInt8*	destinationPixelData = (UInt8*) destinationBitmapInternals.mPixelData.getMutableBytePtr();
			UInt32	destinationBytesPerRow = destinationBitmapInternals.mBytesPerRow;

	// Loop on vertical
	for (SInt32 h = 0; h < sourceBitmapInternals.mSize.mHeight; h++) {
		// Setup
		const	UInt8*	srcPtr = (const UInt8*) (sourcePixelData + h * sourceBytesPerRow);
				UInt8*	dstPtr = destinationPixelData + h * destinationBytesPerRow;
		for (UInt32 w = 0; w < (UInt32) sourceBitmapInternals.mSize.mWidth; w++) {
			// Convert color
#if TARGET_RT_LITTLE_ENDIAN
			*(dstPtr++) = 0xFF;
			*(dstPtr++) = (*srcPtr++);
			*(dstPtr++) = (*srcPtr++);
			*(dstPtr++) = (*srcPtr++);
#else
#if defined(__clang__)
	#warning TODO - convertRGB888ToRGBA8888 for Big Endian
#endif
			AssertFailUnimplemented();
#endif
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertRGBA8888ToRGB565(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	UInt8*	sourcePixelData = (const UInt8*) sourceBitmapInternals.mPixelData.getBytePtr();
			UInt32	sourceBytesPerRow = sourceBitmapInternals.mBytesPerRow;
			UInt8*	destinationPixelData = (UInt8*) destinationBitmapInternals.mPixelData.getMutableBytePtr();
			UInt32	destinationBytesPerRow = destinationBitmapInternals.mBytesPerRow;

	// Loop on vertical
	for (SInt32 h = 0; h < sourceBitmapInternals.mSize.mHeight; h++) {
		// Setup
		const	UInt32*	srcPtr = (const UInt32*) (sourcePixelData + h * sourceBytesPerRow);
				UInt16*	dstPtr = (UInt16*) (destinationPixelData + h * destinationBytesPerRow);

		// Loop on horizontal
		for (SInt32 w = 0; w < sourceBitmapInternals.mSize.mWidth; w++, srcPtr++, dstPtr++) {
			// Convert color
#if TARGET_RT_LITTLE_ENDIAN
			UInt32	red = (((*srcPtr >> 0) & 0xFF) * 31 + 127) / 255;
			UInt32	green = (((*srcPtr >> 8) & 0xFF) * 63 + 127) / 255;
			UInt32	blue = (((*srcPtr >> 16) & 0xFF) * 31 + 127) / 255;
			*dstPtr = (UInt16) ((red << 11) | (green << 5) | blue);
#else
#if defined(__clang__)
	#warning TODO - convertRGBA8888ToRGB565 for Big Endian
#endif
			AssertFailUnimplemented();
#endif
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertRGBA8888ToRGBA4444(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	UInt8*	sourcePixelData = (const UInt8*) sourceBitmapInternals.mPixelData.getBytePtr();
			UInt32	sourceBytesPerRow = sourceBitmapInternals.mBytesPerRow;
			UInt8*	destinationPixelData = (UInt8*) destinationBitmapInternals.mPixelData.getMutableBytePtr();
			UInt32	destinationBytesPerRow = destinationBitmapInternals.mBytesPerRow;

	// Loop on vertical
	for (SInt32 h = 0; h < sourceBitmapInternals.mSize.mHeight; h++) {
		// Setup
		const	UInt32*	srcPtr = (const UInt32*) (sourcePixelData + h * sourceBytesPerRow);
				UInt16*	dstPtr = (UInt16*) (destinationPixelData + h * destinationBytesPerRow);

		// Loop on horizontal
		for (SInt32 w = 0; w < sourceBitmapInternals.mSize.mWidth; w++, srcPtr++, dstPtr++) {
			// Convert color
#if TARGET_RT_LITTLE_ENDIAN
			UInt32	red = (((*srcPtr >> 0) & 0xFF) * 15 + 127) / 255;
			UInt32	green = (((*srcPtr >> 8) & 0xFF) * 15 + 127) / 255;
			UInt32	blue = (((*srcPtr >> 16) & 0xFF) * 15 + 127) / 255;
			UInt32	alpha = (((*srcPtr >> 24) & 0xFF) * 15 + 127) / 255;
			*dstPtr = (UInt16) ((red << 12) | (green << 8) | (blue << 4) | alpha);
#else
#if defined(__clang__)
	#warning TODO - convertRGBA8888ToRGBA4444 for Big Endian
#endif
			AssertFailUnimplemented();
#endif
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertRGBA8888ToRGBA5551(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	UInt8*	sourcePixelData = (const UInt8*) sourceBitmapInternals.mPixelData.getBytePtr();
			UInt32	sourceBytesPerRow = sourceBitmapInternals.mBytesPerRow;
			UInt8*	destinationPixelData = (UInt8*) destinationBitmapInternals.mPixelData.getMutableBytePtr();
			UInt32	destinationBytesPerRow = destinationBitmapInternals.mBytesPerRow;

	// Loop on vertical
	for (SInt32 h = 0; h < sourceBitmapInternals.mSize.mHeight; h++) {
		// Setup
		const	UInt32*	srcPtr = (const UInt32*) (sourcePixelData + h * sourceBytesPerRow);
				UInt16*	dstPtr = (UInt16*) (destinationPixelData + h * destinationBytesPerRow);

		// Loop on horizontal
		for (SInt32 w = 0; w < sourceBitmapInternals.mSize.mWidth; w++, srcPtr++, dstPtr++) {
			// Convert color
#if TARGET_RT_LITTLE_ENDIAN
			UInt32	red = (((*srcPtr >> 0) & 0xFF) * 31 + 127) / 255;
			UInt32	green = (((*srcPtr >> 8) & 0xFF) * 31 + 127) / 255;
			UInt32	blue = (((*srcPtr >> 16) & 0xFF) * 31 + 127) / 255;
			UInt32	alpha = (((*srcPtr >> 24) & 0xFF) * 1 + 127) / 255;
			*dstPtr = (UInt16) ((red << 11) | (green << 6) | (blue << 1) | alpha);
#else
#if defined(__clang__)
	#warning TODO - convertRGBA8888ToRGBA5551 for Big Endian
#endif
			AssertFailUnimplemented();
#endif
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertRGBA8888ToARGB8888(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertARGB8888ToRGBA4444(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertARGB8888ToRGBA5551(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertARGB8888ToRGBA8888(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}
