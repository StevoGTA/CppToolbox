//----------------------------------------------------------------------------------------------------------------------
//	CColor.h			©2005 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"
#include "CMatrix.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CColor
/*
	RGB:
		Red 0.0 to 1.0
		Green 0.0 to 1.0
		Blue 0.0 to 1.0

	HSV:
		Hue 0.0 to 360.0: 0.0 is red, 60.0 is yellow, 120.0 is green, 180.0 is cyan, 240.0 is blue,
				300.0 is magenta, 360.0 = 0.0
		Saturation 0.0 to 1.0: 0.0 is all white, 1.0 is fully hue
		Value 0.0 to 1.0: 0.0 is all black, 1.0 is fully hue + saturation

	Alpha 0.0 to 1.0: 0.0 is fully transparent, 1.0 is fully opaque
*/

class CColor : public CEquatable {
	// Primaries
	public:
		enum Primaries {
			// Values
			kPrimariesRec601,	// SD Video
			kPrimariesRec709,	// HD Video
			kPrimariesRec2020,	// HDR Video
			kPrimariesEBU3213,	// PAL Video

			kPrimariesDCIP3,
			kPrimariesP3D65,
			kPrimariesP22,		// sRGB Video

			// Aliases
			kPrimariesSMPTE_C = kPrimariesRec601,
		};

	// Conversion Matrix
	public:
		enum YCbCrConversionMatrix {
			kYCbCrConversionMatrixRec601,
			kYCbCrConversionMatrixRec709,
			kYCbCrConversionMatrixRec2020,
		};

	// Transfer function
	public:
		enum TransferFunction {
			kTransferFunctionRec709,	// SD and HD Video
			kTransferFunctionRec2020,	// HDR Video
			kTransferFunctionSRGB,		// sRGB Video
		};

	// Structs
	public:
		struct RGBValues {
			// Methods
			public:
						// Lifecycle methods
						RGBValues(Float32 red, Float32 green, Float32 blue, Float32 alpha = 1.0) :
							mRed(red), mGreen(green), mBlue(blue), mAlpha(alpha)
							{}
						RGBValues(UInt8 red, UInt8 green, UInt8 blue, UInt8 alpha = 255) :
							mRed((Float32) red / (Float32) 255.0), mGreen((Float32) green / (Float32) 255.0),
									mBlue((Float32) blue / (Float32) 255.0), mAlpha((Float32) alpha / (Float32) 255.0)
							{}
						RGBValues(const RGBValues& other) :
							mRed(other.mRed), mGreen(other.mGreen), mBlue(other.mBlue), mAlpha(other.mAlpha)
							{}

						// Instance methods
				Float32	getRed() const
							{ return mRed; }
				Float32	getGreen() const
							{ return mGreen; }
				Float32	getBlue() const
							{ return mBlue; }
				Float32	getAlpha() const
							{ return mAlpha; }

				bool	operator==(const RGBValues& other)
							{ return (mRed == other.mRed) && (mGreen == other.mGreen) && (mBlue == other.mBlue) &&
									(mAlpha == other.mAlpha); }

			// Properties
			private:
				Float32	mRed;
				Float32	mGreen;
				Float32	mBlue;
				Float32	mAlpha;
		};

		struct RGBTransformer {
			// Methods
			public:
								// Lifecycle methods
								RGBTransformer() : mR(1.0), mG(1.0), mB(1.0), mA(1.0) {}
								RGBTransformer(Float32 r, Float32 g, Float32 b, Float32 a) :
									mR(r), mG(g), mB(b), mA(a)
									{}

								// Instance methods
				RGBValues		multiply(const RGBValues& rgbValues) const
									{ return RGBValues(rgbValues.getRed() * mR, rgbValues.getGreen() * mG,
											rgbValues.getBlue() * mB, rgbValues.getAlpha() * mA); }
				RGBValues		add(const RGBValues& rgbValues) const
									{ return RGBValues(rgbValues.getRed() + mR, rgbValues.getGreen() + mG,
											rgbValues.getBlue() + mB, rgbValues.getAlpha() + mA); }

