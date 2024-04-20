//----------------------------------------------------------------------------------------------------------------------
//	CColor.cpp			©2005 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CColor.h"

#include "CReferenceCountable.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CColor::Internals

class CColor::Internals : public TReferenceCountableAutoDelete<CColor::Internals> {
	public:
		enum Type {
			kTypeRGB,
			kTypeHSV,
		};

					Internals(const RGBValues& rgbValues) :
						TReferenceCountableAutoDelete(),
								mType(kTypeRGB), _(rgbValues) {}
					Internals(const HSVValues& hsvValues) :
						TReferenceCountableAutoDelete(),
								mType(kTypeHSV), _(hsvValues) {}

		void		cleanup()
						{
							// Check type
							switch (mType) {
								case kTypeRGB:	Delete(_.mRGBValues);	break;
								case kTypeHSV:	Delete(_.mHSVValues);	break;
							}

							// Do super
							TReferenceCountableAutoDelete<CColor::Internals>::cleanup();
						}

		RGBValues	getRGBValues()
						{
							// Check type
							switch (mType) {
								case kTypeRGB:	return *_.mRGBValues;

								case kTypeHSV: {
									// HSV => RGB - from http://www.easyrgb.com/math.php?MATH=M21#text21
									const	HSVValues&	hsvValues = *_.mHSVValues;
											Float32		h = hsvValues.getHue() / (Float32) 60.0;
											Float32		s = hsvValues.getSaturation();
											Float32		v = hsvValues.getValue();

									if (s == 0.0)
										// Achromatic (gray)
										return RGBValues(v, v, v, hsvValues.getAlpha());
									else {
										// Have chroma
										if (h == 6.0)
											h = 0.0;

										Float32	i = floorf(h);
										Float32	v1 = v * ((Float32) 1.0 - s);
										Float32	v2 = v * ((Float32) 1.0 - s * (h - i));
										Float32	v3 = v * ((Float32) 1.0 - s * ((Float32) 1.0 - (h - i)));

										Float32	r, g, b;
										if (i == 0.0) {
											r = v;
											g = v3;
											b = v1;
										} else if (i == 1.0) {
											r = v2;
											g = v;
											b = v1;
										} else if (i == 2.0) {
											r = v1;
											g = v;
											b = v3;
										} else if (i == 3.0) {
											r = v1;
											g = v2;
											b = v;
										} else if (i == 4.0) {
											r = v3;
											g = v1;
											b = v;
										} else {
											r = v;
											g = v1;
											b = v2;
										}

										return RGBValues(r, g, b, hsvValues.getAlpha());
									}
								}

#if defined(TARGET_OS_WINDOWS)
								default:		return *_.mRGBValues;
#endif
							}
						}
		HSVValues	getHSVValues()
						{
							// Check type
							switch (mType) {
								case kTypeRGB: {
									// RGB => HSV - based on http://www.cs.rit.edu/~ncs/color/t_convert.html
									const	RGBValues&	rgbValues = *_.mRGBValues;
											Float32		r = rgbValues.getRed();
											Float32		g = rgbValues.getGreen();
											Float32		b = rgbValues.getBlue();
											Float32		min = (r < g) ? ((r < b) ? r : b) : ((g < b) ? g : b);
											Float32		max = (r > g) ? ((r > b) ? r : b) : ((g > b) ? g : b);

									// Value
									Float32	v = max;

									// Any hue?
									Float32	h, s;
									if (min == max) {
										// No
										h = 0.0;
										s = 0.0;
									} else {
										// Yes
										Float32	delta = max - min;

										// Saturation
										s = delta / max;

										if (r == max)
											// between yellow and magenta
											h = (g - b) / delta;
										else if (g == max)
											// between cyan and yellow
											h = 2 + (b - r) / delta;
										else
											// between magenta & cyan
											h = 4 + (r - g) / delta;

										// degrees
										h *= 60.0;
										if (h < 0.0)
											h += 360.0;
									}

									return HSVValues(h, s, v, rgbValues.getAlpha());
								}

								case kTypeHSV:	return *_.mHSVValues;

#if defined(TARGET_OS_WINDOWS)
								default:		return *_.mHSVValues;
#endif
							}
						}

