//----------------------------------------------------------------------------------------------------------------------
//	CColor.cpp			©2005 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CColor.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sRedKey(OSSTR("r"));
static	CString	sGreenKey(OSSTR("g"));
static	CString	sBlueKey(OSSTR("b"));
static	CString	sHueKey(OSSTR("h"));
static	CString	sSaturationKey(OSSTR("s"));
static	CString	sValueKey(OSSTR("v"));
static	CString	sAlphaKey(OSSTR("a"));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CColorInternals

class CColorInternals {
	public:
		CColorInternals()
			{
				mR = 0.0;
				mG = 0.0;
				mB = 0.0;

				mH = 0.0;
				mS = 1.0;
				mV = 0.0;

				mA = 1.0;
			}
		~CColorInternals() {}

		Float32	mR;
		Float32	mG;
		Float32	mB;

		Float32	mH;
		Float32	mS;
		Float32	mV;

		Float32	mA;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

static	void	sConvertRGBToHSV(CColorInternals& internals);
static	void	sConvertHSVToRGB(CColorInternals& internals);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CColor

// MARK: Properties

const	CColor	CColor::mClear(kColorTypeHSV, (UInt8) 0, 0, 0, 0);
const	CColor	CColor::mAliceBlue(kColorTypeRGB, (UInt8) 240, 248, 255, 255);
const	CColor	CColor::mAntiqueWhite(kColorTypeRGB, (UInt8) 250, 235, 215, 255);
const	CColor	CColor::mAqua(kColorTypeRGB, (UInt8) 0, 255, 255, 255);
const	CColor	CColor::mAquamarine(kColorTypeRGB, (UInt8) 127, 255, 212, 255);
const	CColor	CColor::mAzure(kColorTypeRGB, (UInt8) 240, 255, 255, 255);
const	CColor	CColor::mBeige(kColorTypeRGB, (UInt8) 245, 245, 220, 255);
const	CColor	CColor::mBisque(kColorTypeRGB, (UInt8) 255, 228, 196, 255);
const	CColor	CColor::mBlack(kColorTypeRGB, (UInt8) 0, 0, 0, 255);
const	CColor	CColor::mBlanchedAlmond(kColorTypeRGB, (UInt8) 255, 235, 205, 255);
const	CColor	CColor::mBlue(kColorTypeRGB, (UInt8) 0, 0, 255, 255);
const	CColor	CColor::mBlueViolet(kColorTypeRGB, (UInt8) 138, 43, 226, 255);
const	CColor	CColor::mBrown(kColorTypeRGB, (UInt8) 165, 42, 42, 255);
const	CColor	CColor::mBurlywood(kColorTypeRGB, (UInt8) 222, 184, 135, 255);
const	CColor	CColor::mCadetBlue(kColorTypeRGB, (UInt8) 95, 158, 160, 255);
const	CColor	CColor::mChartreuse(kColorTypeRGB, (UInt8) 127, 255, 0, 255);
const	CColor	CColor::mChocolate(kColorTypeRGB, (UInt8) 210, 105, 30, 255);
const	CColor	CColor::mCoral(kColorTypeRGB, (UInt8) 255, 127, 80, 255);
const	CColor	CColor::mCornflowerBlue(kColorTypeRGB, (UInt8) 100, 149, 237, 255);
const	CColor	CColor::mCornSilk(kColorTypeRGB, (UInt8) 255, 248, 220, 255);
const	CColor	CColor::mCrimson(kColorTypeRGB, (UInt8) 220, 20, 60, 255);
const	CColor	CColor::mCyan(kColorTypeRGB, (UInt8) 0, 255, 255, 255);
const	CColor	CColor::mDarkBlue(kColorTypeRGB, (UInt8) 0, 0, 139, 255);
const	CColor	CColor::mDarkCyan(kColorTypeRGB, (UInt8) 0, 139, 139, 255);
const	CColor	CColor::mDarkGoldenrod(kColorTypeRGB, (UInt8) 184, 134, 11, 255);
const	CColor	CColor::mDarkGray(kColorTypeRGB, (UInt8) 169, 169, 169, 255);
const	CColor	CColor::mDarkGreen(kColorTypeRGB, (UInt8) 0, 100, 0, 255);
const	CColor	CColor::mDarkGrey(kColorTypeRGB, (UInt8) 169, 169, 169, 255);
const	CColor	CColor::mDarkKhaki(kColorTypeRGB, (UInt8) 189, 183, 107, 255);
const	CColor	CColor::mDarkMagenta(kColorTypeRGB, (UInt8) 139, 0, 139, 255);
const	CColor	CColor::mDarkOliveGreen(kColorTypeRGB, (UInt8) 85, 107, 47, 255);
const	CColor	CColor::mDarkOrange(kColorTypeRGB, (UInt8) 255, 140, 0, 255);
const	CColor	CColor::mDarkOrchid(kColorTypeRGB, (UInt8) 153, 50, 204, 255);
const	CColor	CColor::mDarkdRed(kColorTypeRGB, (UInt8) 139, 0, 0, 255);
const	CColor	CColor::mDarkSalmon(kColorTypeRGB, (UInt8) 233, 150, 122, 255);
const	CColor	CColor::mDarkSeaGreen(kColorTypeRGB, (UInt8) 143, 188, 143, 255);
const	CColor	CColor::mDarkSlateBlue(kColorTypeRGB, (UInt8) 72, 61, 139, 255);
const	CColor	CColor::mDarkSlateGray(kColorTypeRGB, (UInt8) 47, 79, 79, 255);
const	CColor	CColor::mDarkSlateGrey(kColorTypeRGB, (UInt8) 47, 79, 79, 255);
const	CColor	CColor::mDarkTurquoise(kColorTypeRGB, (UInt8) 0, 206, 209, 255);
const	CColor	CColor::mDarkViolet(kColorTypeRGB, (UInt8) 148, 0, 211, 255);
const	CColor	CColor::mDeepPink(kColorTypeRGB, (UInt8) 255, 20, 147, 255);
const	CColor	CColor::mDeepSkyBlue(kColorTypeRGB, (UInt8) 0, 191, 255, 255);
const	CColor	CColor::mDimGray(kColorTypeRGB, (UInt8) 105, 105, 105, 255);
const	CColor	CColor::mDimGrey(kColorTypeRGB, (UInt8) 105, 105, 105, 255);
const	CColor	CColor::mDodgerBlue(kColorTypeRGB, (UInt8) 30, 144, 255, 255);
const	CColor	CColor::mFireBrick(kColorTypeRGB, (UInt8) 178, 34, 34, 255);
const	CColor	CColor::mFloralWhite(kColorTypeRGB, (UInt8) 255, 250, 240, 255);
const	CColor	CColor::mForestGreen(kColorTypeRGB, (UInt8) 34, 139, 34, 255);
const	CColor	CColor::mFuchsia(kColorTypeRGB, (UInt8) 255, 0, 255, 255);
const	CColor	CColor::mGainsboro(kColorTypeRGB, (UInt8) 220, 220, 220, 255);
const	CColor	CColor::mGhostWhite(kColorTypeRGB, (UInt8) 248, 248, 255, 255);
const	CColor	CColor::mGold(kColorTypeRGB, (UInt8) 255, 215, 0, 255);
const	CColor	CColor::mGoldenrod(kColorTypeRGB, (UInt8) 218, 165, 32, 255);
const	CColor	CColor::mGray(kColorTypeRGB, (UInt8) 128, 128, 128, 255);
const	CColor	CColor::mGrey(kColorTypeRGB, (UInt8) 128, 128, 128, 255);
const	CColor	CColor::mGreen(kColorTypeRGB, (UInt8) 0, 128, 0, 255);
const	CColor	CColor::mGreenYellow(kColorTypeRGB, (UInt8) 173, 255, 47, 255);
const	CColor	CColor::mHoneydew(kColorTypeRGB, (UInt8) 240, 255, 240, 255);
const	CColor	CColor::mHotPink(kColorTypeRGB, (UInt8) 255, 105, 180, 255);
const	CColor	CColor::mIndianRed(kColorTypeRGB, (UInt8) 205, 92, 92, 255);
const	CColor	CColor::mIndigo(kColorTypeRGB, (UInt8) 75, 0, 130, 255);
const	CColor	CColor::mIvory(kColorTypeRGB, (UInt8) 255, 255, 240, 255);
const	CColor	CColor::mKhaki(kColorTypeRGB, (UInt8) 240, 230, 140, 255);
const	CColor	CColor::mLavender(kColorTypeRGB, (UInt8) 230, 230, 250, 255);
const	CColor	CColor::mLavenderBlush(kColorTypeRGB, (UInt8) 255, 240, 245, 255);
const	CColor	CColor::mLawnGreen(kColorTypeRGB, (UInt8) 124, 252, 0, 255);
const	CColor	CColor::mLemonChiffon(kColorTypeRGB, (UInt8) 255, 250, 205, 255);
const	CColor	CColor::mLightBlue(kColorTypeRGB, (UInt8) 173, 216, 230, 255);
const	CColor	CColor::mLightCoral(kColorTypeRGB, (UInt8) 240, 128, 128, 255);
const	CColor	CColor::mLightCyan(kColorTypeRGB, (UInt8) 224, 255, 255, 255);
const	CColor	CColor::mLightGoldenrodYellow(kColorTypeRGB, (UInt8) 250, 250, 210, 255);
const	CColor	CColor::mLightGray(kColorTypeRGB, (UInt8) 211, 211, 211, 255);
const	CColor	CColor::mLightGreen(kColorTypeRGB, (UInt8) 144, 238, 144, 255);
const	CColor	CColor::mLightGrey(kColorTypeRGB, (UInt8) 211, 211, 211, 255);
const	CColor	CColor::mLightPink(kColorTypeRGB, (UInt8) 255, 182, 193, 255);
const	CColor	CColor::mLighSalmon(kColorTypeRGB, (UInt8) 255, 160, 122, 255);
const	CColor	CColor::mLightSeaGreen(kColorTypeRGB, (UInt8) 32, 178, 170, 255);
const	CColor	CColor::mLightSkyBlue(kColorTypeRGB, (UInt8) 135, 206, 250, 255);
const	CColor	CColor::mLightSlateGray(kColorTypeRGB, (UInt8) 119, 136, 153, 255);
const	CColor	CColor::mLightSlateGrey(kColorTypeRGB, (UInt8) 119, 136, 153, 255);
const	CColor	CColor::mLightSteelBlue(kColorTypeRGB, (UInt8) 176, 196, 222, 255);
const	CColor	CColor::mLightYellow(kColorTypeRGB, (UInt8) 255, 255, 224, 255);
const	CColor	CColor::mLime(kColorTypeRGB, (UInt8) 0, 255, 0, 255);
const	CColor	CColor::mLimeGreen(kColorTypeRGB, (UInt8) 50, 205, 50, 255);
const	CColor	CColor::mLinen(kColorTypeRGB, (UInt8) 250, 240, 230, 255);
const	CColor	CColor::mMagenta(kColorTypeRGB, (UInt8) 255, 0, 255, 255);
const	CColor	CColor::mMaroon(kColorTypeRGB, (UInt8) 128, 0, 0, 255);
const	CColor	CColor::mMediumAquamarine(kColorTypeRGB, (UInt8) 102, 205, 170, 255);
const	CColor	CColor::mMediumBlue(kColorTypeRGB, (UInt8) 0, 0, 205, 255);
const	CColor	CColor::mMediumOrchid(kColorTypeRGB, (UInt8) 186, 85, 211, 255);
const	CColor	CColor::mMediumPurple(kColorTypeRGB, (UInt8) 147, 112, 219, 255);
const	CColor	CColor::mMediumSeaGreen(kColorTypeRGB, (UInt8) 60, 179, 113, 255);
const	CColor	CColor::mMediumSlateBlue(kColorTypeRGB, (UInt8) 123, 104, 238, 255);
const	CColor	CColor::mMediumSpringGreen(kColorTypeRGB, (UInt8) 0, 250, 154, 255);
const	CColor	CColor::mMediumTurquoise(kColorTypeRGB, (UInt8) 72, 209, 204, 255);
const	CColor	CColor::mMediumVioletRed(kColorTypeRGB, (UInt8) 199, 21, 133, 255);
const	CColor	CColor::mMidnightBlue(kColorTypeRGB, (UInt8) 25, 25, 112, 255);
const	CColor	CColor::mMintCream(kColorTypeRGB, (UInt8) 245, 255, 250, 255);
const	CColor	CColor::mMistyRose(kColorTypeRGB, (UInt8) 255, 228, 225, 255);
const	CColor	CColor::mMoccasin(kColorTypeRGB, (UInt8) 255, 228, 181, 255);
const	CColor	CColor::mNavajoWhite(kColorTypeRGB, (UInt8) 255, 222, 173, 255);
const	CColor	CColor::mNavy(kColorTypeRGB, (UInt8) 0, 0, 128, 255);
const	CColor	CColor::mOldLace(kColorTypeRGB, (UInt8) 253, 245, 230, 255);
const	CColor	CColor::mOlive(kColorTypeRGB, (UInt8) 128, 128, 0, 255);
const	CColor	CColor::mOliveDrab(kColorTypeRGB, (UInt8) 107, 142, 35, 255);
const	CColor	CColor::mOrange(kColorTypeRGB, (UInt8) 255, 165, 0, 255);
const	CColor	CColor::mOrangeRed(kColorTypeRGB, (UInt8) 255, 69, 0, 255);
const	CColor	CColor::mOrchid(kColorTypeRGB, (UInt8) 218, 112, 214, 255);
const	CColor	CColor::mPaleGoldenrod(kColorTypeRGB, (UInt8) 238, 232, 170, 255);
const	CColor	CColor::mPaleGreen(kColorTypeRGB, (UInt8) 152, 251, 152, 255);
const	CColor	CColor::mPaleTurquoise(kColorTypeRGB, (UInt8) 175, 238, 238, 255);
const	CColor	CColor::mPaleVioletRed(kColorTypeRGB, (UInt8) 219, 112, 147, 255);
const	CColor	CColor::mPapayaWhip(kColorTypeRGB, (UInt8) 255, 239, 213, 255);
const	CColor	CColor::mPeachPuff(kColorTypeRGB, (UInt8) 255, 218, 185, 255);
const	CColor	CColor::mPeru(kColorTypeRGB, (UInt8) 205, 133, 63, 255);
const	CColor	CColor::mPink(kColorTypeRGB, (UInt8) 255, 192, 203, 255);
const	CColor	CColor::mPlum(kColorTypeRGB, (UInt8) 221, 160, 221, 255);
const	CColor	CColor::mPowderBlue(kColorTypeRGB, (UInt8) 176, 224, 230, 255);
const	CColor	CColor::mPurple(kColorTypeRGB, (UInt8) 128, 0, 128, 255);
const	CColor	CColor::mRed(kColorTypeRGB, (UInt8) 255, 0, 0, 255);
const	CColor	CColor::mRosyBrown(kColorTypeRGB, (UInt8) 188, 143, 143, 255);
const	CColor	CColor::mRoyalBlue(kColorTypeRGB, (UInt8) 65, 105, 225, 255);
const	CColor	CColor::mSadleBrown(kColorTypeRGB, (UInt8) 139, 69, 19, 255);
const	CColor	CColor::mSalmon(kColorTypeRGB, (UInt8) 250, 128, 114, 255);
const	CColor	CColor::mSandyBrown(kColorTypeRGB, (UInt8) 244, 164, 96, 255);
const	CColor	CColor::mSeaGreen(kColorTypeRGB, (UInt8) 46, 139, 87, 255);
const	CColor	CColor::mSeashell(kColorTypeRGB, (UInt8) 255, 245, 238, 255);
const	CColor	CColor::mSienna(kColorTypeRGB, (UInt8) 160, 82, 45, 255);
const	CColor	CColor::mSilver(kColorTypeRGB, (UInt8) 192, 192, 192, 255);
const	CColor	CColor::mSkyBlue(kColorTypeRGB, (UInt8) 135, 206, 235, 255);
const	CColor	CColor::mSlateBlue(kColorTypeRGB, (UInt8) 106, 90, 205, 255);
const	CColor	CColor::mSlateGray(kColorTypeRGB, (UInt8) 112, 128, 144, 255);
const	CColor	CColor::mSlateGrey(kColorTypeRGB, (UInt8) 112, 128, 144, 255);
const	CColor	CColor::mSnow(kColorTypeRGB, (UInt8) 255, 250, 250, 255);
const	CColor	CColor::mSpringGreen(kColorTypeRGB, (UInt8) 0, 255, 127, 255);
const	CColor	CColor::mSteelBlue(kColorTypeRGB, (UInt8) 70, 130, 180, 255);
const	CColor	CColor::mTan(kColorTypeRGB, (UInt8) 210, 180, 140, 255);
const	CColor	CColor::mTeal(kColorTypeRGB, (UInt8) 0, 128, 128, 255);
const	CColor	CColor::mThistle(kColorTypeRGB, (UInt8) 216, 191, 216, 255);
const	CColor	CColor::mTomato(kColorTypeRGB, (UInt8) 255, 99, 71, 255);
const	CColor	CColor::mTurquoise(kColorTypeRGB, (UInt8) 64, 224, 208, 255);
const	CColor	CColor::mViolet(kColorTypeRGB, (UInt8) 238, 130, 238, 255);
const	CColor	CColor::mWheat(kColorTypeRGB, (UInt8) 245, 222, 179, 255);
const	CColor	CColor::mWhite(kColorTypeRGB, (UInt8) 255, 255, 255, 255);
const	CColor	CColor::mWhiteSmoke(kColorTypeRGB, (UInt8) 245, 245, 245, 255);
const	CColor	CColor::mYellow(kColorTypeRGB, (UInt8) 255, 255, 0, 255);
const	CColor	CColor::mYellowGreen(kColorTypeRGB, (UInt8) 154, 205, 50, 255);

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CColor::CColor()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CColorInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CColor::CColor(const CColor& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CColorInternals();
	
	// Copy
	*this = other;
}

//----------------------------------------------------------------------------------------------------------------------
CColor::CColor(const CDictionary& info)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CColorInternals();