				RGBTransformer	operator*(Float32 factor) const
									{ return RGBTransformer(mR * factor, mG * factor, mB * factor, mA * factor); }
				RGBTransformer	operator+(const RGBTransformer& other) const
									{ return RGBTransformer(mR + other.mR, mG + other.mG, mB + other.mB,
											mA + other.mA); }

			// Properties
			private:
				Float32	mR;
				Float32	mG;
				Float32	mB;
				Float32	mA;
		};

		struct RGBColorTransform {
			// Methods
			public:
									// Lifecycle methods
									RGBColorTransform() {}
									RGBColorTransform(const RGBTransformer& multiplier,
											const RGBTransformer& adder) :
										mMultiplier(multiplier), mAdder(adder)
										{}

									// Instance methods
				RGBValues			apply(const RGBValues& rgbValues) const
										{ return mAdder.add(mMultiplier.multiply(rgbValues)); }

				RGBColorTransform	operator*(Float32 factor) const
										{ return RGBColorTransform(mMultiplier * factor, mAdder * factor); }
				RGBColorTransform	operator+(const RGBColorTransform& other) const
										{ return RGBColorTransform(mMultiplier + other.mMultiplier,
												mAdder + other.mAdder); }

			// Properties
			private:
				RGBTransformer	mMultiplier;
				RGBTransformer	mAdder;
		};

		struct HSVValues {
			// Methods
			public:
						// Lifecycle methods
						HSVValues(Float32 hue, Float32 saturation, Float32 value, Float32 alpha = 1.0) :
							mHue(hue), mSaturation(saturation), mValue(value), mAlpha(alpha)
							{}
						HSVValues(const HSVValues& other) :
							mHue(other.mHue), mSaturation(other.mSaturation), mValue(other.mValue), mAlpha(other.mAlpha)
							{}

						// Instance methods
				Float32	getHue() const
							{ return mHue; }
				Float32	getSaturation() const
							{ return mSaturation; }
				Float32	getValue() const
							{ return mValue; }
				Float32	getAlpha() const
							{ return mAlpha; }

				bool	operator==(const HSVValues& other)
							{ return (mHue == other.mHue) && (mSaturation == other.mSaturation) &&
									(mValue == other.mValue) && (mAlpha == other.mAlpha); }

			// Properties
			private:
				Float32	mHue;
				Float32	mSaturation;
				Float32	mValue;
				Float32	mAlpha;
		};

		struct HSVTransformer {
			// Methods
			public:
								// Lifecycle methods
								HSVTransformer() : mH(1.0), mS(1.0), mV(1.0), mA(1.0) {}
								HSVTransformer(Float32 r, Float32 g, Float32 b, Float32 a) :
										mH(r), mS(g), mV(b), mA(a)
										{}

								// Instance methods
				HSVValues		multiply(const HSVValues& hsvValues) const
									{ return HSVValues(hsvValues.getHue() * mH, hsvValues.getSaturation() * mS,
											hsvValues.getValue() * mV, hsvValues.getAlpha() * mA); }
				HSVValues		add(const HSVValues& hsvValues) const
									{ return HSVValues(hsvValues.getHue() + mH, hsvValues.getSaturation() + mS,
											hsvValues.getValue() + mV, hsvValues.getAlpha() + mA); }

				HSVTransformer	operator*(Float32 factor) const
									{ return HSVTransformer(mH * factor, mS * factor, mV * factor, mA * factor); }
				HSVTransformer	operator+(const HSVTransformer& other) const
									{ return HSVTransformer(mH + other.mH, mS + other.mS, mV + other.mV,
											mA + other.mA); }

			// Properties
			private:
				Float32	mH;
				Float32	mS;
				Float32	mV;
				Float32	mA;
		};

		struct HSVColorTransform {
			// Methods
			public:
									// Lifecycle methods
									HSVColorTransform() {}
									HSVColorTransform(const HSVTransformer& multiplier,
											const HSVTransformer& adder) :
										mMultiplier(multiplier), mAdder(adder)
										{}

									// Instance methods
				HSVValues			apply(const HSVValues& hsvValues) const
										{ return mAdder.add(mMultiplier.multiply(hsvValues)); }

