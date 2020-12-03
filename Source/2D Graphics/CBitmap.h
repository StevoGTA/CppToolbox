//----------------------------------------------------------------------------------------------------------------------
//	CBitmap.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "C2DGeometry.h"
#include "CColor.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Raw pixel data

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
// MARK: - CBitmap

class CBitmapInternals;
class CBitmap {
	// Types
	public:
		enum Format {
			// 16 bit formats
			kFormatRGB565,
			kFormatRGBA4444,
			kFormatRGBA5551,

			// 24 bit formats
			kFormatRGB888,

			// 32 bit formats
			kFormatRGBA8888,
			kFormatARGB8888,
		};

		enum RotationOperation {
			kRotationOperationRotateNone	= 0x00,
			kRotationOperationRotate90		= 0x01,
			kRotationOperationRotate180		= 0x02,
			kRotationOperationRotate270		= 0x03,

			kRotationOperationFlipLR		= 0x04,
		};

	// Methods
	public:
									// Lifecycle methods
									CBitmap(const S2DSizeS32& size = S2DSizeS32(1, 1), Format format = kFormatRGBA8888,
											UInt16 bytesPerRow = 0);
									CBitmap(const S2DSizeS32& size, Format format, const CData& pixelData,
											UInt16 bytesPerRow);
									CBitmap(const CBitmap& other, Format format);
									CBitmap(const CBitmap& other, RotationOperation rotationOperation);
									CBitmap(const CBitmap& other);
		virtual						~CBitmap();

									// Instance methods
				const	S2DSizeS32&	getSize() const;

						CData&		getPixelData() const;
						Format		getFormat() const;
						UInt16		getBytesPerRow() const;
						UInt16		getBytesPerPixel() const;

						void		clearPixels()
										{ clearPixels(S2DRectS32(S2DPointS32(), getSize())); }
						void		clearPixels(const S2DRectS32& rect);

						void		setPixel(const S2DPointS32& point, const CColor& color);	// Set one pixel to the one pixel's worth of given data
						void		setPixels(const S2DRectS32& rect, const CColor& color);		// Set all pixels to the one pixel's worth of given data

	// Properties
	private:
		CBitmapInternals*	mInternals;
};
