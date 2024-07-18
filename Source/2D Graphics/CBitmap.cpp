//----------------------------------------------------------------------------------------------------------------------
//	CBitmap.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CBitmap.h"

#include "CppToolboxAssert.h"
#include "CReferenceCountable.h"
#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: RowWriterRGBA8888

class RowWriterRGBA8888 : public CBitmap::RowWriter {
	public:
				RowWriterRGBA8888(void* bytePtr) : RowWriter(), mPixelData((CBitmap::PixelDataRGBA8888*) bytePtr) {}

		void	write(const CBitmap::PixelDataRGB888& pixelData)
					{
						// Store
						mPixelData->mComponents.mR = pixelData.mR;
						mPixelData->mComponents.mG = pixelData.mG;
						mPixelData->mComponents.mB = pixelData.mB;
						mPixelData->mComponents.mA = 0xFF;

						// Update
						mPixelData++;
					}

	private:
		CBitmap::PixelDataRGBA8888*	mPixelData;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBitmap::Internals

class CBitmap::Internals : public TCopyOnWriteReferenceCountable<Internals>{
	public:
						Internals(const S2DSizeS32& size, CBitmap::Format format,
								const CData& pixelData = CData::mEmpty,
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
										mBytesPerRow += 0x10 - (mBytesPerRow & 0x0F);
								}
								mPixelData =
										!pixelData.isEmpty() ?
												pixelData : CData((CData::ByteCount) mBytesPerRow * mSize.mHeight);
							}
						Internals(const Internals& other) :
							TCopyOnWriteReferenceCountable(),
									mFormat(other.mFormat), mSize(other.mSize), mPixelData(other.mPixelData),
									mBytesPerPixel(other.mBytesPerPixel), mBytesPerRow(other.mBytesPerRow)
							{}

		static	void	convertRGB565ToRGB888(const Internals& sourceBitmapInternals,
								Internals& destinationBitmapInternals)
							{ AssertFailUnimplemented(); }
		static	void	convertRGB565ToRGBA8888(const Internals& sourceBitmapInternals,
								Internals& destinationBitmapInternals)
							{
								// Setup
								const	UInt8*	sourcePixelData =
														(const UInt8*) sourceBitmapInternals.mPixelData.getBytePtr();
										UInt32	sourceBytesPerRow = sourceBitmapInternals.mBytesPerRow;
										UInt8*	destinationPixelData =
														(UInt8*) destinationBitmapInternals.mPixelData
																.getMutableBytePtr();
										UInt32	destinationBytesPerRow = destinationBitmapInternals.mBytesPerRow;

								// Loop on vertical
								for (SInt32 h = 0; h < sourceBitmapInternals.mSize.mHeight; h++) {
									// Setup
									const	UInt16*	srcPtr = (const UInt16*) (sourcePixelData + h * sourceBytesPerRow);
											UInt32*	dstPtr =
															(UInt32*) (destinationPixelData + h *
																	destinationBytesPerRow);

									// Loop on horizontal
									for (SInt32 w = 0; w < sourceBitmapInternals.mSize.mWidth;
											w++, srcPtr++, dstPtr++) {
										// Convert color
#if TARGET_RT_LITTLE_ENDIAN
										UInt32	red = (((*srcPtr >> 11) & 0x1F) * 255 + 15) / 31;
										UInt32	green = (((*srcPtr >> 5) & 0x3F) * 255 + 31) / 63;
										UInt32	blue = (((*srcPtr >> 0) & 0x1F) * 255 + 15) / 31;
										*dstPtr = (red << 0) | (green << 8) | (blue << 16) | 0xFF000000;
#else
										AssertFailUnimplemented();
#endif
									}
								}
							}
		static	void	convertRGB565ToARGB8888(const Internals& sourceBitmapInternals,
								Internals& destinationBitmapInternals)
							{ AssertFailUnimplemented(); }