				HSVColorTransform	operator*(Float32 factor) const
										{ return HSVColorTransform(mMultiplier * factor, mAdder * factor); }
				HSVColorTransform	operator+(const HSVColorTransform& other) const
										{ return HSVColorTransform(mMultiplier + other.mMultiplier,
													mAdder + other.mAdder); }

			// Properties
			private:
				HSVTransformer	mMultiplier;
				HSVTransformer	mAdder;
		};

	// Classes
	private:
		class Internals;

	// Methods
	public:
									// Lifecycle methods
									CColor(const RGBValues& rgbValues);
									CColor(const HSVValues& hsvValues);
									CColor(const CString& rgbHexString);
									CColor(const CDictionary& info);
									CColor(const CColor& other);
									~CColor();

									// CEquatable methods
				bool				operator==(const CEquatable& other) const;

									// Instance methods
				RGBValues			getRGBValues() const;
				HSVValues			getHSVValues() const;

				CDictionary			getInfo() const;

				CColor				operator*(const RGBColorTransform& transform) const
										{ return CColor(transform.apply(getRGBValues())); }
				CColor				operator*(const HSVColorTransform& transform) const
										{ return CColor(transform.apply(getHSVValues())); }

									// Class methods
		static	OR<const CColor>	colorForName(const CString& colorName);

		static	bool				areEqual(const CColor& color1, const CColor& color2)
										{ return color1 == color2; }

