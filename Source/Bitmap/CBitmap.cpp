//----------------------------------------------------------------------------------------------------------------------
//	CBitmap.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CBitmap.h"

#include "CppToolboxAssert.h"

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

class CBitmapInternals {
	public:
							CBitmapInternals(const SBitmapSize& size, EBitmapFormat format, const CData& pixelData,
									UInt16 bytesPerRow) :
								mSize(size), mFormat(format), mBytesPerRow(bytesPerRow), mReferenceCount(1)
								{
									// Finish setup
									switch (mFormat) {
										case kBitmapFormatRGBA4444:
										case kBitmapFormatRGBA5551:
										case kBitmapFormatRGB565:
											mBytesPerPixel = 2;
											break;

										case kBitmapFormatRGB888:
											mBytesPerPixel = 3;
											break;

										case kBitmapFormatRGBA8888:
										case kBitmapFormatARGB8888:
											mBytesPerPixel = 4;
											break;
									}

									if (mBytesPerRow == 0)
										// Set default bytes per row
										mBytesPerRow = (mSize.mWidth * mBytesPerPixel) & 0xFFFFFFF0 + 0x0F;

									mPixelData = !pixelData.isEmpty() ? pixelData : CData(mBytesPerRow * mSize.mHeight);
								}

		CBitmapInternals*	addReference() { mReferenceCount++; return this; }
		void				removeReference()
								{
									// Decrement reference count and check if we are the last one
									if (--mReferenceCount == 0) {
										// We going away
										CBitmapInternals*	THIS = this;
										DisposeOf(THIS);
									}
								}

		EBitmapFormat		mFormat;
		SBitmapSize			mSize;
		CData				mPixelData;
		UInt32				mBytesPerPixel;
		UInt32				mBytesPerRow;
		UInt32				mReferenceCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBitmap

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CBitmap::CBitmap(const SBitmapSize& size, EBitmapFormat format, UInt16 bytesPerRow)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertFailIf(size.mWidth < 1);
	AssertFailIf(size.mHeight < 1);

	// Setup
	mInternals =
			new CBitmapInternals(SBitmapSize(std::max(1, size.mWidth), std::max(1, size.mHeight)), format,
					CData::mEmpty, bytesPerRow);
}

//----------------------------------------------------------------------------------------------------------------------
CBitmap::CBitmap(const SBitmapSize& size, EBitmapFormat format, const CData& pixelData, UInt16 bytesPerRow)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertFailIf(size.mWidth < 1);
	AssertFailIf(size.mHeight < 1);

	// Setup
	mInternals =
			new CBitmapInternals(SBitmapSize(std::max(1, size.mWidth), std::max(1, size.mHeight)), format, pixelData,
					bytesPerRow);
}