		Type	mType;
		union _ {
			_(const RGBValues& rgbValues) : mRGBValues(new RGBValues(rgbValues)) {}
			_(const HSVValues& hsvValues) : mHSVValues(new HSVValues(hsvValues)) {}

			RGBValues*	mRGBValues;
			HSVValues*	mHSVValues;
		} _;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CColor

// MARK: Properties

const	CColor	CColor::mClear(RGBValues((UInt8) 0, 0, 0, 0));
const	CColor	CColor::mAliceBlue(RGBValues((UInt8) 240, 248, 255));
const	CColor	CColor::mAntiqueWhite(RGBValues((UInt8) 250, 235, 215));
const	CColor	CColor::mAqua(RGBValues((UInt8) 0, 255, 255));
const	CColor	CColor::mAquamarine(RGBValues((UInt8) 127, 255, 212));
const	CColor	CColor::mAzure(RGBValues((UInt8) 240, 255, 255));
const	CColor	CColor::mBeige(RGBValues((UInt8) 245, 245, 220));
const	CColor	CColor::mBisque(RGBValues((UInt8) 255, 228, 196));
const	CColor	CColor::mBlack(RGBValues((UInt8) 0, 0, 0));
const	CColor	CColor::mBlanchedAlmond(RGBValues((UInt8) 255, 235, 205));
const	CColor	CColor::mBlue(RGBValues((UInt8) 0, 0, 255));
const	CColor	CColor::mBlueViolet(RGBValues((UInt8) 138, 43, 226));
const	CColor	CColor::mBrown(RGBValues((UInt8) 165, 42, 42));
const	CColor	CColor::mBurlywood(RGBValues((UInt8) 222, 184, 135));
const	CColor	CColor::mCadetBlue(RGBValues((UInt8) 95, 158, 160));
const	CColor	CColor::mChartreuse(RGBValues((UInt8) 127, 255, 0));
const	CColor	CColor::mChocolate(RGBValues((UInt8) 210, 105, 30));
const	CColor	CColor::mCoral(RGBValues((UInt8) 255, 127, 80));
const	CColor	CColor::mCornflowerBlue(RGBValues((UInt8) 100, 149, 237));
const	CColor	CColor::mCornSilk(RGBValues((UInt8) 255, 248, 220));
const	CColor	CColor::mCrimson(RGBValues((UInt8) 220, 20, 60));
const	CColor	CColor::mCyan(RGBValues((UInt8) 0, 255, 255));
const	CColor	CColor::mDarkBlue(RGBValues((UInt8) 0, 0, 139));
const	CColor	CColor::mDarkCyan(RGBValues((UInt8) 0, 139, 139));
const	CColor	CColor::mDarkGoldenrod(RGBValues((UInt8) 184, 134, 11));
const	CColor	CColor::mDarkGray(RGBValues((UInt8) 169, 169, 169));
const	CColor	CColor::mDarkGreen(RGBValues((UInt8) 0, 100, 0));
const	CColor	CColor::mDarkGrey(RGBValues((UInt8) 169, 169, 169));
const	CColor	CColor::mDarkKhaki(RGBValues((UInt8) 189, 183, 107));
const	CColor	CColor::mDarkMagenta(RGBValues((UInt8) 139, 0, 139));
const	CColor	CColor::mDarkOliveGreen(RGBValues((UInt8) 85, 107, 47));
const	CColor	CColor::mDarkOrange(RGBValues((UInt8) 255, 140, 0));
const	CColor	CColor::mDarkOrchid(RGBValues((UInt8) 153, 50, 204));
const	CColor	CColor::mDarkdRed(RGBValues((UInt8) 139, 0, 0));
const	CColor	CColor::mDarkSalmon(RGBValues((UInt8) 233, 150, 122));
const	CColor	CColor::mDarkSeaGreen(RGBValues((UInt8) 143, 188, 143));
const	CColor	CColor::mDarkSlateBlue(RGBValues((UInt8) 72, 61, 139));
const	CColor	CColor::mDarkSlateGray(RGBValues((UInt8) 47, 79, 79));
const	CColor	CColor::mDarkSlateGrey(RGBValues((UInt8) 47, 79, 79));
const	CColor	CColor::mDarkTurquoise(RGBValues((UInt8) 0, 206, 209));
const	CColor	CColor::mDarkViolet(RGBValues((UInt8) 148, 0, 211));
const	CColor	CColor::mDeepPink(RGBValues((UInt8) 255, 20, 147));
const	CColor	CColor::mDeepSkyBlue(RGBValues((UInt8) 0, 191, 255));
const	CColor	CColor::mDimGray(RGBValues((UInt8) 105, 105, 105));
const	CColor	CColor::mDimGrey(RGBValues((UInt8) 105, 105, 105));
const	CColor	CColor::mDodgerBlue(RGBValues((UInt8) 30, 144, 255));
const	CColor	CColor::mFireBrick(RGBValues((UInt8) 178, 34, 34));
const	CColor	CColor::mFloralWhite(RGBValues((UInt8) 255, 250, 240));
const	CColor	CColor::mForestGreen(RGBValues((UInt8) 34, 139, 34));
const	CColor	CColor::mFuchsia(RGBValues((UInt8) 255, 0, 255));
const	CColor	CColor::mGainsboro(RGBValues((UInt8) 220, 220, 220));
const	CColor	CColor::mGhostWhite(RGBValues((UInt8) 248, 248, 255));
const	CColor	CColor::mGold(RGBValues((UInt8) 255, 215, 0));
const	CColor	CColor::mGoldenrod(RGBValues((UInt8) 218, 165, 32));
const	CColor	CColor::mGray(RGBValues((UInt8) 128, 128, 128));
const	CColor	CColor::mGrey(RGBValues((UInt8) 128, 128, 128));
const	CColor	CColor::mGreen(RGBValues((UInt8) 0, 128, 0));
const	CColor	CColor::mGreenYellow(RGBValues((UInt8) 173, 255, 47));
const	CColor	CColor::mHoneydew(RGBValues((UInt8) 240, 255, 240));
const	CColor	CColor::mHotPink(RGBValues((UInt8) 255, 105, 180));
const	CColor	CColor::mIndianRed(RGBValues((UInt8) 205, 92, 92));
const	CColor	CColor::mIndigo(RGBValues((UInt8) 75, 0, 130));
const	CColor	CColor::mIvory(RGBValues((UInt8) 255, 255, 240));
const	CColor	CColor::mKhaki(RGBValues((UInt8) 240, 230, 140));
const	CColor	CColor::mLavender(RGBValues((UInt8) 230, 230, 250));
const	CColor	CColor::mLavenderBlush(RGBValues((UInt8) 255, 240, 245));
const	CColor	CColor::mLawnGreen(RGBValues((UInt8) 124, 252, 0));
const	CColor	CColor::mLemonChiffon(RGBValues((UInt8) 255, 250, 205));
const	CColor	CColor::mLightBlue(RGBValues((UInt8) 173, 216, 230));
const	CColor	CColor::mLightCoral(RGBValues((UInt8) 240, 128, 128));
const	CColor	CColor::mLightCyan(RGBValues((UInt8) 224, 255, 255));
const	CColor	CColor::mLightGoldenrodYellow(RGBValues((UInt8) 250, 250, 210));
const	CColor	CColor::mLightGray(RGBValues((UInt8) 211, 211, 211));
const	CColor	CColor::mLightGreen(RGBValues((UInt8) 144, 238, 144));
const	CColor	CColor::mLightGrey(RGBValues((UInt8) 211, 211, 211));
const	CColor	CColor::mLightPink(RGBValues((UInt8) 255, 182, 193));
const	CColor	CColor::mLighSalmon(RGBValues((UInt8) 255, 160, 122));
const	CColor	CColor::mLightSeaGreen(RGBValues((UInt8) 32, 178, 170));
const	CColor	CColor::mLightSkyBlue(RGBValues((UInt8) 135, 206, 250));
const	CColor	CColor::mLightSlateGray(RGBValues((UInt8) 119, 136, 153));
const	CColor	CColor::mLightSlateGrey(RGBValues((UInt8) 119, 136, 153));
const	CColor	CColor::mLightSteelBlue(RGBValues((UInt8) 176, 196, 222));
const	CColor	CColor::mLightYellow(RGBValues((UInt8) 255, 255, 224));
const	CColor	CColor::mLime(RGBValues((UInt8) 0, 255, 0));
const	CColor	CColor::mLimeGreen(RGBValues((UInt8) 50, 205, 50));
const	CColor	CColor::mLinen(RGBValues((UInt8) 250, 240, 230));
const	CColor	CColor::mMagenta(RGBValues((UInt8) 255, 0, 255));
const	CColor	CColor::mMaroon(RGBValues((UInt8) 128, 0, 0));
const	CColor	CColor::mMediumAquamarine(RGBValues((UInt8) 102, 205, 170));
const	CColor	CColor::mMediumBlue(RGBValues((UInt8) 0, 0, 205));
const	CColor	CColor::mMediumOrchid(RGBValues((UInt8) 186, 85, 211));
const	CColor	CColor::mMediumPurple(RGBValues((UInt8) 147, 112, 219));
const	CColor	CColor::mMediumSeaGreen(RGBValues((UInt8) 60, 179, 113));
const	CColor	CColor::mMediumSlateBlue(RGBValues((UInt8) 123, 104, 238));
const	CColor	CColor::mMediumSpringGreen(RGBValues((UInt8) 0, 250, 154));
const	CColor	CColor::mMediumTurquoise(RGBValues((UInt8) 72, 209, 204));
const	CColor	CColor::mMediumVioletRed(RGBValues((UInt8) 199, 21, 133));
const	CColor	CColor::mMidnightBlue(RGBValues((UInt8) 25, 25, 112));
const	CColor	CColor::mMintCream(RGBValues((UInt8) 245, 255, 250));
const	CColor	CColor::mMistyRose(RGBValues((UInt8) 255, 228, 225));
const	CColor	CColor::mMoccasin(RGBValues((UInt8) 255, 228, 181));
const	CColor	CColor::mNavajoWhite(RGBValues((UInt8) 255, 222, 173));
const	CColor	CColor::mNavy(RGBValues((UInt8) 0, 0, 128));
const	CColor	CColor::mOldLace(RGBValues((UInt8) 253, 245, 230));
const	CColor	CColor::mOlive(RGBValues((UInt8) 128, 128, 0));
const	CColor	CColor::mOliveDrab(RGBValues((UInt8) 107, 142, 35));
const	CColor	CColor::mOrange(RGBValues((UInt8) 255, 165, 0));
const	CColor	CColor::mOrangeRed(RGBValues((UInt8) 255, 69, 0));
const	CColor	CColor::mOrchid(RGBValues((UInt8) 218, 112, 214));
const	CColor	CColor::mPaleGoldenrod(RGBValues((UInt8) 238, 232, 170));
const	CColor	CColor::mPaleGreen(RGBValues((UInt8) 152, 251, 152));
const	CColor	CColor::mPaleTurquoise(RGBValues((UInt8) 175, 238, 238));
const	CColor	CColor::mPaleVioletRed(RGBValues((UInt8) 219, 112, 147));
const	CColor	CColor::mPapayaWhip(RGBValues((UInt8) 255, 239, 213));
const	CColor	CColor::mPeachPuff(RGBValues((UInt8) 255, 218, 185));
const	CColor	CColor::mPeru(RGBValues((UInt8) 205, 133, 63));
const	CColor	CColor::mPink(RGBValues((UInt8) 255, 192, 203));
const	CColor	CColor::mPlum(RGBValues((UInt8) 221, 160, 221));
const	CColor	CColor::mPowderBlue(RGBValues((UInt8) 176, 224, 230));
const	CColor	CColor::mPurple(RGBValues((UInt8) 128, 0, 128));
const	CColor	CColor::mRed(RGBValues((UInt8) 255, 0, 0));
const	CColor	CColor::mRosyBrown(RGBValues((UInt8) 188, 143, 143));
const	CColor	CColor::mRoyalBlue(RGBValues((UInt8) 65, 105, 225));
const	CColor	CColor::mSadleBrown(RGBValues((UInt8) 139, 69, 19));
const	CColor	CColor::mSalmon(RGBValues((UInt8) 250, 128, 114));
const	CColor	CColor::mSandyBrown(RGBValues((UInt8) 244, 164, 96));
const	CColor	CColor::mSeaGreen(RGBValues((UInt8) 46, 139, 87));
const	CColor	CColor::mSeashell(RGBValues((UInt8) 255, 245, 238));
const	CColor	CColor::mSienna(RGBValues((UInt8) 160, 82, 45));
const	CColor	CColor::mSilver(RGBValues((UInt8) 192, 192, 192));
const	CColor	CColor::mSkyBlue(RGBValues((UInt8) 135, 206, 235));
const	CColor	CColor::mSlateBlue(RGBValues((UInt8) 106, 90, 205));
const	CColor	CColor::mSlateGray(RGBValues((UInt8) 112, 128, 144));
const	CColor	CColor::mSlateGrey(RGBValues((UInt8) 112, 128, 144));
const	CColor	CColor::mSnow(RGBValues((UInt8) 255, 250, 250));
const	CColor	CColor::mSpringGreen(RGBValues((UInt8) 0, 255, 127));
const	CColor	CColor::mSteelBlue(RGBValues((UInt8) 70, 130, 180));
const	CColor	CColor::mTan(RGBValues((UInt8) 210, 180, 140));
const	CColor	CColor::mTeal(RGBValues((UInt8) 0, 128, 128));
const	CColor	CColor::mThistle(RGBValues((UInt8) 216, 191, 216));
const	CColor	CColor::mTomato(RGBValues((UInt8) 255, 99, 71));
const	CColor	CColor::mTurquoise(RGBValues((UInt8) 64, 224, 208));
const	CColor	CColor::mViolet(RGBValues((UInt8) 238, 130, 238));
const	CColor	CColor::mWheat(RGBValues((UInt8) 245, 222, 179));
const	CColor	CColor::mWhite(RGBValues((UInt8) 255, 255, 255));
const	CColor	CColor::mWhiteSmoke(RGBValues((UInt8) 245, 245, 245));
const	CColor	CColor::mYellow(RGBValues((UInt8) 255, 255, 0));
const	CColor	CColor::mYellowGreen(RGBValues((UInt8) 154, 205, 50));

const	SMatrix3x3_32	CColor::mYCbCrConverstionMatrixRec601VideoRange(
								1.164F,  1.164F, 1.164F,
								0.000F, -0.392F, 2.017F,
								1.596F, -0.813F, 0.000F);
const	SMatrix3x3_32	CColor::mYCbCrConverstionMatrixRec601FullRange(
								1.000F,  1.000F, 1.000F,
								0.000F, -0.343F, 1.765F,
								1.400F, -0.711F, 0.000F);
const	SMatrix3x3_32	CColor::mYCbCrConverstionMatrixRec709VideoRange(
								1.164F,  1.164F, 1.164F,
								0.000F, -0.213F, 2.112F,
								1.793F, -0.533F, 0.000F);

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CColor::CColor(const RGBValues& rgbValues)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(rgbValues);
}