	mInternals->mR = info.getFloat32(sRedKey);
	mInternals->mG = info.getFloat32(sGreenKey);
	mInternals->mB = info.getFloat32(sBlueKey);
	mInternals->mH = info.getFloat32(sHueKey);
	mInternals->mS = info.getFloat32(sSaturationKey);
	mInternals->mV = info.getFloat32(sValueKey);
	mInternals->mA = info.getFloat32(sAlphaKey);
}

//----------------------------------------------------------------------------------------------------------------------
CColor::CColor(const CString& hexString)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CColorInternals();
	
	// hexString can be of the form:
	//	#RGB
	//	#RRGGBB
	//	#RRGGBBAA
	//	RGB
	//	RRGGBB
	//	RRGGBBAA
	CStringCharIndex	startIndex = hexString.hasPrefix(CString(OSSTR("#"))) ? 1 : 0;

	if ((hexString.getLength() - startIndex) == 3) {
		// RGB
		UInt8	value;
		
		value = hexString.getSubString(startIndex + 0, 1).getUInt8(16);
		mInternals->mR = ((Float32) value * (Float32) 16.0 + (Float32) value) / (Float32) 255.0;
		
		value = hexString.getSubString(startIndex + 1, 1).getUInt8(16);
		mInternals->mG = ((Float32) value * (Float32) 16.0 + (Float32) value) / (Float32) 255.0;
		
		value = hexString.getSubString(startIndex + 2, 1).getUInt8(16);
		mInternals->mB = ((Float32) value * (Float32) 16.0 + (Float32) value) / (Float32) 255.0;
		
		mInternals->mA = 1.0;
	} else if ((hexString.getLength() - startIndex) == 6) {
		// RRGGBB
		mInternals->mR = (Float32) hexString.getSubString(startIndex + 0, 2).getUInt8(16) / (Float32) 255.0;
		mInternals->mG = (Float32) hexString.getSubString(startIndex + 2, 2).getUInt8(16) / (Float32) 255.0;
		mInternals->mB = (Float32) hexString.getSubString(startIndex + 4, 2).getUInt8(16) / (Float32) 255.0;
		mInternals->mA = 1.0;
	} else {
		// RRGGBBAA
		mInternals->mR = (Float32) hexString.getSubString(startIndex + 0, 2).getUInt8(16) / (Float32) 255.0;
		mInternals->mG = (Float32) hexString.getSubString(startIndex + 2, 2).getUInt8(16) / (Float32) 255.0;
		mInternals->mB = (Float32) hexString.getSubString(startIndex + 4, 2).getUInt8(16) / (Float32) 255.0;
		mInternals->mA = (Float32) hexString.getSubString(startIndex + 6, 2).getUInt8(16) / (Float32) 255.0;
	}

	sConvertRGBToHSV(*mInternals);
}

