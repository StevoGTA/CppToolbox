//----------------------------------------------------------------------------------------------------------------------
//	CBitmap.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "C2DGeometry.h"
#include "CColor.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: BitmapFormat

enum EBitmapFormat {
	// 16 bit formats
	kBitmapFormatRGB565,
	kBitmapFormatRGBA4444,
	kBitmapFormatRGBA5551,

	// 24 bit formats
	kBitmapFormatRGB888,

	// 32 bit formats
	kBitmapFormatRGBA8888,
	kBitmapFormatARGB8888,
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Raw pixel data

union SPixelDataRGBA4444 {
	UInt16	mRawData;
	struct {
		UInt16	mR : 4;
		UInt16	mG : 4;
		UInt16	mB : 4;
		UInt16	mA : 4;
	} mColor;
};

union SPixelDataRGBA5551 {
	UInt16	mRawData;
	struct {
		UInt16	mR : 5;
		UInt16	mG : 5;
		UInt16	mB : 5;
		UInt16	mA : 1;
	} mColor;
};

union SPixelDataRGB565 {
	UInt16	mRawData;
	struct {
		UInt16	mR : 5;
		UInt16	mG : 6;
		UInt16	mB : 5;
	} mColor;
};

union SPixelDataRGB888 {
	UInt16	mRawData;
	struct {
		UInt16	mR : 8;
		UInt16	mG : 8;
		UInt16	mB : 8;
	} mColor;
};

union SPixelDataRGBA8888 {
	UInt16	mRawData;
	struct {
		UInt16	mR : 8;
		UInt16	mG : 8;
		UInt16	mB : 8;
		UInt16	mA : 8;
	} mColor;
};

union SPixelDataARGB8888 {
	UInt16	mRawData;
	struct {
		UInt16	mA : 8;
		UInt16	mR : 8;
		UInt16	mG : 8;
		UInt16	mB : 8;
	} mColor;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Rotation operation

enum EBitmapRotationOperation {
	kBitmapRotationOperationRotateNone	= 0x00,
	kBitmapRotationOperationRotate90	= 0x01,
	kBitmapRotationOperationRotate180	= 0x02,
	kBitmapRotationOperationRotate270	= 0x03,

	kBitmapRotationOperationFlipLR		= 0x04,
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBitmap

class CBitmapInternals;
class CBitmap {
	// Methods
	public:
										// Lifecycle methods
										CBitmap(const S2DSizeS32& size = S2DSizeS32(1, 1),
												EBitmapFormat format = kBitmapFormatRGBA8888, UInt16 bytesPerRow = 0);
										CBitmap(const S2DSizeS32& size, EBitmapFormat format, const CData& pixelData,
												UInt16 bytesPerRow);
										CBitmap(const CBitmap& other, EBitmapFormat format);
										CBitmap(const CBitmap& other, UInt32 rotationOperation);
										CBitmap(const CBitmap& other);
		virtual							~CBitmap();

										// Instance methods
				const	S2DSizeS32		getSize() const;

						CData&			getPixelData() const;
						EBitmapFormat	getFormat() const;
						UInt16			getBytesPerRow() const;
						UInt16			getBytesPerPixel() const;

						void			clearPixels()
											{ clearPixels(S2DRectS32(S2DPointS32(), getSize())); }
						void			clearPixels(const S2DRectS32& rect);

						void			setPixel(const S2DPointS32& point, const CColor& color);	// Set one pixel to the one pixel's worth of given data
						void			setPixels(const S2DRectS32& rect, const CColor& color);		// Set all pixels to the one pixel's worth of given data

	// Properties
	private:
		CBitmapInternals*	mInternals;
};