		static	void	convertRGBA4444ToRGBA8888(const Internals& sourceBitmapInternals,
								Internals& destinationBitmapInternals)
							{
								// Setup
								const	UInt8*	sourcePixelData =
														(const UInt8*) sourceBitmapInternals.mPixelData.getBytePtr();
										UInt32	sourceBytesPerRow = sourceBitmapInternals.mBytesPerRow;
										UInt8*	destinationPixelData =
														(UInt8*) destinationBitmapInternals.mPixelData
																.getMutableBytePtr();
										UInt32	destinationBytesPerRow = destinationBitmapInternals.mBytesPerRow;

								// Loop on vertical
								for (SInt32 h = 0; h < sourceBitmapInternals.mSize.mHeight; h++) {
									// Setup
									const	UInt16*	srcPtr = (const UInt16*) (sourcePixelData + h * sourceBytesPerRow);
											UInt32*	dstPtr =
															(UInt32*) (destinationPixelData + h *
																	destinationBytesPerRow);

									// Loop on horizontal
									for (SInt32 w = 0; w < sourceBitmapInternals.mSize.mWidth;
											w++, srcPtr++, dstPtr++) {
										// Convert color
#if TARGET_RT_LITTLE_ENDIAN
										UInt32	red = (((*srcPtr >> 12) & 0x0F) * 255 + 7) / 15;
										UInt32	green = (((*srcPtr >> 8) & 0x0F) * 255 + 7) / 15;
										UInt32	blue = (((*srcPtr >> 4) & 0x0F) * 255 + 7) / 15;
										UInt32	alpha = (((*srcPtr >> 0) & 0x0F) * 255 + 7) / 15;
										*dstPtr = (red << 0) | (green << 8) | (blue << 16) | (alpha << 24);
#else
										AssertFailUnimplemented();
#endif
									}
								}
							}
		static	void	convertRGBA4444ToARGB8888(const Internals& sourceBitmapInternals,
								Internals& destinationBitmapInternals)
							{ AssertFailUnimplemented(); }

		static	void	convertRGBA5551ToRGBA8888(const Internals& sourceBitmapInternals,
								Internals& destinationBitmapInternals)
							{
								// Setup
								const	UInt8*	sourcePixelData =
														(const UInt8*) sourceBitmapInternals.mPixelData.getBytePtr();
										UInt32	sourceBytesPerRow = sourceBitmapInternals.mBytesPerRow;
										UInt8*	destinationPixelData =
														(UInt8*) destinationBitmapInternals.mPixelData
																.getMutableBytePtr();
										UInt32	destinationBytesPerRow = destinationBitmapInternals.mBytesPerRow;

								// Loop on vertical
								for (SInt32 h = 0; h < sourceBitmapInternals.mSize.mHeight; h++) {
									// Setup
									const	UInt16*	srcPtr = (const UInt16*) (sourcePixelData + h * sourceBytesPerRow);
											UInt32*	dstPtr =
															(UInt32*) (destinationPixelData + h *
																	destinationBytesPerRow);

									// Loop on horizontal
									for (SInt32 w = 0; w < sourceBitmapInternals.mSize.mWidth;
											w++, srcPtr++, dstPtr++) {
										// Convert color
#if TARGET_RT_LITTLE_ENDIAN
										UInt32	red = (((*srcPtr >> 11) & 0x1F) * 255 + 15) / 31;
										UInt32	green = (((*srcPtr >> 6) & 0x1F) * 255 + 15) / 31;
										UInt32	blue = (((*srcPtr >> 1) & 0x1F) * 255 + 15) / 31;
										UInt32	alpha = (((*srcPtr >> 0) & 0x01) * 255 + 0) / 1;
										*dstPtr = (red << 0) | (green << 8) | (blue << 16) | (alpha << 24);
#else
										AssertFailUnimplemented();
#endif
									}
								}
							}
		static	void	convertRGBA5551ToARGB8888(const Internals& sourceBitmapInternals,
								Internals& destinationBitmapInternals)
							{ AssertFailUnimplemented(); }

