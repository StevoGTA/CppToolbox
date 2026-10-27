//----------------------------------------------------------------------------------------------------------------------
//	STimecode.cpp			©2023 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "STimecode.h"

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: STimecode::FrameRate

// MARK: Properties

const	STimecode::FrameRate	STimecode::FrameRate::_23_976(24000, 1001, false);
const	STimecode::FrameRate	STimecode::FrameRate::_24(24.0);
const	STimecode::FrameRate	STimecode::FrameRate::_25(25.0);
const	STimecode::FrameRate	STimecode::FrameRate::_29_97(30000, 1001, false);
const	STimecode::FrameRate	STimecode::FrameRate::_29_97DropFrame(30000, 1001, true);
const	STimecode::FrameRate	STimecode::FrameRate::_30(30.0);
const	STimecode::FrameRate	STimecode::FrameRate::_30DropFrame(30.0, true);
const	STimecode::FrameRate	STimecode::FrameRate::_47_952(48000, 1001, false);
const	STimecode::FrameRate	STimecode::FrameRate::_48(48.0);
const	STimecode::FrameRate	STimecode::FrameRate::_50(50.0);
const	STimecode::FrameRate	STimecode::FrameRate::_59_94(60000, 1001, false);
const	STimecode::FrameRate	STimecode::FrameRate::_59_94DropFrame(60000, 1001, true);
const	STimecode::FrameRate	STimecode::FrameRate::_60(60.0);

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
STimecode::FrameRate::FrameRate(UInt32 numerator, UInt32 denominator, bool isDropFrame) :
		mFrameRate((Float64) numerator / (Float64) denominator), mIsDropFrame(isDropFrame)
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
STimecode::FrameRate::FrameRate(const CDictionary& storageInfo) :
		mFrameRate(storageInfo.getFloat64(CString(OSSTR("frameRate")))),
		mIsDropFrame(storageInfo.getBool(CString(OSSTR("isDropFrame"))))
//----------------------------------------------------------------------------------------------------------------------
{
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CString STimecode::FrameRate::getDisplayString() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt32	frameRateThousandths = (UInt32) ::round(mFrameRate * 1000.0);
	UInt32	digitsAfterDecimalPoint;
	if ((frameRateThousandths % 10) != 0)
		// 3 decimal places
		digitsAfterDecimalPoint = 3;
	else if ((frameRateThousandths % 100) != 0)
		// 2 decimal places
		digitsAfterDecimalPoint = 2;
	else if ((frameRateThousandths % 1000) != 0)
		// 1 decimal place
		digitsAfterDecimalPoint = 1;
	else
		// No decimal places
		digitsAfterDecimalPoint = 0;
	CString	frameRateString(mFrameRate, 0, digitsAfterDecimalPoint);

	return mIsDropFrame ? frameRateString + CString(OSSTR(" DF")) : frameRateString;
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary STimecode::FrameRate::getStorageInfo() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose Storage info
	CDictionary	storageInfo;
	storageInfo.set(CString(OSSTR("frameRate")), mFrameRate);
	storageInfo.set(CString(OSSTR("isDropFrame")), mIsDropFrame);

	return storageInfo;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
const TArray<STimecode::FrameRate>& STimecode::FrameRate::getStandard()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	TSArray<FrameRate>*	sArray = nil;
	if (sArray == nil) {
		// Setup
		static	FrameRate	sFrameRates[] =
									{
										_23_976,
										_24,
										_25,
										_29_97,
										_29_97DropFrame,
										_30,
										_30DropFrame,
										_47_952,
										_48,
										_50,
										_59_94,
										_59_94DropFrame,
										_60,
									};
		sArray = new TSARRAY_FROM_C_ARRAY(FrameRate, sFrameRates);
	}

	return *sArray;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - STimecode::HMSF

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CString STimecode::HMSF::getDisplayString() const
//----------------------------------------------------------------------------------------------------------------------
{
	return CString::make(isDropFrame() ? OSSTR("%02d:%02d:%02d;%02d") : OSSTR("%02d:%02d:%02d:%02d"), getHours(),
			getMinutes(), getSeconds(), getFrames());
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - STimecode

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
STimecode::HMSF STimecode::getHMSF(UniversalTimeInterval timeInterval, const FrameRate& frameRate)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SInt32	base = (SInt32) frameRate.getBase();
	SInt32	frameIndex = (SInt32) ::round(timeInterval * frameRate.getFrameRate());

	// Check if drop frame
	if (frameRate.isDropFrame()) {
		// Drop Frame
		// From https://www.davidheidelberger.com/2010/06/10/drop-frame-timecode/
		SInt32	dropFrames = (SInt32) ::round(frameRate.getFrameRate() * 0.066666);
		SInt32	framesPer24Hours = (SInt32) ::round(frameRate.getFrameRate() * 60.0 * 60.0) * 24;
		SInt32	framesPer10Minutes = (SInt32) ::round(frameRate.getFrameRate() * 60.0 * 10.0);
		SInt32	framesPerMinute = base * 60 - dropFrames;

		// Get frameIndex as a positive number within 0 - 24 hours
		while (frameIndex < 0) { frameIndex += framesPer24Hours; }
		frameIndex %= framesPer24Hours;

		// Update frameIndex for dropped frames
		SInt32	d = frameIndex / framesPer10Minutes;
		SInt32	m = frameIndex % framesPer10Minutes;
		frameIndex +=
				(m > dropFrames) ?
						(dropFrames * 9 * d) + dropFrames * ((m - dropFrames) / framesPerMinute) : dropFrames * 9 * d;
	}

	return HMSF(frameIndex / (60 * 60 * base), (frameIndex / (60 * base)) % 60, (frameIndex / base) % 60,
			frameIndex % base, frameRate.isDropFrame());
}

//----------------------------------------------------------------------------------------------------------------------
STimecode::HMSF STimecode::getEquivalentHMSF(const HMSF& hmsf, const FrameRate& frameRate)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup - a Drop Frame label that is dropped moves forward to the first valid label
	SInt32	dropFrames = (SInt32) ::round(frameRate.getFrameRate() / 15.0);
	SInt32	frames =
					(hmsf.isDropFrame() && (hmsf.getSeconds() == 0) && (hmsf.getFrames() < dropFrames) &&
									((hmsf.getMinutes() % 10) != 0)) ?
							dropFrames : hmsf.getFrames();

	return getHMSF(
			getTimeInterval(
					HMSF(hmsf.getHours(), hmsf.getMinutes(), hmsf.getSeconds(), frames, hmsf.isDropFrame()), frameRate),
			frameRate);
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTimeInterval STimecode::getTimeInterval(const HMSF& hmsf, const FrameRate& frameRate)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SInt32	totalMinutes = hmsf.getHours() * 60 + hmsf.getMinutes();
	SInt32	frameIndex = (totalMinutes * 60 + hmsf.getSeconds()) * (SInt32) frameRate.getBase() + hmsf.getFrames();

	// Check if drop frame
	if (hmsf.isDropFrame())
		// Drop Frame
		// From https://www.davidheidelberger.com/2010/06/10/drop-frame-timecode/
		frameIndex -= (SInt32) ::round(frameRate.getFrameRate() / 15.0) * (totalMinutes - (totalMinutes / 10));

	return (UniversalTimeInterval) frameIndex / frameRate.getFrameRate();
}

//----------------------------------------------------------------------------------------------------------------------
CString STimecode::getDisplayString(UniversalTimeInterval timeInterval, const FrameRate& frameRate)
//----------------------------------------------------------------------------------------------------------------------
{
	return getHMSF(timeInterval, frameRate).getDisplayString();
}
