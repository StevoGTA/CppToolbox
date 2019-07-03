//----------------------------------------------------------------------------------------------------------------------
//	CColor.h			©2005 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Color Type

//----------------------------------------------------------------------------------------------------------------------
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

enum EColorType {
	kColorTypeRGB,
	kColorTypeHSV,
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SRGBColorTransformMultiplier

struct SRGBColorTransformMultiplier {
											// Lifecycle methods
											SRGBColorTransformMultiplier() : mR(1.0), mG(1.0), mB(1.0), mA(1.0) {}
											SRGBColorTransformMultiplier(Float32 r, Float32 g, Float32 b, Float32 a) :
												mR(r), mG(g), mB(b), mA(a) {}

											// Instance methods
	inline	SRGBColorTransformMultiplier	operator*(Float32 factor) const
												{
													return SRGBColorTransformMultiplier(mR * factor, mG * factor,
															mB * factor, mA * factor);
												}
	inline	SRGBColorTransformMultiplier	operator+(const SRGBColorTransformMultiplier& other) const
												{
													return SRGBColorTransformMultiplier(mR + other.mR,
															mG + other.mG, mB + other.mB, mA + other.mA);
												}

	// Properties
	Float32	mR;
	Float32	mG;
	Float32	mB;
	Float32	mA;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SRGBColorTransformAdder

struct SRGBColorTransformAdder {
									// Lifecycle methods
									SRGBColorTransformAdder() : mR(0.0), mG(0.0), mB(0.0), mA(0.0) {}
									SRGBColorTransformAdder(Float32 r, Float32 g, Float32 b, Float32 a) :
										mR(r), mG(g), mB(b), mA(a) {}

									// Instance methods
	inline	SRGBColorTransformAdder	operator*(Float32 factor) const
										{
											return SRGBColorTransformAdder(mR * factor, mG * factor,
													mB * factor, mA * factor);
										}
	inline	SRGBColorTransformAdder	operator+(const SRGBColorTransformAdder& other) const
										{
											return SRGBColorTransformAdder(mR + other.mR,
													mG + other.mG, mB + other.mB, mA + other.mA);
										}

	// Properties
	Float32	mR;
	Float32	mG;
	Float32	mB;
	Float32	mA;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SRGBColorTransform

struct SRGBColorTransform {
								// Lifecycle methods
								SRGBColorTransform() :
									mMultiplier(SRGBColorTransformMultiplier()),
									mAdder(SRGBColorTransformAdder())
									{}
								SRGBColorTransform(const SRGBColorTransformMultiplier& multiplier,
										const SRGBColorTransformAdder& adder) :
									mMultiplier(multiplier), mAdder(adder) {}

								// Instance methods
	inline	SRGBColorTransform	operator*(Float32 factor) const
									{ return SRGBColorTransform(mMultiplier * factor, mAdder * factor); }
	inline	SRGBColorTransform	operator+(const SRGBColorTransform& other) const
									{
										return SRGBColorTransform(mMultiplier + other.mMultiplier,
												mAdder + other.mAdder);
									}

	// Properties
	SRGBColorTransformMultiplier	mMultiplier;
	SRGBColorTransformAdder			mAdder;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SHSVColorTransformMultiplier

struct SHSVColorTransformMultiplier {
	// Lifecycle methods
	SHSVColorTransformMultiplier() : mH(1.0), mS(1.0), mV(1.0), mA(1.0) {}
	SHSVColorTransformMultiplier(Float32 h, Float32 s, Float32 v, Float32 a) : mH(h), mS(s), mV(v), mA(a) {}

	// Properties
	Float32	mH;
	Float32	mS;
	Float32	mV;
	Float32	mA;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SHSVColorTransformAdder

struct SHSVColorTransformAdder {
	// Lifecycle methods
	SHSVColorTransformAdder() : mH(0.0), mS(0.0), mV(0.0), mA(0.0) {}
	SHSVColorTransformAdder(Float32 h, Float32 s, Float32 v, Float32 a) : mH(h), mS(s), mV(v), mA(a) {}