		static	void	convertRGB888ToRGB565(const Internals& sourceBitmapInternals,
								Internals& destinationBitmapInternals)
							{ AssertFailUnimplemented(); }
		static	void	convertRGB888ToRGBA4444(const Internals& sourceBitmapInternals,
								Internals& destinationBitmapInternals)
							{ AssertFailUnimplemented(); }
		static	void	convertRGB888ToRGBA5551(const Internals& sourceBitmapInternals,
								Internals& destinationBitmapInternals)
							{ AssertFailUnimplemented(); }
		static	void	convertRGB888ToRGBA8888(const Internals& sourceBitmapInternals,
								Internals& destinationBitmapInternals)
							{
								// Setup
								const	UInt8*	sourcePixelData =
														(const UInt8*) sourceBitmapInternals.mPixelData.getBytePtr();
										UInt32	sourceBytesPerRow = sourceBitmapInternals.mBytesPerRow;
										UInt8*	destinationPixelData =
														(UInt8*) destinationBitmapInternals.mPixelData
																.getMutableBytePtr();
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
										AssertFailUnimplemented();
#endif
									}
								}
							}
		static	void	convertRGB888ToARGB8888(const Internals& sourceBitmapInternals,
								Internals& destinationBitmapInternals)
							{
								// Setup
								const	UInt8*	sourcePixelData =
														(const UInt8*) sourceBitmapInternals.mPixelData.getBytePtr();
										UInt32	sourceBytesPerRow = sourceBitmapInternals.mBytesPerRow;
										UInt8*	destinationPixelData =
														(UInt8*) destinationBitmapInternals.mPixelData
																.getMutableBytePtr();
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
										AssertFailUnimplemented();
#endif
									}
								}
							}

		static	void	convertRGBA8888ToRGB565(const Internals& sourceBitmapInternals,
								Internals& destinationBitmapInternals)
							{
								// Setup
								const	UInt8*	sourcePixelData =
														(const UInt8*) sourceBitmapInternals.mPixelData.getBytePtr();
										UInt32	sourceBytesPerRow = sourceBitmapInternals.mBytesPerRow;
										UInt8*	destinationPixelData =
														(UInt8*) destinationBitmapInternals.mPixelData
																.getMutableBytePtr();
										UInt32	destinationBytesPerRow = destinationBitmapInternals.mBytesPerRow;

								// Loop on vertical
								for (SInt32 h = 0; h < sourceBitmapInternals.mSize.mHeight; h++) {
									// Setup
									const	UInt32*	srcPtr = (const UInt32*) (sourcePixelData + h * sourceBytesPerRow);
											UInt16*	dstPtr =
															(UInt16*) (destinationPixelData + h *
																	destinationBytesPerRow);

									// Loop on horizontal
									for (SInt32 w = 0; w < sourceBitmapInternals.mSize.mWidth;
											w++, srcPtr++, dstPtr++) {
										// Convert color
#if TARGET_RT_LITTLE_ENDIAN
										UInt32	red = (((*srcPtr >> 0) & 0xFF) * 31 + 127) / 255;
										UInt32	green = (((*srcPtr >> 8) & 0xFF) * 63 + 127) / 255;
										UInt32	blue = (((*srcPtr >> 16) & 0xFF) * 31 + 127) / 255;
										*dstPtr = (UInt16) ((red << 11) | (green << 5) | blue);
#else
										AssertFailUnimplemented();
#endif
									}
								}
							}
		static	void	convertRGBA8888ToRGBA4444(const Internals& sourceBitmapInternals,
								Internals& destinationBitmapInternals)
							{
								// Setup
								const	UInt8*	sourcePixelData =
														(const UInt8*) sourceBitmapInternals.mPixelData.getBytePtr();
										UInt32	sourceBytesPerRow = sourceBitmapInternals.mBytesPerRow;
										UInt8*	destinationPixelData =
														(UInt8*) destinationBitmapInternals.mPixelData
																.getMutableBytePtr();
										UInt32	destinationBytesPerRow = destinationBitmapInternals.mBytesPerRow;

								// Loop on vertical
								for (SInt32 h = 0; h < sourceBitmapInternals.mSize.mHeight; h++) {
									// Setup
									const	UInt32*	srcPtr = (const UInt32*) (sourcePixelData + h * sourceBytesPerRow);
											UInt16*	dstPtr =
															(UInt16*) (destinationPixelData + h *
																	destinationBytesPerRow);

									// Loop on horizontal
									for (SInt32 w = 0; w < sourceBitmapInternals.mSize.mWidth;
											w++, srcPtr++, dstPtr++) {
										// Convert color
#if TARGET_RT_LITTLE_ENDIAN
										UInt32	red = (((*srcPtr >> 0) & 0xFF) * 15 + 127) / 255;
										UInt32	green = (((*srcPtr >> 8) & 0xFF) * 15 + 127) / 255;
										UInt32	blue = (((*srcPtr >> 16) & 0xFF) * 15 + 127) / 255;
										UInt32	alpha = (((*srcPtr >> 24) & 0xFF) * 15 + 127) / 255;
										*dstPtr = (UInt16) ((red << 12) | (green << 8) | (blue << 4) | alpha);
#else
										AssertFailUnimplemented();
#endif
									}
								}
							}
		static	void	convertRGBA8888ToRGBA5551(const Internals& sourceBitmapInternals,
								Internals& destinationBitmapInternals)
							{
								// Setup
								const	UInt8*	sourcePixelData =
														(const UInt8*) sourceBitmapInternals.mPixelData.getBytePtr();
										UInt32	sourceBytesPerRow = sourceBitmapInternals.mBytesPerRow;
										UInt8*	destinationPixelData =
														(UInt8*) destinationBitmapInternals.mPixelData
																.getMutableBytePtr();
										UInt32	destinationBytesPerRow = destinationBitmapInternals.mBytesPerRow;

								// Loop on vertical
								for (SInt32 h = 0; h < sourceBitmapInternals.mSize.mHeight; h++) {
									// Setup
									const	UInt32*	srcPtr =
															(const UInt32*) (sourcePixelData + h * sourceBytesPerRow);
											UInt16*	dstPtr =
															(UInt16*) (destinationPixelData + h *
																	destinationBytesPerRow);

									// Loop on horizontal
									for (SInt32 w = 0; w < sourceBitmapInternals.mSize.mWidth;
											w++, srcPtr++, dstPtr++) {
										// Convert color
#if TARGET_RT_LITTLE_ENDIAN
										UInt32	red = (((*srcPtr >> 0) & 0xFF) * 31 + 127) / 255;
										UInt32	green = (((*srcPtr >> 8) & 0xFF) * 31 + 127) / 255;
										UInt32	blue = (((*srcPtr >> 16) & 0xFF) * 31 + 127) / 255;
										UInt32	alpha = (((*srcPtr >> 24) & 0xFF) * 1 + 127) / 255;
										*dstPtr = (UInt16) ((red << 11) | (green << 6) | (blue << 1) | alpha);
#else
										AssertFailUnimplemented();
#endif
									}
								}
							}
		static	void	convertRGBA8888ToARGB8888(const Internals& sourceBitmapInternals,
								Internals& destinationBitmapInternals)
							{ AssertFailUnimplemented(); }

		static	void	convertARGB8888ToRGBA4444(const Internals& sourceBitmapInternals,
								Internals& destinationBitmapInternals)
							{ AssertFailUnimplemented(); }
		static	void	convertARGB8888ToRGBA5551(const Internals& sourceBitmapInternals,
								Internals& destinationBitmapInternals)
							{ AssertFailUnimplemented(); }
		static	void	convertARGB8888ToRGBA8888(const Internals& sourceBitmapInternals,
								Internals& destinationBitmapInternals)
							{ AssertFailUnimplemented(); }

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
			new Internals(S2DSizeS32(std::max(1, size.mWidth), std::max(1, size.mHeight)), format,
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
			new Internals(S2DSizeS32(std::max(1, size.mWidth), std::max(1, size.mHeight)), format, pixelData,
					OV<UInt16>(bytesPerRow));
}