	// Properties
	public:
		static	const	CColor			mClear;
		static	const	CColor			mAliceBlue;
		static	const	CColor			mAntiqueWhite;
		static	const	CColor			mAqua;
		static	const	CColor			mAquamarine;
		static	const	CColor			mAzure;
		static	const	CColor			mBeige;
		static	const	CColor			mBisque;
		static	const	CColor			mBlack;
		static	const	CColor			mBlanchedAlmond;
		static	const	CColor			mBlue;
		static	const	CColor			mBlueViolet;
		static	const	CColor			mBrown;
		static	const	CColor			mBurlywood;
		static	const	CColor			mCadetBlue;
		static	const	CColor			mChartreuse;
		static	const	CColor			mChocolate;
		static	const	CColor			mCoral;
		static	const	CColor			mCornflowerBlue;
		static	const	CColor			mCornSilk;
		static	const	CColor			mCrimson;
		static	const	CColor			mCyan;
		static	const	CColor			mDarkBlue;
		static	const	CColor			mDarkCyan;
		static	const	CColor			mDarkGoldenrod;
		static	const	CColor			mDarkGray;
		static	const	CColor			mDarkGreen;
		static	const	CColor			mDarkGrey;
		static	const	CColor			mDarkKhaki;
		static	const	CColor			mDarkMagenta;
		static	const	CColor			mDarkOliveGreen;
		static	const	CColor			mDarkOrange;
		static	const	CColor			mDarkOrchid;
		static	const	CColor			mDarkdRed;
		static	const	CColor			mDarkSalmon;
		static	const	CColor			mDarkSeaGreen;
		static	const	CColor			mDarkSlateBlue;
		static	const	CColor			mDarkSlateGray;
		static	const	CColor			mDarkSlateGrey;
		static	const	CColor			mDarkTurquoise;
		static	const	CColor			mDarkViolet;
		static	const	CColor			mDeepPink;
		static	const	CColor			mDeepSkyBlue;
		static	const	CColor			mDimGray;
		static	const	CColor			mDimGrey;
		static	const	CColor			mDodgerBlue;
		static	const	CColor			mFireBrick;
		static	const	CColor			mFloralWhite;
		static	const	CColor			mForestGreen;
		static	const	CColor			mFuchsia;
		static	const	CColor			mGainsboro;
		static	const	CColor			mGhostWhite;
		static	const	CColor			mGold;
		static	const	CColor			mGoldenrod;
		static	const	CColor			mGray;
		static	const	CColor			mGrey;
		static	const	CColor			mGreen;
		static	const	CColor			mGreenYellow;
		static	const	CColor			mHoneydew;
		static	const	CColor			mHotPink;
		static	const	CColor			mIndianRed;
		static	const	CColor			mIndigo;
		static	const	CColor			mIvory;
		static	const	CColor			mKhaki;
		static	const	CColor			mLavender;
		static	const	CColor			mLavenderBlush;
		static	const	CColor			mLawnGreen;
		static	const	CColor			mLemonChiffon;
		static	const	CColor			mLightBlue;
		static	const	CColor			mLightCoral;
		static	const	CColor			mLightCyan;
		static	const	CColor			mLightGoldenrodYellow;
		static	const	CColor			mLightGray;
		static	const	CColor			mLightGreen;
		static	const	CColor			mLightGrey;
		static	const	CColor			mLightPink;
		static	const	CColor			mLighSalmon;
		static	const	CColor			mLightSeaGreen;
		static	const	CColor			mLightSkyBlue;
		static	const	CColor			mLightSlateGray;
		static	const	CColor			mLightSlateGrey;
		static	const	CColor			mLightSteelBlue;
		static	const	CColor			mLightYellow;
		static	const	CColor			mLime;
		static	const	CColor			mLimeGreen;
		static	const	CColor			mLinen;
		static	const	CColor			mMagenta;
		static	const	CColor			mMaroon;
		static	const	CColor			mMediumAquamarine;
		static	const	CColor			mMediumBlue;
		static	const	CColor			mMediumOrchid;
		static	const	CColor			mMediumPurple;
		static	const	CColor			mMediumSeaGreen;
		static	const	CColor			mMediumSlateBlue;
		static	const	CColor			mMediumSpringGreen;
		static	const	CColor			mMediumTurquoise;
		static	const	CColor			mMediumVioletRed;
		static	const	CColor			mMidnightBlue;
		static	const	CColor			mMintCream;
		static	const	CColor			mMistyRose;
		static	const	CColor			mMoccasin;
		static	const	CColor			mNavajoWhite;
		static	const	CColor			mNavy;
		static	const	CColor			mOldLace;
		static	const	CColor			mOlive;
		static	const	CColor			mOliveDrab;
		static	const	CColor			mOrange;
		static	const	CColor			mOrangeRed;
		static	const	CColor			mOrchid;
		static	const	CColor			mPaleGoldenrod;
		static	const	CColor			mPaleGreen;
		static	const	CColor			mPaleTurquoise;
		static	const	CColor			mPaleVioletRed;
		static	const	CColor			mPapayaWhip;
		static	const	CColor			mPeachPuff;
		static	const	CColor			mPeru;
		static	const	CColor			mPink;
		static	const	CColor			mPlum;
		static	const	CColor			mPowderBlue;
		static	const	CColor			mPurple;
		static	const	CColor			mRed;
		static	const	CColor			mRosyBrown;
		static	const	CColor			mRoyalBlue;
		static	const	CColor			mSadleBrown;
		static	const	CColor			mSalmon;
		static	const	CColor			mSandyBrown;
		static	const	CColor			mSeaGreen;
		static	const	CColor			mSeashell;
		static	const	CColor			mSienna;
		static	const	CColor			mSilver;
		static	const	CColor			mSkyBlue;
		static	const	CColor			mSlateBlue;
		static	const	CColor			mSlateGray;
		static	const	CColor			mSlateGrey;
		static	const	CColor			mSnow;
		static	const	CColor			mSpringGreen;
		static	const	CColor			mSteelBlue;
		static	const	CColor			mTan;
		static	const	CColor			mTeal;
		static	const	CColor			mThistle;
		static	const	CColor			mTomato;
		static	const	CColor			mTurquoise;
		static	const	CColor			mViolet;
		static	const	CColor			mWheat;
		static	const	CColor			mWhite;
		static	const	CColor			mWhiteSmoke;
		static	const	CColor			mYellow;
		static	const	CColor			mYellowGreen;

		static	const	SMatrix3x3_32	mYCbCrConverstionMatrixRec601VideoRange;
		static	const	SMatrix3x3_32	mYCbCrConverstionMatrixRec601FullRange;
		static	const	SMatrix3x3_32	mYCbCrConverstionMatrixRec709VideoRange;

	private:
						Internals*		mInternals;
};