	// Properties
	Float32	mH;
	Float32	mS;
	Float32	mV;
	Float32	mA;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SHSVColorTransform

struct SHSVColorTransform {
	// Lifecycle methods
	SHSVColorTransform(const SHSVColorTransformMultiplier& multiplier, const SHSVColorTransformAdder& adder) :
		mMultiplier(multiplier), mAdder(adder) {}

	// Properties
	SHSVColorTransformMultiplier	mMultiplier;
	SHSVColorTransformAdder			mAdder;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CColor

class CColorInternals;
class CColor : public CEquatable {
	// Methods
	public:
									// Lifecycle methods
									CColor();
									CColor(const CColor& other);
									CColor(const CDictionary& info);
									CColor(const CString& hexString);
								
									CColor(EColorType type, Float32 val1, Float32 val2, Float32 val3, Float32 alpha);
									CColor(EColorType type, UInt8 val1, UInt8 val2, UInt8 val3, UInt8 alpha);
								
									~CColor();

									// CEquatable methods
						bool		operator==(const CEquatable& other) const
											{ return equals((const CColor&) other); }

									// Instance methods
						Float32		getRed() const;
						Float32		getGreen() const;
						Float32		getBlue() const;

						Float32		getHue() const;
						Float32		getSaturation() const;
						Float32		getValue() const;

						Float32		getAlpha() const;

						CDictionary	getInfo() const;
						CString		getInfoAsString() const;

						bool		equals(const CColor& other) const;

						CColor&		operator=(const CColor& other);

						CColor		operator+(const CColor& other) const;

						CColor		operator*(const SRGBColorTransform& transform) const;
						CColor		operator*(const SHSVColorTransform& transform) const;
						CColor&		operator*=(const SRGBColorTransform& transform);
						CColor&		operator*=(const SHSVColorTransform& transform);

									// Class methods
		static	const	CColor*		getColorWithName(const CString& colorName);
		static			bool		areEqual(const CColor& color1, const CColor& color2)
											{ return color1 == color2; }