//----------------------------------------------------------------------------------------------------------------------
CColor::CColor(const HSVValues& hsvValues)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(hsvValues);
}

//----------------------------------------------------------------------------------------------------------------------
CColor::CColor(const CString& rgbHexString)
//----------------------------------------------------------------------------------------------------------------------
{
	// rgbHexString can be of the form:
	//	#RGB
	//	#RRGGBB
	//	#RRGGBBAA
	//	RGB
	//	RRGGBB
	//	RRGGBBAA
	CString::CharIndex	startIndex = rgbHexString.hasPrefix(CString(OSSTR("#"))) ? 1 : 0;

	if ((rgbHexString.getLength() - startIndex) == 3) {
		// RGB
		UInt8	value;
		
		value = rgbHexString.getSubString(startIndex + 0, 1).getUInt8(16);
		Float32	r = ((Float32) value * (Float32) 16.0 + (Float32) value) / (Float32) 255.0;
		
		value = rgbHexString.getSubString(startIndex + 1, 1).getUInt8(16);
		Float32	g = ((Float32) value * (Float32) 16.0 + (Float32) value) / (Float32) 255.0;
		
		value = rgbHexString.getSubString(startIndex + 2, 1).getUInt8(16);
		Float32	b = ((Float32) value * (Float32) 16.0 + (Float32) value) / (Float32) 255.0;

		mInternals = new Internals(RGBValues(r, g, b));
	} else if ((rgbHexString.getLength() - startIndex) == 6) {
		// RRGGBB
		Float32	r = (Float32) rgbHexString.getSubString(startIndex + 0, 2).getUInt8(16) / (Float32) 255.0;
		Float32	g = (Float32) rgbHexString.getSubString(startIndex + 2, 2).getUInt8(16) / (Float32) 255.0;
		Float32	b = (Float32) rgbHexString.getSubString(startIndex + 4, 2).getUInt8(16) / (Float32) 255.0;

		mInternals = new Internals(RGBValues(r, g, b));

	} else {
		// RRGGBBAA
		Float32	r = (Float32) rgbHexString.getSubString(startIndex + 0, 2).getUInt8(16) / (Float32) 255.0;
		Float32	g = (Float32) rgbHexString.getSubString(startIndex + 2, 2).getUInt8(16) / (Float32) 255.0;
		Float32	b = (Float32) rgbHexString.getSubString(startIndex + 4, 2).getUInt8(16) / (Float32) 255.0;
		Float32	a = (Float32) rgbHexString.getSubString(startIndex + 6, 2).getUInt8(16) / (Float32) 255.0;

		mInternals = new Internals(RGBValues(r, g, b, a));
	}
}