//----------------------------------------------------------------------------------------------------------------------
CBitmap::CBitmap(const CBitmap& other, Format format)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals(other.mInternals->mSize, format);

	// Convert
	switch (other.mInternals->mFormat) {
		case kFormatRGB565:
			// RGB565 =>
			switch (format) {
				case kFormatRGB888:
					// => RGB888
					Internals::convertRGB565ToRGB888(*other.mInternals, *mInternals);
					break;

				case kFormatRGBA8888:
					// => RGBA8888
					Internals::convertRGB565ToRGBA8888(*other.mInternals, *mInternals);
					break;

				case kFormatARGB8888:
					// => ARGB8888
					Internals::convertRGB565ToARGB8888(*other.mInternals, *mInternals);
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
					Internals::convertRGBA4444ToRGBA8888(*other.mInternals, *mInternals);
					break;

				case kFormatARGB8888:
					// => ARGB8888
					Internals::convertRGBA4444ToARGB8888(*other.mInternals, *mInternals);
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
					Internals::convertRGBA5551ToRGBA8888(*other.mInternals, *mInternals);
					break;

				case kFormatARGB8888:
					// => ARGB8888
					Internals::convertRGBA5551ToARGB8888(*other.mInternals, *mInternals);
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
					Internals::convertRGB888ToRGB565(*other.mInternals, *mInternals);
					break;

				case kFormatRGBA4444:
					// => RGBA4444
					Internals::convertRGB888ToRGBA4444(*other.mInternals, *mInternals);
					break;

				case kFormatRGBA5551:
					// => RGBA5551
					Internals::convertRGB888ToRGBA5551(*other.mInternals, *mInternals);
					break;

				case kFormatRGBA8888:
					// => RGBA8888
					Internals::convertRGB888ToRGBA8888(*other.mInternals, *mInternals);
					break;

				case kFormatARGB8888:
					// => ARGB8888
					Internals::convertRGB888ToARGB8888(*other.mInternals, *mInternals);
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
					Internals::convertRGBA8888ToRGB565(*other.mInternals, *mInternals);
					break;

				case kFormatRGBA4444:
					// => RGBA4444
					Internals::convertRGBA8888ToRGBA4444(*other.mInternals, *mInternals);
					break;

				case kFormatRGBA5551:
					// => RGBA5551
					Internals::convertRGBA8888ToRGBA5551(*other.mInternals, *mInternals);
					break;

				case kFormatARGB8888:
					// => ARGB8888
					Internals::convertRGBA8888ToARGB8888(*other.mInternals, *mInternals);
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
					Internals::convertARGB8888ToRGBA4444(*other.mInternals, *mInternals);
					break;

				case kFormatRGBA5551:
					// => RGBA5551
					Internals::convertARGB8888ToRGBA5551(*other.mInternals, *mInternals);
					break;

				case kFormatRGBA8888:
					// => RGBA8888
					Internals::convertARGB8888ToRGBA8888(*other.mInternals, *mInternals);
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

	mInternals = new Internals(newSize, other.mInternals->mFormat);

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
I<CBitmap::RowWriter> CBitmap::getRowWriter(SInt32 y) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf(y >= mInternals->mSize.mHeight);

	// Setup
	void*	bytePtr = (UInt8*) mInternals->mPixelData.getBytePtr() + y * mInternals->mBytesPerRow;

	// Check format
	switch (mInternals->mFormat) {
		case kFormatRGBA8888:	return I<RowWriter>(new RowWriterRGBA8888(bytePtr));
		default:				AssertFailUnimplemented();	return I<RowWriter>(nil);
	}
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
	Internals::prepareForWrite(&mInternals);

	// Update pixel data
	CColor::RGBValues	rgbValues = color.getRGBValues();
	void*				pixelDataPtr =
								(UInt8*) mInternals->mPixelData.getMutableBytePtr() +
										point.mY * mInternals->mBytesPerRow + point.mX * mInternals->mBytesPerPixel;
	switch (mInternals->mFormat) {
		case kFormatRGBA4444: {
			// RGBA4444
			PixelDataRGBA4444*	pixelData = (PixelDataRGBA4444*) pixelDataPtr;
			pixelData->mComponents.mR = (UInt16) (rgbValues.getRed() * 15.0);
			pixelData->mComponents.mG = (UInt16) (rgbValues.getGreen() * 15.0);
			pixelData->mComponents.mB = (UInt16) (rgbValues.getBlue() * 15.0);
			pixelData->mComponents.mA = (UInt16) (rgbValues.getAlpha() * 15.0);
			break;
		}

		case kFormatRGBA5551: {
			// RGBA5551
			PixelDataRGBA5551*	pixelData = (PixelDataRGBA5551*) pixelDataPtr;
			pixelData->mComponents.mR = (UInt16) (rgbValues.getRed() * 31.0);
			pixelData->mComponents.mG = (UInt16) (rgbValues.getGreen() * 31.0);
			pixelData->mComponents.mB = (UInt16) (rgbValues.getBlue() * 31.0);
			pixelData->mComponents.mA = (UInt16) (rgbValues.getAlpha() * 1.0);
			break;
		}

		case kFormatRGB565: {
			// RGB565
			PixelDataRGB565*	pixelData = (PixelDataRGB565*) pixelDataPtr;
			pixelData->mComponents.mR = (UInt16) (rgbValues.getRed() * 31.0);
			pixelData->mComponents.mG = (UInt16) (rgbValues.getGreen() * 63.0);
			pixelData->mComponents.mB = (UInt16) (rgbValues.getBlue() * 31.0);
			break;
		}

		case kFormatRGB888: {
			// RGB888
			PixelDataRGB888*	pixelData = (PixelDataRGB888*) pixelDataPtr;
			pixelData->mR = (UInt8) (rgbValues.getRed() * 255.0);
			pixelData->mG = (UInt8) (rgbValues.getGreen() * 255.0);
			pixelData->mB = (UInt8) (rgbValues.getBlue() * 255.0);
			break;
		}

		case kFormatRGBA8888: {
			// RGBA8888
			PixelDataRGBA8888*	pixelData = (PixelDataRGBA8888*) pixelDataPtr;
			pixelData->mComponents.mR = (UInt8) (rgbValues.getRed() * 255.0);
			pixelData->mComponents.mG = (UInt8) (rgbValues.getGreen() * 255.0);
			pixelData->mComponents.mB = (UInt8) (rgbValues.getBlue() * 255.0);
			pixelData->mComponents.mA = (UInt8) (rgbValues.getAlpha() * 255.0);
			break;
		}

		case kFormatARGB8888: {
			// ARGB8888
			PixelDataARGB8888*	pixelData = (PixelDataARGB8888*) pixelDataPtr;
			pixelData->mComponents.mA = (UInt8) (rgbValues.getAlpha() * 255.0);
			pixelData->mComponents.mR = (UInt8) (rgbValues.getRed() * 255.0);
			pixelData->mComponents.mG = (UInt8) (rgbValues.getGreen() * 255.0);
			pixelData->mComponents.mB = (UInt8) (rgbValues.getBlue() * 255.0);
			break;
		}
	}
}