	// Properties
	public:
		static	const	CColor				mClear;
		static	const	CColor				mAliceBlue;
		static	const	CColor				mAntiqueWhite;
		static	const	CColor				mAqua;
		static	const	CColor				mAquamarine;
		static	const	CColor				mAzure;
		static	const	CColor				mBeige;
		static	const	CColor				mBisque;
		static	const	CColor				mBlack;
		static	const	CColor				mBlanchedAlmond;
		static	const	CColor				mBlue;
		static	const	CColor				mBlueViolet;
		static	const	CColor				mBrown;
		static	const	CColor				mBurlywood;
		static	const	CColor				mCadetBlue;
		static	const	CColor				mChartreuse;
		static	const	CColor				mChocolate;
		static	const	CColor				mCoral;
		static	const	CColor				mCornflowerBlue;
		static	const	CColor				mCornSilk;
		static	const	CColor				mCrimson;
		static	const	CColor				mCyan;
		static	const	CColor				mDarkBlue;
		static	const	CColor				mDarkCyan;
		static	const	CColor				mDarkGoldenrod;
		static	const	CColor				mDarkGray;
		static	const	CColor				mDarkGreen;
		static	const	CColor				mDarkGrey;
		static	const	CColor				mDarkKhaki;
		static	const	CColor				mDarkMagenta;
		static	const	CColor				mDarkOliveGreen;
		static	const	CColor				mDarkOrange;
		static	const	CColor				mDarkOrchid;
		static	const	CColor				mDarkdRed;
		static	const	CColor				mDarkSalmon;
		static	const	CColor				mDarkSeaGreen;
		static	const	CColor				mDarkSlateBlue;
		static	const	CColor				mDarkSlateGray;
		static	const	CColor				mDarkSlateGrey;
		static	const	CColor				mDarkTurquoise;
		static	const	CColor				mDarkViolet;
		static	const	CColor				mDeepPink;
		static	const	CColor				mDeepSkyBlue;
		static	const	CColor				mDimGray;
		static	const	CColor				mDimGrey;
		static	const	CColor				mDodgerBlue;
		static	const	CColor				mFireBrick;
		static	const	CColor				mFloralWhite;
		static	const	CColor				mForestGreen;
		static	const	CColor				mFuchsia;
		static	const	CColor				mGainsboro;
		static	const	CColor				mGhostWhite;
		static	const	CColor				mGold;
		static	const	CColor				mGoldenrod;
		static	const	CColor				mGray;
		static	const	CColor				mGrey;
		static	const	CColor				mGreen;
		static	const	CColor				mGreenYellow;
		static	const	CColor				mHoneydew;
		static	const	CColor				mHotPink;
		static	const	CColor				mIndianRed;
		static	const	CColor				mIndigo;
		static	const	CColor				mIvory;
		static	const	CColor				mKhaki;
		static	const	CColor				mLavender;
		static	const	CColor				mLavenderBlush;
		static	const	CColor				mLawnGreen;
		static	const	CColor				mLemonChiffon;
		static	const	CColor				mLightBlue;
		static	const	CColor				mLightCoral;
		static	const	CColor				mLightCyan;
		static	const	CColor				mLightGoldenrodYellow;
		static	const	CColor				mLightGray;
		static	const	CColor				mLightGreen;
		static	const	CColor				mLightGrey;
		static	const	CColor				mLightPink;
		static	const	CColor				mLighSalmon;
		static	const	CColor				mLightSeaGreen;
		static	const	CColor				mLightSkyBlue;
		static	const	CColor				mLightSlateGray;
		static	const	CColor				mLightSlateGrey;
		static	const	CColor				mLightSteelBlue;
		static	const	CColor				mLightYellow;
		static	const	CColor				mLime;
		static	const	CColor				mLimeGreen;
		static	const	CColor				mLinen;
		static	const	CColor				mMagenta;
		static	const	CColor				mMaroon;
		static	const	CColor				mMediumAquamarine;
		static	const	CColor				mMediumBlue;
		static	const	CColor				mMediumOrchid;
		static	const	CColor				mMediumPurple;
		static	const	CColor				mMediumSeaGreen;
		static	const	CColor				mMediumSlateBlue;
		static	const	CColor				mMediumSpringGreen;
		static	const	CColor				mMediumTurquoise;
		static	const	CColor				mMediumVioletRed;
		static	const	CColor				mMidnightBlue;
		static	const	CColor				mMintCream;
		static	const	CColor				mMistyRose;
		static	const	CColor				mMoccasin;
		static	const	CColor				mNavajoWhite;
		static	const	CColor				mNavy;
		static	const	CColor				mOldLace;
		static	const	CColor				mOlive;
		static	const	CColor				mOliveDrab;
		static	const	CColor				mOrange;
		static	const	CColor				mOrangeRed;
		static	const	CColor				mOrchid;
		static	const	CColor				mPaleGoldenrod;
		static	const	CColor				mPaleGreen;
		static	const	CColor				mPaleTurquoise;
		static	const	CColor				mPaleVioletRed;
		static	const	CColor				mPapayaWhip;
		static	const	CColor				mPeachPuff;
		static	const	CColor				mPeru;
		static	const	CColor				mPink;
		static	const	CColor				mPlum;
		static	const	CColor				mPowderBlue;
		static	const	CColor				mPurple;
		static	const	CColor				mRed;
		static	const	CColor				mRosyBrown;
		static	const	CColor				mRoyalBlue;
		static	const	CColor				mSadleBrown;
		static	const	CColor				mSalmon;
		static	const	CColor				mSandyBrown;
		static	const	CColor				mSeaGreen;
		static	const	CColor				mSeashell;
		static	const	CColor				mSienna;
		static	const	CColor				mSilver;
		static	const	CColor				mSkyBlue;
		static	const	CColor				mSlateBlue;
		static	const	CColor				mSlateGray;
		static	const	CColor				mSlateGrey;
		static	const	CColor				mSnow;
		static	const	CColor				mSpringGreen;
		static	const	CColor				mSteelBlue;
		static	const	CColor				mTan;
		static	const	CColor				mTeal;
		static	const	CColor				mThistle;
		static	const	CColor				mTomato;
		static	const	CColor				mTurquoise;
		static	const	CColor				mViolet;
		static	const	CColor				mWheat;
		static	const	CColor				mWhite;
		static	const	CColor				mWhiteSmoke;
		static	const	CColor				mYellow;
		static	const	CColor				mYellowGreen;

	private:
						CColorInternals*	mInternals;
};