//----------------------------------------------------------------------------------------------------------------------
CColor::CColor(EColorType type, Float32 val1, Float32 val2, Float32 val3, Float32 alpha)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CColorInternals();

	// Initializor
	if (type == kColorTypeRGB) {
		// RGB Color
		mInternals->mR = val1;
		mInternals->mG = val2;
		mInternals->mB = val3;
		
		sConvertRGBToHSV(*mInternals);
	} else {
		// HSV Color
		mInternals->mH = val1;
		mInternals->mS = val2;
		mInternals->mV = val3;
		
		sConvertHSVToRGB(*mInternals);
	}
	
	mInternals->mA = alpha;
}

//----------------------------------------------------------------------------------------------------------------------
CColor::CColor(EColorType type, UInt8 val1, UInt8 val2, UInt8 val3, UInt8 alpha)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CColorInternals();

	// Initializor
	if (type == kColorTypeRGB) {
		// RGB Color
		mInternals->mR = (Float32) val1 / (Float32) 255.0;
		mInternals->mG = (Float32) val2 / (Float32) 255.0;
		mInternals->mB = (Float32) val3 / (Float32) 255.0;
		
		sConvertRGBToHSV(*mInternals);
	} else {
		// HSV Color
		mInternals->mH = (Float32) val1 / (Float32) 255.0;
		mInternals->mS = (Float32) val2 / (Float32) 255.0;
		mInternals->mV = (Float32) val3 / (Float32) 255.0;
		
		sConvertHSVToRGB(*mInternals);
	}
	
	mInternals->mA = (Float32) alpha / (Float32) 255.0;
}