//----------------------------------------------------------------------------------------------------------------------
CBitmap::CBitmap(const CBitmap& other, EBitmapFormat format)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CBitmapInternals(other.mInternals->mSize, format, CData::mEmpty, 0);

	// Convert
	switch (other.mInternals->mFormat) {
		case kBitmapFormatRGB565:
			// RGB565 =>
			switch (format) {
				case kBitmapFormatRGB888:
					// => RGB888
					sConvertRGB565ToRGB888(*other.mInternals, *mInternals);
					break;

				case kBitmapFormatRGBA8888:
					// => RGBA8888
					sConvertRGB565ToRGBA8888(*other.mInternals, *mInternals);
					break;

				case kBitmapFormatARGB8888:
					// => ARGB8888
					sConvertRGB565ToARGB8888(*other.mInternals, *mInternals);
					break;

				default:
					// Would never do
					AssertFailUnimplemented();
			}
			break;

		case kBitmapFormatRGBA4444:
			// RGBA4444 =>
			switch (format) {
				case kBitmapFormatRGBA8888:
					// => RGBA8888
					sConvertRGBA4444ToRGBA8888(*other.mInternals, *mInternals);
					break;

				case kBitmapFormatARGB8888:
					// => ARGB8888
					sConvertRGBA4444ToARGB8888(*other.mInternals, *mInternals);
					break;

				default:
					// Would never do
					AssertFailUnimplemented();
			}
			break;

		case kBitmapFormatRGBA5551:
			// RGBA5551 =>
			switch (format) {
				case kBitmapFormatRGBA8888:
					// => RGBA8888
					sConvertRGBA5551ToRGBA8888(*other.mInternals, *mInternals);
					break;

				case kBitmapFormatARGB8888:
					// => ARGB8888
					sConvertRGBA5551ToARGB8888(*other.mInternals, *mInternals);
					break;

				default:
					// Would never do
					AssertFailUnimplemented();
			}
			break;

		case kBitmapFormatRGB888:
			// RGB888 =>
			switch (format) {
				case kBitmapFormatRGB565:
					// => RGB565
					sConvertRGB888ToRGB565(*other.mInternals, *mInternals);
					break;

				case kBitmapFormatRGBA4444:
					// => RGBA4444
					sConvertRGB888ToRGBA4444(*other.mInternals, *mInternals);
					break;

				case kBitmapFormatRGBA5551:
					// => RGBA5551
					sConvertRGB888ToRGBA5551(*other.mInternals, *mInternals);
					break;

				case kBitmapFormatRGBA8888:
					// => RGBA8888
					sConvertRGB888ToRGBA8888(*other.mInternals, *mInternals);
					break;

				case kBitmapFormatARGB8888:
					// => ARGB8888
					sConvertRGB888ToARGB8888(*other.mInternals, *mInternals);
					break;

				default:
					// Would never do
					AssertFailUnimplemented();
			}
			break;

		case kBitmapFormatRGBA8888:
			// RGBA8888 =>
			switch (format) {
				case kBitmapFormatRGB565:
					// => RGB565
					sConvertRGBA8888ToRGB565(*other.mInternals, *mInternals);
					break;

				case kBitmapFormatRGBA4444:
					// => RGBA4444
					sConvertRGBA8888ToRGBA4444(*other.mInternals, *mInternals);
					break;

				case kBitmapFormatRGBA5551:
					// => RGBA5551
					sConvertRGBA8888ToRGBA5551(*other.mInternals, *mInternals);
					break;

				case kBitmapFormatARGB8888:
					// => ARGB8888
					sConvertRGBA8888ToARGB8888(*other.mInternals, *mInternals);
					break;

				default:
					// Would never do
					AssertFailUnimplemented();
			}
			break;

		case kBitmapFormatARGB8888:
			// ARGB8888 =>
			switch (format) {
				case kBitmapFormatRGBA4444:
					// => RGBA4444
					sConvertARGB8888ToRGBA4444(*other.mInternals, *mInternals);
					break;

				case kBitmapFormatRGBA5551:
					// => RGBA5551
					sConvertARGB8888ToRGBA5551(*other.mInternals, *mInternals);
					break;

				case kBitmapFormatRGBA8888:
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
CBitmap::CBitmap(const CBitmap& other, UInt32 rotationOperation)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SBitmapSize	newSize;
	switch (rotationOperation & 0x03) {
		case kBitmapRotationOperationRotateNone:
		case kBitmapRotationOperationRotate180:
			// Flipping
			newSize = other.mInternals->mSize;
			break;

		case kBitmapRotationOperationRotate90:
		case kBitmapRotationOperationRotate270:
			// Rotating
			newSize = SBitmapSize(other.mInternals->mSize.mHeight, other.mInternals->mSize.mWidth);
			break;
	}

	mInternals = new CBitmapInternals(newSize, other.mInternals->mFormat, CData::mEmpty, 0);

	// Rotate
	// bytePtr = A * y + B * x + C
	SInt32		A = 0;
	SInt32		B = 0;
	SInt32		C = 0;
	SInt32		bytesPerPixel = (SInt32) other.mInternals->mBytesPerPixel;
	SInt32		bytesPerRow = other.mInternals->mBytesPerRow;
	SBitmapSize	size = other.mInternals->mSize;
	switch ((UInt32) rotationOperation) {
		case kBitmapRotationOperationRotateNone:
			// bytePtr = y * bytesPerRow + x * bytesPerPixel
			// bytePtr = bytesPerRow * y + bytesPerPixel * x + 0
			A = bytesPerRow;
			B = bytesPerPixel;
			C = 0;
			break;

		case kBitmapRotationOperationRotateNone | kBitmapRotationOperationFlipLR:
			// bytePtr = y * bytesPerRow + ((width - 1) - x) * bytesPerPixel
			// bytePtr = y * bytesPerRow + (width - 1) * bytesPerPixel - x * bytesPerPixel
			// bytePtr = bytesPerRow * y - bytesPerPixel * x + (width - 1) * bytesPerPixel
			A = bytesPerRow;
			B = -bytesPerPixel;
			C = (size.mWidth - 1) * bytesPerPixel;
			break;

		case kBitmapRotationOperationRotate90:
			// bytePtr = x * bytesPerRow + ((height - 1) - y) * bytesPerPixel
			// bytePtr = x * bytesPerRow + (height - 1) * bytesPerPixel - y * bytesPerPixel
			// bytePtr = -bytesPerPixel * y + bytesPerRow * x + (height - 1) * bytesPerPixel
			A = -bytesPerPixel;
			B = bytesPerRow;
			C = (size.mHeight - 1) * bytesPerPixel;
			break;

		case kBitmapRotationOperationRotate90 | kBitmapRotationOperationFlipLR:
			// bytePtr = x * bytesPerRow + y * bytesPerPixel
			// bytePtr = bytesPerPixel * y + bytesPerRow * x + 0
			A = bytesPerPixel;
			B = bytesPerRow;
			C = 0;
			break;

		case kBitmapRotationOperationRotate180:
			// bytePtr = ((height - 1) - y) * bytesPerRow + ((width - 1) - x) * bytesPerPixel
			// bytePtr = (height - 1) * bytesPerRow - y * bytesPerRow + (width - 1) * bytesPerPixel - x * bytesPerPixel
			// bytePtr = -bytesPerRow * y -bytesPerPixel * x + (height - 1) * bytesPerRow + (width - 1) * bytesPerPixel
			A = -bytesPerRow;
			B = -bytesPerPixel;
			C = (size.mHeight - 1) * bytesPerRow + (size.mWidth - 1) * bytesPerPixel;
			break;

		case kBitmapRotationOperationRotate180 | kBitmapRotationOperationFlipLR:
			// bytePtr = ((height - 1) - y) * bytesPerRow + x * bytesPerPixel
			// bytePtr = (height - 1) * bytesPerRow - y * bytesPerRow + x * bytesPerPixel
			// bytePtr = -bytesPerRow * y + bytesPerPixel * x + (height - 1) * bytesPerRow;
			A = -bytesPerRow;
			B = bytesPerPixel;
			C = (size.mHeight - 1) * bytesPerRow;
			break;

		case kBitmapRotationOperationRotate270:
			// bytePtr = ((width - 1) - x) * bytesPerRow + y * bytesPerPixel
			// bytePtr = (width - 1) * bytesPerRow - x * bytesPerRow + y * bytesPerPixel
			// bytePtr = bytesPerPixel * y - bytesPerRow * x + (width - 1) * bytesPerRow;
			A = bytesPerPixel;
			B = -bytesPerRow;
			C = (size.mWidth - 1) * bytesPerRow;
			break;

		case kBitmapRotationOperationRotate270 | kBitmapRotationOperationFlipLR:
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
const SBitmapSize CBitmap::getSize() const
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
EBitmapFormat CBitmap::getFormat() const
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
void CBitmap::setPixel(const SBitmapPoint& pt, const CColor& color)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertFailIf((pt.mX < 0) || (pt.mX >= mInternals->mSize.mWidth));
	if ((pt.mX < 0) || (pt.mX >= mInternals->mSize.mWidth))
		return;

	AssertFailIf((pt.mY < 0) || (pt.mY >= mInternals->mSize.mHeight));
	if ((pt.mY < 0) || (pt.mY >= mInternals->mSize.mHeight))
		return;

	void*	pixelDataPtr =
					(UInt8*) mInternals->mPixelData.getMutableBytePtr() + pt.mY * mInternals->mBytesPerRow +
							pt.mX * mInternals->mBytesPerPixel;
	switch (mInternals->mFormat) {
		case kBitmapFormatRGBA4444: {
			// RGBA4444
			SPixelDataRGBA4444*	pixelData = (SPixelDataRGBA4444*) pixelDataPtr;
			pixelData->mColor.mR = (UInt16) (color.getRed() * 15.0);
			pixelData->mColor.mG = (UInt16) (color.getGreen() * 15.0);
			pixelData->mColor.mB = (UInt16) (color.getBlue() * 15.0);
			pixelData->mColor.mA = (UInt16) (color.getAlpha() * 15.0);
			} break;

		case kBitmapFormatRGBA5551: {
			// RGBA5551
			SPixelDataRGBA5551*	pixelData = (SPixelDataRGBA5551*) pixelDataPtr;
			pixelData->mColor.mR = (UInt16) (color.getRed() * 31.0);
			pixelData->mColor.mG = (UInt16) (color.getGreen() * 31.0);
			pixelData->mColor.mB = (UInt16) (color.getBlue() * 31.0);
			pixelData->mColor.mA = (UInt16) (color.getAlpha() * 1.0);
			} break;

		case kBitmapFormatRGB565: {
			// RGB565
			SPixelDataRGB565*	pixelData = (SPixelDataRGB565*) pixelDataPtr;
			pixelData->mColor.mR = (UInt16) (color.getRed() * 31.0);
			pixelData->mColor.mG = (UInt16) (color.getGreen() * 63.0);
			pixelData->mColor.mB = (UInt16) (color.getBlue() * 31.0);
			} break;

		case kBitmapFormatRGB888: {
			// RGB888
			SPixelDataRGB888*	pixelData = (SPixelDataRGB888*) pixelDataPtr;
			pixelData->mColor.mR = (UInt16) (color.getRed() * 255.0);
			pixelData->mColor.mG = (UInt16) (color.getGreen() * 255.0);
			pixelData->mColor.mB = (UInt16) (color.getBlue() * 255.0);
			} break;

		case kBitmapFormatRGBA8888: {
			// RGBA8888
			SPixelDataRGBA8888*	pixelData = (SPixelDataRGBA8888*) pixelDataPtr;
			pixelData->mColor.mR = (UInt16) (color.getRed() * 255.0);
			pixelData->mColor.mG = (UInt16) (color.getGreen() * 255.0);
			pixelData->mColor.mB = (UInt16) (color.getBlue() * 255.0);
			pixelData->mColor.mA = (UInt16) (color.getAlpha() * 255.0);
			} break;

		case kBitmapFormatARGB8888: {
			// ARGB8888
			SPixelDataARGB8888*	pixelData = (SPixelDataARGB8888*) pixelDataPtr;
			pixelData->mColor.mA = (UInt16) (color.getAlpha() * 255.0);
			pixelData->mColor.mR = (UInt16) (color.getRed() * 255.0);
			pixelData->mColor.mG = (UInt16) (color.getGreen() * 255.0);
			pixelData->mColor.mB = (UInt16) (color.getBlue() * 255.0);
			} break;
	}
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
void sConvertRGB565ToRGB888(const CBitmapInternals& sourceBitmapInternals, CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailWith(kUnimplementedError);
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
			AssertFailWith(kUnimplementedError);
#endif
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertRGB565ToARGB8888(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailWith(kUnimplementedError);
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
			AssertFailWith(kUnimplementedError);
#endif
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertRGBA4444ToARGB8888(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailWith(kUnimplementedError);
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
			AssertFailWith(kUnimplementedError);
#endif
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertRGBA5551ToARGB8888(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailWith(kUnimplementedError);
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertRGB888ToRGB565(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailWith(kUnimplementedError);
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertRGB888ToRGBA4444(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailWith(kUnimplementedError);
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertRGB888ToRGBA5551(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailWith(kUnimplementedError);
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
				UInt32*	dstPtr = (UInt32*) (destinationPixelData + h * destinationBytesPerRow);
		for (UInt32 w = 0; w < sourceBitmapInternals.mSize.mWidth; w++, dstPtr++) {
			// Convert color
#if TARGET_RT_LITTLE_ENDIAN
			UInt32	red = *(srcPtr++);
			UInt32	green = *(srcPtr++);
			UInt32	blue = *(srcPtr++);
			*dstPtr = (red << 0) | (green << 8) | (blue << 16) | 0xFF000000;
#else
#if defined(__clang__)
	#warning TODO - convertRGB888ToRGBA8888 for Big Endian
#endif
			AssertFailWith(kUnimplementedError);
#endif
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertRGB888ToARGB8888(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailWith(kUnimplementedError);
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
			*dstPtr = (red << 11) | (green << 5) | blue;
#else
#if defined(__clang__)
	#warning TODO - convertRGBA8888ToRGB565 for Big Endian
#endif
			AssertFailWith(kUnimplementedError);
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
			*dstPtr = (red << 12) | (green << 8) | (blue << 4) | alpha;
#else
#if defined(__clang__)
	#warning TODO - convertRGBA8888ToRGBA4444 for Big Endian
#endif
			AssertFailWith(kUnimplementedError);
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
			*dstPtr = (red << 11) | (green << 6) | (blue << 1) | alpha;
#else
#if defined(__clang__)
	#warning TODO - convertRGBA8888ToRGBA5551 for Big Endian
#endif
			AssertFailWith(kUnimplementedError);
#endif
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertRGBA8888ToARGB8888(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailWith(kUnimplementedError);
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertARGB8888ToRGBA4444(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailWith(kUnimplementedError);
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertARGB8888ToRGBA5551(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailWith(kUnimplementedError);
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertARGB8888ToRGBA8888(const CBitmapInternals& sourceBitmapInternals,
		CBitmapInternals& destinationBitmapInternals)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailWith(kUnimplementedError);
}