//----------------------------------------------------------------------------------------------------------------------
CColor::CColor(const CDictionary& info)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get type
	Internals::Type	type = (Internals::Type) info.getUInt8(CString(OSSTR("type")));
	if (type == Internals::kTypeRGB)
		// RGB
		mInternals =
				new Internals(
						RGBValues(info.getFloat32(CString(OSSTR("r"))), info.getFloat32(CString(OSSTR("g"))),
								info.getFloat32(CString(OSSTR("b"))), info.getFloat32(CString(OSSTR("a")))));
	else
		// HSV
		mInternals =
				new Internals(
						HSVValues(info.getFloat32(CString(OSSTR("h"))), info.getFloat32(CString(OSSTR("s"))),
								info.getFloat32(CString(OSSTR("v"))), info.getFloat32(CString(OSSTR("a")))));
}

//----------------------------------------------------------------------------------------------------------------------
CColor::CColor(const CColor& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CColor::~CColor()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: CEquatable methods

//----------------------------------------------------------------------------------------------------------------------
bool CColor::operator==(const CEquatable& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	CColor&	otherColor = (const CColor&) other;

	// Check type
	if ((mInternals->mType == Internals::kTypeRGB) && (otherColor.mInternals->mType == Internals::kTypeRGB))
		// Both RGB (most likely path)
		return *mInternals->_.mRGBValues == *otherColor.mInternals->_.mRGBValues;
	else if ((mInternals->mType == Internals::kTypeHSV) && (otherColor.mInternals->mType == Internals::kTypeHSV))
		// Both HSV (only other path not requiring conversion)
		return *mInternals->_.mHSVValues == *otherColor.mInternals->_.mHSVValues;
	else
		// For now, just do RGB comparison
		return mInternals->getRGBValues() == otherColor.mInternals->getRGBValues();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CColor::RGBValues CColor::getRGBValues() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getRGBValues();
}

//----------------------------------------------------------------------------------------------------------------------
CColor::HSVValues CColor::getHSVValues() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getHSVValues();
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary CColor::getInfo() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CDictionary	info;

	// Store
	info.set(CString(OSSTR("type")), (UInt8) mInternals->mType);
	if (mInternals->mType == Internals::kTypeRGB) {
		// RGB
		info.set(CString(OSSTR("r")), mInternals->_.mRGBValues->getRed());
		info.set(CString(OSSTR("g")), mInternals->_.mRGBValues->getGreen());
		info.set(CString(OSSTR("b")), mInternals->_.mRGBValues->getBlue());
		info.set(CString(OSSTR("a")), mInternals->_.mRGBValues->getAlpha());
	} else {
		// HSV
		info.set(CString(OSSTR("h")), mInternals->_.mHSVValues->getHue());
		info.set(CString(OSSTR("s")), mInternals->_.mHSVValues->getSaturation());
		info.set(CString(OSSTR("v")), mInternals->_.mHSVValues->getValue());
		info.set(CString(OSSTR("a")), mInternals->_.mHSVValues->getAlpha());
	}

	return info;
}

//----------------------------------------------------------------------------------------------------------------------
CColor& CColor::operator=(const CColor& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if assignment to self
	if (this == &other)
		return *this;

	// Remove reference to ourselves
	mInternals->removeReference();

	// Add reference to other
	mInternals = other.mInternals->addReference();

	return *this;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
OR<const CColor> CColor::colorForName(const CString& colorName)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check colorname
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