//----------------------------------------------------------------------------------------------------------------------
CColor::~CColor()
//----------------------------------------------------------------------------------------------------------------------
{
	DisposeOf(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
Float32 CColor::getRed() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mR;
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CColor::getGreen() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mG;
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CColor::getBlue() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mB;
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CColor::getHue() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mH;
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CColor::getSaturation() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mS;
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CColor::getValue() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mV;
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CColor::getAlpha() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mA;
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary CColor::getInfo() const
//----------------------------------------------------------------------------------------------------------------------
{
	CDictionary	info;

	info.set(sRedKey, mInternals->mR);
	info.set(sGreenKey, mInternals->mG);
	info.set(sBlueKey, mInternals->mB);
	info.set(sHueKey, mInternals->mH);
	info.set(sSaturationKey, mInternals->mS);
	info.set(sValueKey, mInternals->mV);
	info.set(sAlphaKey, mInternals->mA);

	return info;
}

//----------------------------------------------------------------------------------------------------------------------
CString CColor::getInfoAsString() const
//----------------------------------------------------------------------------------------------------------------------
{
	return CString(OSSTR("r: ")) + CString(mInternals->mR, 5, 3) + CString(OSSTR(", g: ")) +
			CString(mInternals->mG, 5, 3) + CString(OSSTR(", b: ")) + CString(mInternals->mB, 5, 3) +
			CString(OSSTR(", h: ")) + CString(mInternals->mH, 5, 3) + CString(OSSTR(", s: ")) +
			CString(mInternals->mS, 5, 3) + CString(OSSTR(", v: ")) + CString(mInternals->mV, 5, 3) +
			CString(OSSTR(", a: ")) + CString(mInternals->mA, 5, 3);
}

//----------------------------------------------------------------------------------------------------------------------
bool CColor::equals(const CColor& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mR == other.mInternals->mR &&
			mInternals->mG == other.mInternals->mG &&
			mInternals->mB == other.mInternals->mB &&

			mInternals->mH == other.mInternals->mH &&
			mInternals->mS == other.mInternals->mS &&
			mInternals->mV == other.mInternals->mV &&

			mInternals->mA == other.mInternals->mA;
}

//----------------------------------------------------------------------------------------------------------------------
CColor& CColor::operator=(const CColor& other)
//----------------------------------------------------------------------------------------------------------------------
{
	if (this != &other) {
		mInternals->mR = other.mInternals->mR;
		mInternals->mG = other.mInternals->mG;
		mInternals->mB = other.mInternals->mB;

		mInternals->mH = other.mInternals->mH;
		mInternals->mS = other.mInternals->mS;
		mInternals->mV = other.mInternals->mV;

		mInternals->mA = other.mInternals->mA;
	}
	
	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CColor CColor::operator+(const CColor& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	CColor	color;

	// Perform addition
	color.mInternals->mR = mInternals->mR + other.mInternals->mR;
	color.mInternals->mG = mInternals->mG + other.mInternals->mG;
	color.mInternals->mB = mInternals->mB + other.mInternals->mB;
	color.mInternals->mA = mInternals->mA + other.mInternals->mA;

	// Update HSV
	sConvertRGBToHSV(*color.mInternals);
	
	return color;
}

//----------------------------------------------------------------------------------------------------------------------
CColor CColor::operator*(const SRGBColorTransform& transform) const
//----------------------------------------------------------------------------------------------------------------------
{
	CColor	color;
	
	// Perform transform
	color.mInternals->mR = mInternals->mR * transform.mMultiplier.mR + transform.mAdder.mR;
	color.mInternals->mG = mInternals->mG * transform.mMultiplier.mG + transform.mAdder.mG;
	color.mInternals->mB = mInternals->mB * transform.mMultiplier.mB + transform.mAdder.mB;
	color.mInternals->mA = mInternals->mA * transform.mMultiplier.mA + transform.mAdder.mA;

	// Update HSV
	sConvertRGBToHSV(*color.mInternals);
	
	return color;
}

//----------------------------------------------------------------------------------------------------------------------
CColor CColor::operator*(const SHSVColorTransform& transform) const
//----------------------------------------------------------------------------------------------------------------------
{
	CColor	color;
	
	// Perform transform
	color.mInternals->mH = mInternals->mH * transform.mMultiplier.mH + transform.mAdder.mH;
	color.mInternals->mS = mInternals->mS * transform.mMultiplier.mS + transform.mAdder.mS;
	color.mInternals->mV = mInternals->mV * transform.mMultiplier.mV + transform.mAdder.mV;
	color.mInternals->mA = mInternals->mA * transform.mMultiplier.mA + transform.mAdder.mA;

	// Update RGB
	sConvertHSVToRGB(*color.mInternals);
	
	return color;
}

//----------------------------------------------------------------------------------------------------------------------
CColor& CColor::operator*=(const SRGBColorTransform& transform)
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform transform
	mInternals->mR = mInternals->mR * transform.mMultiplier.mR + transform.mAdder.mR;
	mInternals->mG = mInternals->mG * transform.mMultiplier.mG + transform.mAdder.mG;
	mInternals->mB = mInternals->mB * transform.mMultiplier.mB + transform.mAdder.mB;
	mInternals->mA = mInternals->mA * transform.mMultiplier.mA + transform.mAdder.mA;

	// Update HSV
	sConvertRGBToHSV(*mInternals);
	
	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CColor& CColor::operator*=(const SHSVColorTransform& transform)
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform transform
	mInternals->mH = mInternals->mH * transform.mMultiplier.mH + transform.mAdder.mH;
	mInternals->mS = mInternals->mS * transform.mMultiplier.mS + transform.mAdder.mS;
	mInternals->mV = mInternals->mV * transform.mMultiplier.mV + transform.mAdder.mV;
	mInternals->mA = mInternals->mA * transform.mMultiplier.mA + transform.mAdder.mA;

	// Update RGB
	sConvertHSVToRGB(*mInternals);
	
	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
OR<const CColor> CColor::getColorWithName(const CString& colorName)
//----------------------------------------------------------------------------------------------------------------------
{
	CString	colorNameUse = colorName.lowercased();
	if (colorNameUse == CString(OSSTR("clear")))
		return OR<const CColor>(mClear);
	if (colorNameUse == CString(OSSTR("aliceblue")))
		return OR<const CColor>(mAliceBlue);
	if (colorNameUse == CString(OSSTR("antiquewhite")))
		return OR<const CColor>(mAntiqueWhite);
	if (colorNameUse == CString(OSSTR("aqua")))
		return OR<const CColor>(mAqua);
	if (colorNameUse == CString(OSSTR("aquamarine")))
		return OR<const CColor>(mAquamarine);
	if (colorNameUse == CString(OSSTR("azure")))
		return OR<const CColor>(mAzure);
	if (colorNameUse == CString(OSSTR("beige")))
		return OR<const CColor>(mBeige);
	if (colorNameUse == CString(OSSTR("bisque")))
		return OR<const CColor>(mBisque);
	if (colorNameUse == CString(OSSTR("black")))
		return OR<const CColor>(mBlack);
	if (colorNameUse == CString(OSSTR("blanchedalmond")))
		return OR<const CColor>(mBlanchedAlmond);
	if (colorNameUse == CString(OSSTR("blue")))
		return OR<const CColor>(mBlue);
	if (colorNameUse == CString(OSSTR("blueviolet")))
		return OR<const CColor>(mBlueViolet);
	if (colorNameUse == CString(OSSTR("brown")))
		return OR<const CColor>(mBrown);
	if (colorNameUse == CString(OSSTR("burlywood")))
		return OR<const CColor>(mBurlywood);
	if (colorNameUse == CString(OSSTR("cadetblue")))
		return OR<const CColor>(mCadetBlue);
	if (colorNameUse == CString(OSSTR("chartreuse")))
		return OR<const CColor>(mChartreuse);
	if (colorNameUse == CString(OSSTR("chocolate")))
		return OR<const CColor>(mChocolate);
	if (colorNameUse == CString(OSSTR("coral")))
		return OR<const CColor>(mCoral);
	if (colorNameUse == CString(OSSTR("cornflowerblue")))
		return OR<const CColor>(mCornflowerBlue);
	if (colorNameUse == CString(OSSTR("cornsilk")))
		return OR<const CColor>(mCornSilk);
	if (colorNameUse == CString(OSSTR("crimson")))
		return OR<const CColor>(mCrimson);
	if (colorNameUse == CString(OSSTR("cyan")))
		return OR<const CColor>(mCyan);
	if (colorNameUse == CString(OSSTR("darkblue")))
		return OR<const CColor>(mDarkBlue);
	if (colorNameUse == CString(OSSTR("darkcyan")))
		return OR<const CColor>(mDarkCyan);
	if (colorNameUse == CString(OSSTR("darkgoldenrod")))
		return OR<const CColor>(mDarkGoldenrod);
	if (colorNameUse == CString(OSSTR("darkgray")))
		return OR<const CColor>(mDarkGray);
	if (colorNameUse == CString(OSSTR("darkgreen")))
		return OR<const CColor>(mDarkGreen);
	if (colorNameUse == CString(OSSTR("darkgrey")))
		return OR<const CColor>(mDarkGrey);
	if (colorNameUse == CString(OSSTR("darkkhaki")))
		return OR<const CColor>(mDarkKhaki);
	if (colorNameUse == CString(OSSTR("darkmagenta")))
		return OR<const CColor>(mDarkMagenta);
	if (colorNameUse == CString(OSSTR("darkolivegreen")))
		return OR<const CColor>(mDarkOliveGreen);
	if (colorNameUse == CString(OSSTR("darkorange")))
		return OR<const CColor>(mDarkOrange);
	if (colorNameUse == CString(OSSTR("darkorchid")))
		return OR<const CColor>(mDarkOrchid);
	if (colorNameUse == CString(OSSTR("darkred")))
		return OR<const CColor>(mDarkdRed);
	if (colorNameUse == CString(OSSTR("darksalmon")))
		return OR<const CColor>(mDarkSalmon);
	if (colorNameUse == CString(OSSTR("darkseagreen")))
		return OR<const CColor>(mDarkSeaGreen);
	if (colorNameUse == CString(OSSTR("darkslateblue")))
		return OR<const CColor>(mDarkSlateBlue);
	if (colorNameUse == CString(OSSTR("darkslategray")))
		return OR<const CColor>(mDarkSlateGray);
	if (colorNameUse == CString(OSSTR("darkslategrey")))
		return OR<const CColor>(mDarkSlateGrey);
	if (colorNameUse == CString(OSSTR("darkturquoise")))
		return OR<const CColor>(mDarkTurquoise);
	if (colorNameUse == CString(OSSTR("darkviolet")))
		return OR<const CColor>(mDarkViolet);
	if (colorNameUse == CString(OSSTR("deeppink")))
		return OR<const CColor>(mDeepPink);
	if (colorNameUse == CString(OSSTR("deepskyblue")))
		return OR<const CColor>(mDeepSkyBlue);
	if (colorNameUse == CString(OSSTR("dimgray")))
		return OR<const CColor>(mDimGray);
	if (colorNameUse == CString(OSSTR("dimgrey")))
		return OR<const CColor>(mDimGrey);
	if (colorNameUse == CString(OSSTR("dodgerblue")))
		return OR<const CColor>(mDodgerBlue);
	if (colorNameUse == CString(OSSTR("firebrick")))
		return OR<const CColor>(mFireBrick);
	if (colorNameUse == CString(OSSTR("floralwhite")))
		return OR<const CColor>(mFloralWhite);
	if (colorNameUse == CString(OSSTR("forestgreen")))
		return OR<const CColor>(mForestGreen);
	if (colorNameUse == CString(OSSTR("fuchsia")))
		return OR<const CColor>(mFuchsia);
	if (colorNameUse == CString(OSSTR("gainsboro")))
		return OR<const CColor>(mGainsboro);
	if (colorNameUse == CString(OSSTR("ghostwhite")))
		return OR<const CColor>(mGhostWhite);
	if (colorNameUse == CString(OSSTR("gold")))
		return OR<const CColor>(mGold);
	if (colorNameUse == CString(OSSTR("goldenrod")))
		return OR<const CColor>(mGoldenrod);
	if (colorNameUse == CString(OSSTR("gray")))
		return OR<const CColor>(mGray);
	if (colorNameUse == CString(OSSTR("grey")))
		return OR<const CColor>(mGrey);
	if (colorNameUse == CString(OSSTR("green")))
		return OR<const CColor>(mGreen);
	if (colorNameUse == CString(OSSTR("greenyellow")))
		return OR<const CColor>(mGreenYellow);
	if (colorNameUse == CString(OSSTR("honeydew")))
		return OR<const CColor>(mHoneydew);
	if (colorNameUse == CString(OSSTR("hotpink")))
		return OR<const CColor>(mHotPink);
	if (colorNameUse == CString(OSSTR("indianred")))
		return OR<const CColor>(mIndianRed);
	if (colorNameUse == CString(OSSTR("indigo")))
		return OR<const CColor>(mIndigo);
	if (colorNameUse == CString(OSSTR("ivory")))
		return OR<const CColor>(mIvory);
	if (colorNameUse == CString(OSSTR("khaki")))
		return OR<const CColor>(mKhaki);
	if (colorNameUse == CString(OSSTR("lavender")))
		return OR<const CColor>(mLavender);
	if (colorNameUse == CString(OSSTR("lavenderblush")))
		return OR<const CColor>(mLavenderBlush);
	if (colorNameUse == CString(OSSTR("lawngreen")))
		return OR<const CColor>(mLawnGreen);
	if (colorNameUse == CString(OSSTR("lemonchiffon")))
		return OR<const CColor>(mLemonChiffon);
	if (colorNameUse == CString(OSSTR("lightblue")))
		return OR<const CColor>(mLightBlue);
	if (colorNameUse == CString(OSSTR("lightcoral")))
		return OR<const CColor>(mLightCoral);
	if (colorNameUse == CString(OSSTR("lightcyan")))
		return OR<const CColor>(mLightCyan);
	if (colorNameUse == CString(OSSTR("lightgoldenrodyellow")))
		return OR<const CColor>(mLightGoldenrodYellow);
	if (colorNameUse == CString(OSSTR("lightgray")))
		return OR<const CColor>(mLightGray);
	if (colorNameUse == CString(OSSTR("lightgreen")))
		return OR<const CColor>(mLightGreen);
	if (colorNameUse == CString(OSSTR("lightgrey")))
		return OR<const CColor>(mLightGrey);
	if (colorNameUse == CString(OSSTR("lightpink")))
		return OR<const CColor>(mLightPink);
	if (colorNameUse == CString(OSSTR("lightsalmon")))
		return OR<const CColor>(mLighSalmon);
	if (colorNameUse == CString(OSSTR("lightseagreen")))
		return OR<const CColor>(mLightSeaGreen);
	if (colorNameUse == CString(OSSTR("lightskyblue")))
		return OR<const CColor>(mLightSkyBlue);
	if (colorNameUse == CString(OSSTR("lightslategray")))
		return OR<const CColor>(mLightSlateGray);
	if (colorNameUse == CString(OSSTR("lightslategrey")))
		return OR<const CColor>(mLightSlateGrey);
	if (colorNameUse == CString(OSSTR("lightsteelblue")))
		return OR<const CColor>(mLightSteelBlue);
	if (colorNameUse == CString(OSSTR("lightyellow")))
		return OR<const CColor>(mLightYellow);
	if (colorNameUse == CString(OSSTR("lime")))
		return OR<const CColor>(mLime);
	if (colorNameUse == CString(OSSTR("limegreen")))
		return OR<const CColor>(mLimeGreen);
	if (colorNameUse == CString(OSSTR("linen")))
		return OR<const CColor>(mLinen);
	if (colorNameUse == CString(OSSTR("magenta")))
		return OR<const CColor>(mMagenta);
	if (colorNameUse == CString(OSSTR("maroon")))
		return OR<const CColor>(mMaroon);
	if (colorNameUse == CString(OSSTR("mediumaquamarine")))
		return OR<const CColor>(mMediumAquamarine);
	if (colorNameUse == CString(OSSTR("mediumblue")))
		return OR<const CColor>(mMediumBlue);
	if (colorNameUse == CString(OSSTR("mediumorchid")))
		return OR<const CColor>(mMediumOrchid);
	if (colorNameUse == CString(OSSTR("mediumpurple")))
		return OR<const CColor>(mMediumPurple);
	if (colorNameUse == CString(OSSTR("mediumseagreen")))
		return OR<const CColor>(mMediumSeaGreen);
	if (colorNameUse == CString(OSSTR("mediumslateblue")))
		return OR<const CColor>(mMediumSlateBlue);
	if (colorNameUse == CString(OSSTR("mediumspringgreen")))
		return OR<const CColor>(mMediumSpringGreen);
	if (colorNameUse == CString(OSSTR("mediumturquoise")))
		return OR<const CColor>(mMediumTurquoise);
	if (colorNameUse == CString(OSSTR("mediumvioletred")))
		return OR<const CColor>(mMediumVioletRed);
	if (colorNameUse == CString(OSSTR("midnightblue")))
		return OR<const CColor>(mMidnightBlue);
	if (colorNameUse == CString(OSSTR("mintcream")))
		return OR<const CColor>(mMintCream);
	if (colorNameUse == CString(OSSTR("mistyrose")))
		return OR<const CColor>(mMistyRose);
	if (colorNameUse == CString(OSSTR("moccasin")))
		return OR<const CColor>(mMoccasin);
	if (colorNameUse == CString(OSSTR("navajowhite")))
		return OR<const CColor>(mNavajoWhite);
	if (colorNameUse == CString(OSSTR("navy")))
		return OR<const CColor>(mNavy);
	if (colorNameUse == CString(OSSTR("oldlace")))
		return OR<const CColor>(mOldLace);
	if (colorNameUse == CString(OSSTR("olive")))
		return OR<const CColor>(mOlive);
	if (colorNameUse == CString(OSSTR("olivedrab")))
		return OR<const CColor>(mOliveDrab);
	if (colorNameUse == CString(OSSTR("orange")))
		return OR<const CColor>(mOrange);
	if (colorNameUse == CString(OSSTR("orangered")))
		return OR<const CColor>(mOrangeRed);
	if (colorNameUse == CString(OSSTR("orchid")))
		return OR<const CColor>(mOrchid);
	if (colorNameUse == CString(OSSTR("palegoldenrod")))
		return OR<const CColor>(mPaleGoldenrod);
	if (colorNameUse == CString(OSSTR("palegreen")))
		return OR<const CColor>(mPaleGreen);
	if (colorNameUse == CString(OSSTR("paleturquoise")))
		return OR<const CColor>(mPaleTurquoise);
	if (colorNameUse == CString(OSSTR("palevioletred")))
		return OR<const CColor>(mPaleVioletRed);
	if (colorNameUse == CString(OSSTR("papayawhip")))
		return OR<const CColor>(mPapayaWhip);
	if (colorNameUse == CString(OSSTR("peachpuff")))
		return OR<const CColor>(mPeachPuff);
	if (colorNameUse == CString(OSSTR("peru")))
		return OR<const CColor>(mPeru);
	if (colorNameUse == CString(OSSTR("pink")))
		return OR<const CColor>(mPink);
	if (colorNameUse == CString(OSSTR("plum")))
		return OR<const CColor>(mPlum);
	if (colorNameUse == CString(OSSTR("powderblue")))
		return OR<const CColor>(mPowderBlue);
	if (colorNameUse == CString(OSSTR("purple")))
		return OR<const CColor>(mPurple);
	if (colorNameUse == CString(OSSTR("red")))
		return OR<const CColor>(mRed);
	if (colorNameUse == CString(OSSTR("rosybrown")))
		return OR<const CColor>(mRosyBrown);
	if (colorNameUse == CString(OSSTR("royalblue")))
		return OR<const CColor>(mRoyalBlue);
	if (colorNameUse == CString(OSSTR("saddlebrown")))
		return OR<const CColor>(mSadleBrown);
	if (colorNameUse == CString(OSSTR("salmon")))
		return OR<const CColor>(mSalmon);
	if (colorNameUse == CString(OSSTR("sandybrown")))
		return OR<const CColor>(mSandyBrown);
	if (colorNameUse == CString(OSSTR("seagreen")))
		return OR<const CColor>(mSeaGreen);
	if (colorNameUse == CString(OSSTR("seashell")))
		return OR<const CColor>(mSeashell);
	if (colorNameUse == CString(OSSTR("sienna")))
		return OR<const CColor>(mSienna);
	if (colorNameUse == CString(OSSTR("silver")))
		return OR<const CColor>(mSilver);
	if (colorNameUse == CString(OSSTR("skyblue")))
		return OR<const CColor>(mSkyBlue);
	if (colorNameUse == CString(OSSTR("slateblue")))
		return OR<const CColor>(mSlateBlue);
	if (colorNameUse == CString(OSSTR("slategray")))
		return OR<const CColor>(mSlateGray);
	if (colorNameUse == CString(OSSTR("slategrey")))
		return OR<const CColor>(mSlateGrey);
	if (colorNameUse == CString(OSSTR("snow")))
		return OR<const CColor>(mSnow);
	if (colorNameUse == CString(OSSTR("springgreen")))
		return OR<const CColor>(mSpringGreen);
	if (colorNameUse == CString(OSSTR("steelblue")))
		return OR<const CColor>(mSteelBlue);
	if (colorNameUse == CString(OSSTR("tan")))
		return OR<const CColor>(mTan);
	if (colorNameUse == CString(OSSTR("teal")))
		return OR<const CColor>(mTeal);
	if (colorNameUse == CString(OSSTR("thistle")))
		return OR<const CColor>(mThistle);
	if (colorNameUse == CString(OSSTR("tomato")))
		return OR<const CColor>(mTomato);
	if (colorNameUse == CString(OSSTR("turquoise")))
		return OR<const CColor>(mTurquoise);
	if (colorNameUse == CString(OSSTR("violet")))
		return OR<const CColor>(mViolet);
	if (colorNameUse == CString(OSSTR("wheat")))
		return OR<const CColor>(mWheat);
	if (colorNameUse == CString(OSSTR("white")))
		return OR<const CColor>(mWhite);
	if (colorNameUse == CString(OSSTR("whitesmoke")))
		return OR<const CColor>(mWhiteSmoke);
	if (colorNameUse == CString(OSSTR("yellow")))
		return OR<const CColor>(mYellow);
	if (colorNameUse == CString(OSSTR("yellowgreen")))
		return OR<const CColor>(mYellowGreen);

	return OR<const CColor>();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
void sConvertRGBToHSV(CColorInternals& internals)
//----------------------------------------------------------------------------------------------------------------------
{
	// Based on http://www.cs.rit.edu/~ncs/color/t_convert.html
	
	Float32	min =
					(internals.mR < internals.mG) ?
							((internals.mR < internals.mB) ? internals.mR : internals.mB) :
							((internals.mG < internals.mB) ? internals.mG : internals.mB);
	Float32	max =
					(internals.mR > internals.mG) ?
							((internals.mR > internals.mB) ? internals.mR : internals.mB) :
							((internals.mG > internals.mB) ? internals.mG : internals.mB);
	
	// Value
	internals.mV = max;
	
	// Any hue?
	if (min == max) {
		// No
		internals.mH = 0.0;
		internals.mS = 0.0;
		
		return;
	}
	
	Float32	delta = max - min;
	
	// Saturation
	internals.mS = delta / max;
	
	if (internals.mR == max)
		// between yellow and magenta
		internals.mH = (internals.mG - internals.mB) / delta;
	else if (internals.mG == max)
		// between cyan and yellow
		internals.mH = 2 + (internals.mB - internals.mR) / delta;
	else
		// between magenta & cyan
		internals.mH = 4 + (internals.mR - internals.mG) / delta;
	
	// degrees
	internals.mH *= 60.0;
	if (internals.mH < 0)
		internals.mH += 360.0;
}

//----------------------------------------------------------------------------------------------------------------------
void sConvertHSVToRGB(CColorInternals& internals)
//----------------------------------------------------------------------------------------------------------------------
{
	// From http://www.easyrgb.com/math.php?MATH=M21#text21
	
	if (internals.mS == 0.0) {
		// Achromatic (gray)
		internals.mR = internals.mG = internals.mB = internals.mV;
		
		return;
	}
	
	Float32	h = internals.mH / (Float32) 60.0;
	if (h == 6.0)
		h = 0.0;
	
	Float32	i = floorf(h);
	Float32	v1 = internals.mV * ((Float32) 1.0 - internals.mS);
	Float32	v2 = internals.mV * ((Float32) 1.0 - internals.mS * (h - i));
	Float32	v3 = internals.mV * ((Float32) 1.0 - internals.mS * ((Float32) 1.0 - (h - i)));
	
	if (i == 0.0) {
		internals.mR = internals.mV;
		internals.mG = v3;
		internals.mB = v1;
	} else if (i == 1.0) {
		internals.mR = v2;
		internals.mG = internals.mV;
		internals.mB = v1;
	} else if (i == 2.0) {
		internals.mR = v1;
		internals.mG = internals.mV;
		internals.mB = v3;
	} else if (i == 3.0) {
		internals.mR = v1;
		internals.mG = v2;
		internals.mB = internals.mV;
	} else if (i == 4.0) {
		internals.mR = v3;
		internals.mG = v1;
		internals.mB = internals.mV;
	} else {
		internals.mR = internals.mV;
		internals.mG = v1;
		internals.mB = v2;
	}
}
