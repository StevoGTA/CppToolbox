//----------------------------------------------------------------------------------------------------------------------
//	CTimeInfo.cpp			©2005 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CTimeInfo.h"

#include "CppToolboxAssert.h"
//#include "CDataStore.h"
#include "CLogServices.h"

#if (TARGET_OS_MACOS && !TARGET_LINUX_SIMULATOR) || TARGET_OS_IOS
	#include "CFUtilities.h"
#endif

#if TARGET_OS_LINUX
	#include <time.h>
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

enum {
	kCTimeInfoUsingTypeSamples,
	kCTimeInfoUsingTypeFrames,
	kCTimeInfoUsingTypeSeconds
};

static	CString					sUsingTypeKey("usingType");
static	CString					sSecondCountKey("secondCount");
static	CString					sSampleRateKey("sampleRate");
static	CString					sSampleCountKey("sampleCount");
static	CString					sFrameRateKey("frameRate");
static	CString					sFrameCountKey("frameCount");
static	UniversalTimeInterval	sOffsetInterval = 0.0;

const	CTimeInfo	CTimeInfo::mOneSecond(1.0);
const	CTimeInfo	CTimeInfo::mOneMinute(60.0);
const	CTimeInfo	CTimeInfo::mOneHour(60.0 * 60.0);
const	CTimeInfo	CTimeInfo::mZero;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

#if TARGET_OS_MACOS || TARGET_OS_IOS
CFDateFormatterStyle	sGetCFDateFormatterStyleForCTimeInfoStringStyle(ETimeInfoStringStyle format);
#endif

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CTimeInfoInternals

class CTimeInfoInternals {
	public:
		CTimeInfoInternals()
			{
				mUsingType = kCTimeInfoUsingTypeSeconds;
				mSampleRate = 0.0;
				mSampleCount = 0;
				mFrameRate = 0.0;
				mFrameCount = 0;
				mSecondCount = 0.0;
			}
		~CTimeInfoInternals() {}

		// General data
		UInt8	mUsingType;

		// Sample-based data
		Float32	mSampleRate;
		SInt64	mSampleCount;

		// Frame-based data
		Float32	mFrameRate;
		SInt64	mFrameCount;

		// Second-based data
		Float32	mSecondCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CTimeInfo

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CTimeInfo::CTimeInfo(Float32 secondCount)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CTimeInfoInternals();

	mInternals->mUsingType = kCTimeInfoUsingTypeSeconds;
	mInternals->mSecondCount = secondCount;
}

//----------------------------------------------------------------------------------------------------------------------
CTimeInfo::CTimeInfo(UInt32 flags, Float32 rate, SInt64 count)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CTimeInfoInternals();

	if (flags & kTimeInfoTimeFlagsUseSamples) {
		// Samples
		mInternals->mUsingType = kCTimeInfoUsingTypeSamples;
		mInternals->mSampleRate = rate;
		mInternals->mSampleCount = count;
	} else if (flags & kTimeInfoTimeFlagsUseFrames) {
		// Frames
		mInternals->mUsingType = kCTimeInfoUsingTypeFrames;
		mInternals->mFrameRate = rate;
		mInternals->mFrameCount = count;
	} else {
		// Default to seconds
		mInternals->mUsingType = kCTimeInfoUsingTypeSeconds;
		mInternals->mSecondCount = 0.0;
	}
}

//----------------------------------------------------------------------------------------------------------------------
CTimeInfo::CTimeInfo(const CDictionary& info)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CTimeInfoInternals();

	// Get Using Type
	mInternals->mUsingType = info.getUInt8(sUsingTypeKey, kCTimeInfoUsingTypeSeconds);

	// Get Sample Rate
	mInternals->mSampleRate = info.getFloat32(sSampleRateKey, 0.0);

	// Get Frame Rate
	mInternals->mFrameRate = info.getFloat32(sFrameRateKey, 0.0);

	switch (mInternals->mUsingType) {
		case kCTimeInfoUsingTypeSamples:
			// Set Sample Count
			mInternals->mSampleCount = info.getSInt64(sSampleCountKey, 0);
			mInternals->mFrameCount = 0;
			mInternals->mSecondCount = 0.0;
			break;

		case kCTimeInfoUsingTypeFrames:
			// Set Frame Count
			mInternals->mFrameCount = info.getSInt64(sFrameCountKey, 0);
			mInternals->mSampleCount = 0;
			mInternals->mSecondCount = 0.0;
			break;

		case kCTimeInfoUsingTypeSeconds:
			// Set Second Count
			mInternals->mSecondCount = info.getFloat32(sSecondCountKey, 0.0);
			mInternals->mSampleCount = 0;
			mInternals->mFrameCount = 0;
			break;
	}
}

//----------------------------------------------------------------------------------------------------------------------
CTimeInfo::CTimeInfo(const CTimeInfo& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CTimeInfoInternals();

	*this = other;
}

//----------------------------------------------------------------------------------------------------------------------
CTimeInfo::~CTimeInfo()
//----------------------------------------------------------------------------------------------------------------------
{
	DisposeOf(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CTimeInfo::setSecondCount(Float32 secondCount)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mUsingType = kCTimeInfoUsingTypeSeconds;
	mInternals->mSecondCount = secondCount;
}

//----------------------------------------------------------------------------------------------------------------------
void CTimeInfo::addSecondCount(Float32 secondCount)
//----------------------------------------------------------------------------------------------------------------------
{
	if (mInternals->mUsingType == kCTimeInfoUsingTypeSeconds)
		mInternals->mSecondCount += secondCount;
	else {
		// Convert to seconds
		// Calculate new seconds
		mInternals->mSecondCount = getSecondCount() + secondCount;
		
		// Store type
		mInternals->mUsingType = kCTimeInfoUsingTypeSeconds;
	}
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CTimeInfo::getSecondCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	switch (mInternals->mUsingType) {
		case kCTimeInfoUsingTypeSamples:
			if (mInternals->mSampleCount == 0)
				return 0.0;

			AssertFailIf(mInternals->mSampleRate == 0.0);
			if (mInternals->mSampleRate == 0.0)
				return 0.0;

			return (Float32) mInternals->mSampleCount / mInternals->mSampleRate;
		
		case kCTimeInfoUsingTypeFrames:
			if (mInternals->mFrameCount == 0)
				return 0.0;

			AssertFailIf(mInternals->mFrameRate == 0.0);
			if (mInternals->mFrameRate == 0.0)
				return 0.0;

			return (Float32) mInternals->mFrameCount / mInternals->mFrameRate;
		
		case kCTimeInfoUsingTypeSeconds:
			return mInternals->mSecondCount;
	}
	
	return 0;
}

//----------------------------------------------------------------------------------------------------------------------
void CTimeInfo::setSampleRate(Float32 sampleRate)
//----------------------------------------------------------------------------------------------------------------------
{
	switch (mInternals->mUsingType) {
		case kCTimeInfoUsingTypeSamples:
			// Is it changing?
			if (sampleRate != mInternals->mSampleRate) {
				// Yes
				// We have one?
				if (mInternals->mSampleRate == 0.0)
					// No, just set
					mInternals->mSampleRate = sampleRate;
				else {
					// Must convert to new sample rate
					mInternals->mSampleCount =
							(SInt64) ((Float32) mInternals->mSampleCount * sampleRate / mInternals->mSampleRate);
					mInternals->mSampleRate = sampleRate;
				}
			}
			break;
		
		case kCTimeInfoUsingTypeFrames:
		case kCTimeInfoUsingTypeSeconds:
			// Just store
			mInternals->mSampleRate = sampleRate;
			break;
	}
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CTimeInfo::getSampleRate() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mSampleRate;
}

//----------------------------------------------------------------------------------------------------------------------
void CTimeInfo::setSampleCount(SInt64 sampleCount)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mSampleCount = sampleCount;
	mInternals->mUsingType = kCTimeInfoUsingTypeSamples;
}

//----------------------------------------------------------------------------------------------------------------------
void CTimeInfo::addSampleCount(SInt64 sampleCount)
//----------------------------------------------------------------------------------------------------------------------
{
	if (mInternals->mUsingType == kCTimeInfoUsingTypeSamples)
		mInternals->mSampleCount += sampleCount;
	else {
		mInternals->mSampleCount = getSampleCount() + sampleCount;
		mInternals->mUsingType = kCTimeInfoUsingTypeSamples;
	}
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CTimeInfo::getSampleCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	switch (mInternals->mUsingType) {
		case kCTimeInfoUsingTypeSeconds:
			if (mInternals->mSecondCount == 0.0)
				return 0;

			AssertFailIf(mInternals->mSampleRate == 0.0);
			if (mInternals->mSampleRate == 0.0)
				return 0;

			return (SInt64) (mInternals->mSampleRate * mInternals->mSecondCount);

		case kCTimeInfoUsingTypeSamples:
			return mInternals->mSampleCount;
		
		case kCTimeInfoUsingTypeFrames:
			if (mInternals->mFrameCount == 0)
				return 0;

			AssertFailIf(mInternals->mFrameRate == 0.0);
			if (mInternals->mFrameRate == 0.0)
				return 0;

			AssertFailIf(mInternals->mSampleRate == 0.0);
			if (mInternals->mSampleRate == 0.0)
				return 0;

			return (SInt64) ((Float32) mInternals->mFrameCount / mInternals->mFrameRate * mInternals->mSampleRate);
	}
	
	return 0;
}

//----------------------------------------------------------------------------------------------------------------------
void CTimeInfo::setFrameRate(Float32 frameRate)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mFrameRate = frameRate;
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CTimeInfo::getFrameRate() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFrameRate;
}

//----------------------------------------------------------------------------------------------------------------------
void CTimeInfo::setFrameCount(SInt64 frameCount)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mFrameCount = frameCount;
	mInternals->mUsingType = kCTimeInfoUsingTypeFrames;
}

//----------------------------------------------------------------------------------------------------------------------
void CTimeInfo::addFrameCount(SInt64 frameCount)
//----------------------------------------------------------------------------------------------------------------------
{
	if (mInternals->mUsingType == kCTimeInfoUsingTypeFrames)
		mInternals->mFrameCount += frameCount;
	else {
		mInternals->mFrameCount = getFrameCount() + frameCount;
		mInternals->mUsingType = kCTimeInfoUsingTypeFrames;
	}
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CTimeInfo::getFrameCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	switch (mInternals->mUsingType) {
		case kCTimeInfoUsingTypeSeconds:
			if (mInternals->mSecondCount == 0.0)
				return 0;

			AssertFailIf(mInternals->mFrameRate == 0.0);
			if (mInternals->mFrameRate == 0.0)
				return 0;

			return (SInt64) (mInternals->mFrameRate * mInternals->mSecondCount);

		case kCTimeInfoUsingTypeSamples:
			if (mInternals->mSampleCount == 0)
				return 0;

			AssertFailIf(mInternals->mSampleRate == 0.0);
			if (mInternals->mSampleRate == 0.0)
				return 0;

			AssertFailIf(mInternals->mFrameRate == 0.0);
			if (mInternals->mFrameRate == 0.0)
				return 0;

			return (SInt64) ((Float32) mInternals->mSampleCount / mInternals->mSampleRate * mInternals->mFrameRate);
		
		case kCTimeInfoUsingTypeFrames:
			return mInternals->mFrameCount;
	}
	
	return 0;
}

//----------------------------------------------------------------------------------------------------------------------
CString CTimeInfo::getTimeAsString(UInt32 flags) const
//----------------------------------------------------------------------------------------------------------------------
{
	CString	string;
	bool	doRound = flags & kTimeInfoTimeFlagsRound;
	bool	doAddLabel = flags & kTimeInfoTimeFlagsAddLabel;
	UInt32	useValue = flags & kTimeInfoTimeFlagsUseMask;
	
	Float32	totalSeconds;
	UInt32	hours, minutes, seconds, frames;
	switch (useValue) {
		case kTimeInfoTimeFlagsUseDays:
		case kTimeInfoTimeFlagsUseDaysHours:
		case kTimeInfoTimeFlagsUseDaysHoursMinutes:
		case kTimeInfoTimeFlagsUseDaysHoursMinutesSeconds:
		case kTimeInfoTimeFlagsUseHours:
		case kTimeInfoTimeFlagsUseHoursMinutes:
		case kTimeInfoTimeFlagsUseHoursMinutesSeconds:
		case kTimeInfoTimeFlagsUseMinutes:
		case kTimeInfoTimeFlagsUseMinutesSeconds:
		case kTimeInfoTimeFlagsUseSeconds: {
			// Get total seconds
			totalSeconds = getSecondCount();
			
			// Notes
			bool	started = false;
			
			// Do rounding
			if (doRound) {
				switch (useValue) {
					case kTimeInfoTimeFlagsUseDays:
						// Round to nearest day
						totalSeconds += 24.0 * 60.0 * 60.0 / 2.0;
						break;

					case kTimeInfoTimeFlagsUseDaysHours:
					case kTimeInfoTimeFlagsUseHours:
						// Round to nearest hour
						totalSeconds += 60.0 * 60.0 / 2.0;
						break;

					case kTimeInfoTimeFlagsUseDaysHoursMinutes:
					case kTimeInfoTimeFlagsUseHoursMinutes:
					case kTimeInfoTimeFlagsUseMinutes:
						// Round to nearest minute
						totalSeconds += 60.0 / 2.0;
						break;

					case kTimeInfoTimeFlagsUseDaysHoursMinutesSeconds:
					case kTimeInfoTimeFlagsUseHoursMinutesSeconds:
					case kTimeInfoTimeFlagsUseMinutesSeconds:
					case kTimeInfoTimeFlagsUseSeconds:
						// Round to nearest second
						// 8/24/08 - Stevo removed as on 10.5.7, totalSeconds of 1 generates 0:02 in string with rounding
//						totalSeconds += 0.5;
						break;
				}
			}
			
			// Do days
			if ((useValue == kTimeInfoTimeFlagsUseDays) ||
					(useValue == kTimeInfoTimeFlagsUseDaysHours) ||
					(useValue == kTimeInfoTimeFlagsUseDaysHoursMinutes) ||
					(useValue == kTimeInfoTimeFlagsUseDaysHoursMinutesSeconds)) {
				// Update string
				UInt32	days = (UInt32) totalSeconds / (24 * 60 * 60);
				string += CString(days);
				if (useValue != kTimeInfoTimeFlagsUseDays)
					string += CString(":");
				if ((useValue == kTimeInfoTimeFlagsUseDays) && doAddLabel)
					string += CString("d");

				// Update info
				totalSeconds -= (Float32) (days * 24 * 60 * 60);
				started = true;
			}
			
			// Do hours
			if ((useValue == kTimeInfoTimeFlagsUseDaysHours) ||
					(useValue == kTimeInfoTimeFlagsUseDaysHoursMinutes) ||
					(useValue == kTimeInfoTimeFlagsUseDaysHoursMinutesSeconds) ||
					(useValue == kTimeInfoTimeFlagsUseHours) ||
					(useValue == kTimeInfoTimeFlagsUseHoursMinutes) ||
					(useValue == kTimeInfoTimeFlagsUseHoursMinutesSeconds)) {
				// Update string
				hours = (UInt32) totalSeconds / (60 * 60);
				string += CString(hours, (UInt32) (started ? 2 : 0), true);
				if ((useValue != kTimeInfoTimeFlagsUseDaysHours) && (useValue != kTimeInfoTimeFlagsUseHours))
					string += CString(":");
				if (((useValue == kTimeInfoTimeFlagsUseDaysHours) || (useValue == kTimeInfoTimeFlagsUseHours)) &&
						doAddLabel)
					string += CString("h");

				// Update info
				totalSeconds -= (Float32) (hours * 60 * 60);
				started = true;
			}
			
			// Do minutes
			if ((useValue == kTimeInfoTimeFlagsUseDaysHoursMinutes) ||
					(useValue == kTimeInfoTimeFlagsUseDaysHoursMinutesSeconds) ||
					(useValue == kTimeInfoTimeFlagsUseHoursMinutes) ||
					(useValue == kTimeInfoTimeFlagsUseHoursMinutesSeconds) ||
					(useValue == kTimeInfoTimeFlagsUseMinutes) ||
					(useValue == kTimeInfoTimeFlagsUseMinutesSeconds)) {
				// Update string
				minutes = (UInt32) totalSeconds / 60;
				string += CString(minutes, (UInt32) (started ? 2 : 0), true);
				if ((useValue != kTimeInfoTimeFlagsUseDaysHoursMinutes) &&
						(useValue != kTimeInfoTimeFlagsUseHoursMinutes) &&
						(useValue != kTimeInfoTimeFlagsUseMinutes))
					string += CString(":");
				if (((useValue == kTimeInfoTimeFlagsUseDaysHoursMinutes) ||
						(useValue == kTimeInfoTimeFlagsUseHoursMinutes) ||
						(useValue == kTimeInfoTimeFlagsUseMinutes)) && doAddLabel)
					string += CString("m");

				// Update info
				totalSeconds -= (Float32) (minutes * 60);
			}
			
			// Do seconds
			if ((useValue == kTimeInfoTimeFlagsUseDaysHoursMinutesSeconds) ||
					(useValue == kTimeInfoTimeFlagsUseHoursMinutesSeconds) ||
					(useValue == kTimeInfoTimeFlagsUseMinutesSeconds) ||
					(useValue == kTimeInfoTimeFlagsUseSeconds)) {
				// Update string
				if (doRound)
					string += CString(totalSeconds, 2, 0, true);
				else
					string += CString(totalSeconds, 5, 2, true);
				if (doAddLabel)
					string += CString("s");
			}
			} break;
		
		case kTimeInfoTimeFlagsUseSecondsHMS:
		case kTimeInfoTimeFlagsUseMinutesSecondsHMS:
		case kTimeInfoTimeFlagsUseHoursMinutesSecondsHMS: {
			// Seconds
			// Get total seconds
			totalSeconds = getSecondCount();
			
			// Do days
			bool	started = false;
			UInt32	days = (UInt32) totalSeconds / (24 * 60 * 60);
			if (days > 0) {
				string += CString(days) + CString("d");
				totalSeconds -= (Float32) (days * 24 * 60 * 60);
				started = true;
			}
			
			// Do hours
			hours = (UInt32) totalSeconds / (60 * 60);
			if ((hours > 0) || (useValue == kTimeInfoTimeFlagsUseHoursMinutesSecondsHMS) || started) {
				string +=
						CString(hours, (UInt32) (started ? 2 : 0),
								(useValue == kTimeInfoTimeFlagsUseHoursMinutesSecondsHMS) || started) + CString("h");
				totalSeconds -= (Float32) (hours * 60 * 60);
				started = true;
			}
			
			// Do minutes
			minutes = (UInt32) totalSeconds / 60;
			if ((minutes > 0) || (useValue == kTimeInfoTimeFlagsUseHoursMinutesSecondsHMS) ||
					(useValue == kTimeInfoTimeFlagsUseMinutesSecondsHMS) || started) {
				string += CString(minutes, (UInt32) (started ? 2 : 0), true) + CString("m");
				totalSeconds -= (Float32) (minutes * 60);
			}
			
			// Do seconds
			if (flags & kTimeInfoTimeFlagsRound)
				string += CString(totalSeconds, 2, 0, true);
			else
				string += CString(totalSeconds, 5, 2, true);
			string += CString("s");
			} break;
		
		case kTimeInfoTimeFlagsUseSamples:
			// Samples
			string = CString(getSampleCount());

			if (flags & kTimeInfoTimeFlagsAddLabel)
				string += CString(" samples");
			break;
		
		case kTimeInfoTimeFlagsUseFrames:
			// Frames
			string = CString(getFrameCount());

			if (flags & kTimeInfoTimeFlagsAddLabel)
				string += CString(" frames");
			break;
		
		case kTimeInfoTimeFlagsUse24fpsTimecode:
			// 24fps Timecode
			// Get total seconds
			totalSeconds = getSecondCount();
			
			// Do hours
			hours = (UInt32) totalSeconds / (60 * 60);
			totalSeconds -= (Float32) (hours * 60 * 60);
			
			// Do minutes
			minutes = (UInt32) totalSeconds / 60;
			totalSeconds -= (Float32) (minutes * 60);
			
			// Do seconds
			seconds = (UInt32) totalSeconds;
			totalSeconds -= (Float32) seconds;
			
			// Do frames
			frames = (UInt32) (totalSeconds * 24.0);
			
			// Prepare string
			string =
					CString(hours, (UInt32) 2, true) + CString(":") + CString(minutes, (UInt32) 2, true) + CString(":")
							+ CString(seconds, (UInt32) 2, true) + CString(":") + CString(frames, (UInt32) 2, true);
			break;
		
		case kTimeInfoTimeFlagsUse25fpsTimecode:
			// 25fps Timecode
			// Get total seconds
			totalSeconds = getSecondCount();
			
			// Do hours
			hours = (UInt32) totalSeconds / (60 * 60);
			totalSeconds -= (Float32) (hours * 60 * 60);
			
			// Do minutes
			minutes = (UInt32) totalSeconds / 60;
			totalSeconds -= (Float32) (minutes * 60);
			
			// Do seconds
			seconds = (UInt32) totalSeconds;
			totalSeconds -= (Float32) seconds;
			
			// Do frames
			frames = (UInt32) (totalSeconds * 25.0);
			
			// Prepare string
			string =
					CString(hours, (UInt32) 2, true) + CString(":") + CString(minutes, (UInt32) 2, true) + CString(":")
							+ CString(seconds, (UInt32) 2, true) + CString(":") + CString(frames, (UInt32) 2, true);
			break;
		
		case kTimeInfoTimeFlagsUse30fpsDropFrameTimecode:
			// 30fps Drop-Frame Timecode
			// Get total seconds
			totalSeconds = getSecondCount();
			
			// Do hours
			hours = (UInt32) totalSeconds / (60 * 60);
			totalSeconds -= (Float32) (hours * 60 * 60);
			
			// Do minutes
			minutes = (UInt32) totalSeconds / 60;
			totalSeconds -= (Float32) (minutes * 60);
			
			// Do seconds
			seconds = (UInt32) totalSeconds;
			totalSeconds -= (Float32) seconds;
			
			// Do frames
			frames = (UInt32) (totalSeconds * 30.0);
			
			// Adjust for drop-frame
			frames += hours * 6 * 2;
			frames += minutes / 10 * 2;
			while (frames > 29) {
				frames -= 30;
				seconds++;
				if (seconds == 60) {
					seconds = 0;
					minutes++;
					if (minutes == 60) {
						minutes = 0;
						hours++;
					}
				}
			}
			
			// Prepare string
			string =
					CString(hours, (UInt32) 2, true) + CString(";") + CString(minutes, (UInt32) 2, true) + CString(";")
							+ CString(seconds, (UInt32) 2, true) + CString(";") + CString(frames, (UInt32) 2, true);
			break;
		
		case kTimeInfoTimeFlagsUse30fpsNonDropFrameTimecode:
			// 30fps Non Drop-Frame Timecode
			// Get total seconds
			totalSeconds = getSecondCount();
			
			// Do hours
			hours = (UInt32) totalSeconds / (60 * 60);
			totalSeconds -= (Float32) (hours * 60 * 60);
			
			// Do minutes
			minutes = (UInt32) totalSeconds / 60;
			totalSeconds -= (Float32) (minutes * 60);
			
			// Do seconds
			seconds = (UInt32) totalSeconds;
			totalSeconds -= (Float32) seconds;
			
			// Do frames
			frames = (UInt32) (totalSeconds * 30.0);
			
			// Prepare string
			string =
					CString(hours, (UInt32) 2, true) + CString(":") + CString(minutes, (UInt32) 2, true) + CString(":")
							+ CString(seconds, (UInt32) 2, true) + CString(":") + CString(frames, (UInt32) 2, true);
			break;
	}
	
	return string;
}

//----------------------------------------------------------------------------------------------------------------------
void CTimeInfo::setTimeFromString(const CString& string, UInt32 flags)
//----------------------------------------------------------------------------------------------------------------------
{
	// Break up string into pieces
	switch (flags & kTimeInfoTimeFlagsUseMask) {
		case kTimeInfoTimeFlagsUseDaysHoursMinutesSeconds:
		case kTimeInfoTimeFlagsUseHoursMinutesSeconds:
		case kTimeInfoTimeFlagsUseMinutesSeconds:
		case kTimeInfoTimeFlagsUseSeconds:
		case kTimeInfoTimeFlagsUseHoursMinutesSecondsHMS:
		case kTimeInfoTimeFlagsUseMinutesSecondsHMS:
		case kTimeInfoTimeFlagsUseSecondsHMS: {
			// Seconds
			CString	stringCopy = string;
			stringCopy.replaceSubStrings(CString("h"), CString(":"));
			stringCopy.replaceSubStrings(CString("m"), CString(":"));

			TArray<CString>	array = stringCopy.breakUp(CString(":"));
			switch (array.getCount()) {
				case 4:
					// Days:Hours:Minutes:Seconds
					setSecondCount(
							array[0].getFloat32() * (Float32) (24.0 * 60.0 * 60.0) +
							array[1].getFloat32() * (Float32) (60.0 * 60.0) +
							array[2].getFloat32() * (Float32) 60.0 +
							array[3].getFloat32());
					break;
				
				case 3:
					// Hours:Minutes:Seconds
					setSecondCount(
							array[0].getFloat32() * (Float32) (60.0 * 60.0) +
							array[1].getFloat32() * (Float32) 60.0 +
							array[2].getFloat32());
					break;
				
				case 2:
					// Minutes:Seconds
					setSecondCount(
							array[0].getFloat32() * (Float32) 60.0 +
							array[1].getFloat32());
					break;
				
				case 1:
					// Seconds
					setSecondCount(array[0].getFloat32());
					break;
				
				default:
					// Unknown
					LogIfError(kParamError, "converting to seconds");
					setSecondCount(0.0);
					break;
			}
			} break;
		
		case kTimeInfoTimeFlagsUseSamples:
			// Samples
			setSampleCount(string.getSInt64());
			break;
		
		case kTimeInfoTimeFlagsUseFrames:
			// Frames
			setFrameCount(string.getSInt64());
			break;
		
		case kTimeInfoTimeFlagsUse24fpsTimecode: {
			// 24fps Timecode
			TArray<CString>	array = string.breakUp(CString(":"));
			switch (array.getCount()) {
				case 4:
					// Hours:Minutes:Seconds:Frames
					setSecondCount(
							array[0].getFloat32() * (Float32) (60.0 * 60.0) +
							array[1].getFloat32() * (Float32) 60.0 +
							array[2].getFloat32() +
							array[3].getFloat32() / (Float32) 24.0);
					break;
				
				case 3:
					// Minutes:Seconds:Frames
					setSecondCount(
							array[0].getFloat32() * (Float32) 60.0 +
							array[1].getFloat32() +
							array[2].getFloat32() / (Float32) 24.0);
					break;
				
				case 2:
					// Seconds:Frames
					setSecondCount(
							array[0].getFloat32() +
							array[1].getFloat32() / (Float32) 24.0);
					break;
				
				case 1:
					// Frames
					setSecondCount(array[0].getFloat32() / (Float32) 24.0);
					break;
				
				default:
					// Unknown
					LogIfError(kParamError, "converting to seconds");
					setSecondCount(0.0);
					break;
			}
			} break;
		
		case kTimeInfoTimeFlagsUse25fpsTimecode: {
			// 25fps Timecode
			TArray<CString>	array = string.breakUp(CString(":"));
			switch (array.getCount()) {
				case 4:
					// Hours:Minutes:Seconds:Frames
					setSecondCount(
							array[0].getFloat32() * (Float32) (60.0 * 60.0) +
							array[1].getFloat32() * (Float32) 60.0 +
							array[2].getFloat32() +
							array[3].getFloat32() / (Float32) 25.0);
					break;
				
				case 3:
					// Minutes:Seconds:Frames
					setSecondCount(
							array[0].getFloat32() * (Float32) 60.0 +
							array[1].getFloat32() +
							array[2].getFloat32() / (Float32) 25.0);
					break;
				
				case 2:
					// Seconds:Frames
					setSecondCount(
							array[0].getFloat32() +
							array[1].getFloat32() / (Float32) 25.0);
					break;
				
				case 1:
					// Frames
					setSecondCount(array[0].getFloat32() / (Float32) 25.0);
					break;
				
				default:
					// Unknown
					LogIfError(kParamError, "converting to seconds");
					setSecondCount(0.0);
					break;
			}
			} break;
		
		case kTimeInfoTimeFlagsUse30fpsDropFrameTimecode: {
			// 30fps Drop-Frame Timecode
			SInt64			hours, minutes, seconds, frames;
			TArray<CString>	array = string.breakUp(CString(";"));
			switch (array.getCount()) {
				case 4:
					hours = array[0].getSInt64();
					minutes = array[1].getSInt64();
					seconds = array[2].getSInt64();
					frames = array[3].getSInt64();
					break;
				
				case 3:
					hours = 0;
					minutes = array[0].getSInt64();
					seconds = array[1].getSInt64();
					frames = array[2].getSInt64();
					break;
				
				case 2:
					hours = 0;
					minutes = 0;
					seconds = array[0].getSInt64();
					frames = array[1].getSInt64();
					break;
				
				case 1:
					hours = 0;
					minutes = 0;
					seconds = 0;
					frames = array[0].getSInt64();
					break;
				
				default:
					// Unknown
					LogIfError(kParamError, "converting to seconds");
					setSecondCount(0.0);
					return;
			}
			
			// Adjust for drop-frame
			frames -= hours * 6 * 2;
			frames -= minutes / 10 * 2;
			while (frames < 0) {
				frames += 30;
				seconds--;
				if (seconds < 0) {
					seconds = 59;
					minutes--;
					if (minutes < 0) {
						minutes = 59;
						hours--;
					}
				}
			}
			
			setSecondCount(
					(Float32) hours * (Float32) (60.0 * 60.0) + (Float32) minutes * (Float32) 60.0 + (Float32) seconds +
							(Float32) frames / (Float32) 30.0);
			} break;
		
		case kTimeInfoTimeFlagsUse30fpsNonDropFrameTimecode: {
			// 30fps Non Drop-Frame Timecode
			TArray<CString>	array = string.breakUp(CString(":"));
			switch (array.getCount()) {
				case 4:
					// Hours:Minutes:Seconds:Frames
					setSecondCount(
							array[0].getFloat32() * (Float32) (60.0 * 60.0) +
							array[1].getFloat32() * (Float32) 60.0 +
							array[2].getFloat32() +
							array[3].getFloat32() / (Float32) 30.0);
					break;
				
				case 3:
					// Minutes:Seconds:Frames
					setSecondCount(
							array[0].getFloat32() * (Float32) 60.0 +
							array[1].getFloat32() +
							array[2].getFloat32() / (Float32) 30.0);
					break;
				
				case 2:
					// Seconds:Frames
					setSecondCount(
							array[0].getFloat32() +
							array[1].getFloat32() / (Float32) 30.0);
					break;
				
				case 1:
					// Frames
					setSecondCount(array[0].getFloat32() / (Float32) 30.0);
					break;
				
				default:
					// Unknown
					LogIfError(kParamError, "converting to seconds");
					setSecondCount(0.0);
					break;
			}
			} break;
	}
}

#if TARGET_OS_MACOS
//----------------------------------------------------------------------------------------------------------------------
TimeValue CTimeInfo::getTimeValue(TimeScale timeScale) const
//----------------------------------------------------------------------------------------------------------------------
{
	return (TimeValue) (getSecondCount() * (Float32) timeScale);
}
#endif

//----------------------------------------------------------------------------------------------------------------------
CDictionary CTimeInfo::getInfo() const
//----------------------------------------------------------------------------------------------------------------------
{
	CDictionary	info;
	
	// Set Using Type
	info.set(sUsingTypeKey, mInternals->mUsingType);
	
	// Set Sample Rate
	info.set(sSampleRateKey, mInternals->mSampleRate);
	
	// Set Frame Rate
	info.set(sFrameRateKey, mInternals->mFrameRate);
	
	switch (mInternals->mUsingType) {
		case kCTimeInfoUsingTypeSamples:
			// Set Sample Count
			info.set(sSampleCountKey, mInternals->mSampleCount);
			break;
		
		case kCTimeInfoUsingTypeFrames:
			// Set Frame Count
			info.set(sFrameCountKey, mInternals->mFrameCount);
			break;
		
		case kCTimeInfoUsingTypeSeconds:
			// Set Second Count
			info.set(sSecondCountKey, mInternals->mSecondCount);
			break;
	}

	return info;
}

////----------------------------------------------------------------------------------------------------------------------
//CTimeInfo::CTimeInfo(const CDictionaryX& info)
////----------------------------------------------------------------------------------------------------------------------
//{
//	mInternals = new CTimeInfoInternals();
//
//	// Get Using Type
//	mInternals->mUsingType = info.getUInt8(sUsingTypeKey, kCTimeInfoUsingTypeSeconds);
//
//	// Get Sample Rate
//	mInternals->mSampleRate = info.getFloat32(sSampleRateKey, 0.0);
//
//	// Get Frame Rate
//	mInternals->mFrameRate = info.getFloat32(sFrameRateKey, 0.0);
//
//	switch (mInternals->mUsingType) {
//		case kCTimeInfoUsingTypeSamples:
//			// Set Sample Count
//			mInternals->mSampleCount = info.getSInt64(sSampleCountKey, 0);
//			mInternals->mFrameCount = 0;
//			mInternals->mSecondCount = 0.0;
//			break;
//
//		case kCTimeInfoUsingTypeFrames:
//			// Set Frame Count
//			mInternals->mFrameCount = info.getSInt64(sFrameCountKey, 0);
//			mInternals->mSampleCount = 0;
//			mInternals->mSecondCount = 0.0;
//			break;
//
//		case kCTimeInfoUsingTypeSeconds:
//			// Set Second Count
//			mInternals->mSecondCount = info.getFloat32(sSecondCountKey, 0.0);
//			mInternals->mSampleCount = 0;
//			mInternals->mFrameCount = 0;
//			break;
//	}
//}
//
////----------------------------------------------------------------------------------------------------------------------
//CDictionaryX CTimeInfo::getInfoX() const
////----------------------------------------------------------------------------------------------------------------------
//{
//	CDictionaryX	dict;
//
//	// Set Using Type
//	dict.set(sUsingTypeKey, mInternals->mUsingType);
//
//	// Set Sample Rate
//	dict.set(sSampleRateKey, mInternals->mSampleRate);
//
//	// Set Frame Rate
//	dict.set(sFrameRateKey, mInternals->mFrameRate);
//
//	switch (mInternals->mUsingType) {
//		case kCTimeInfoUsingTypeSamples:
//			// Set Sample Count
//			dict.set(sSampleCountKey, mInternals->mSampleCount);
//			break;
//
//		case kCTimeInfoUsingTypeFrames:
//			// Set Frame Count
//			dict.set(sFrameCountKey, mInternals->mFrameCount);
//			break;
//
//		case kCTimeInfoUsingTypeSeconds:
//			// Set Second Count
//			dict.set(sSecondCountKey, mInternals->mSecondCount);
//			break;
//	}
//
//	return dict;
//}

//----------------------------------------------------------------------------------------------------------------------
bool CTimeInfo::operator==(const CTimeInfo& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Test by seconds
	return getSecondCount() == other.getSecondCount();
}

//----------------------------------------------------------------------------------------------------------------------
bool CTimeInfo::operator!=(const CTimeInfo& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	return getSecondCount() != other.getSecondCount();
}

//----------------------------------------------------------------------------------------------------------------------
bool CTimeInfo::operator<(const CTimeInfo& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	return getSecondCount() < other.getSecondCount();
}

//----------------------------------------------------------------------------------------------------------------------
bool CTimeInfo::operator<=(const CTimeInfo& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	return getSecondCount() <= other.getSecondCount();
}

//----------------------------------------------------------------------------------------------------------------------
bool CTimeInfo::operator>(const CTimeInfo& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	return getSecondCount() > other.getSecondCount();
}

//----------------------------------------------------------------------------------------------------------------------
bool CTimeInfo::operator>=(const CTimeInfo& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	return getSecondCount() >= other.getSecondCount();
}

//----------------------------------------------------------------------------------------------------------------------
CTimeInfo& CTimeInfo::operator=(const CTimeInfo& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Quick check for assignment to self
	if (this == &other)
		return *this;
	
	// Do copy
	mInternals->mUsingType = other.mInternals->mUsingType;
	mInternals->mSecondCount = other.mInternals->mSecondCount;
	mInternals->mSampleRate = other.mInternals->mSampleRate;
	mInternals->mSampleCount = other.mInternals->mSampleCount;
	mInternals->mFrameRate = other.mInternals->mFrameRate;
	mInternals->mFrameCount = other.mInternals->mFrameCount;
	
	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CTimeInfo& CTimeInfo::operator+=(const CTimeInfo& other)
//----------------------------------------------------------------------------------------------------------------------
{
	switch (mInternals->mUsingType) {
		case kCTimeInfoUsingTypeSeconds:
			addSecondCount(other.getSecondCount());
			break;

		case kCTimeInfoUsingTypeSamples:
			addSampleCount(other.getSampleCount());
			break;
		
		case kCTimeInfoUsingTypeFrames:
			addFrameCount(other.getFrameCount());
			break;
	}
	
	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CTimeInfo& CTimeInfo::operator-=(const CTimeInfo& other)
//----------------------------------------------------------------------------------------------------------------------
{
	switch (mInternals->mUsingType) {
		case kCTimeInfoUsingTypeSeconds:
			addSecondCount(-other.getSecondCount());
			break;

		case kCTimeInfoUsingTypeSamples:
			addSampleCount(-other.getSampleCount());
			break;
		
		case kCTimeInfoUsingTypeFrames:
			addFrameCount(-other.getFrameCount());
			break;
	}
	
	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CTimeInfo CTimeInfo::operator+(const CTimeInfo& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	CTimeInfo	timeInfo(*this);
	timeInfo += other;
	
	return timeInfo;
}

//----------------------------------------------------------------------------------------------------------------------
CTimeInfo CTimeInfo::operator-(const CTimeInfo& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	CTimeInfo	timeInfo(*this);
	timeInfo -= other;
	
	return timeInfo;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
bool CTimeInfo::doTimeWindowsOverlap(const CTimeInfo& start1, const CTimeInfo& length1,
		const CTimeInfo& start2, const CTimeInfo& length2)
//----------------------------------------------------------------------------------------------------------------------
{
	CTimeInfo	end1;
	end1 = start1;
	end1 += length1;

	CTimeInfo	end2;
	end2 = start2;
	end2 += length2;
	
	return (end2 > start1) && (end1 > start2);
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTime CTimeInfo::getCurrentUniversalTime()
//----------------------------------------------------------------------------------------------------------------------
{
	UniversalTime	time;
	
#if TARGET_OS_MACOS || TARGET_OS_IOS
	// CFAbsoluteTimeGetCurrent is relative to 1/1/2001
	time = (UniversalTime) ::CFAbsoluteTimeGetCurrent();
#elif TARGET_OS_LINUX
	// (needs verification) gettimeofday is relative to 1/1/2001
	struct	timeval	tv;
	gettimeofday(&tv, nil);
	
	time = (UniversalTime) tv.tv_sec + (UniversalTime) tv.tv_usec / 1000000.0;
#elif TARGET_OS_WINDOWS
	// _ftime64_s is relative to 1/1/1970
	struct	_timeb timebuffer;
	::_ftime64_s(&timebuffer);

	time = timebuffer.time + (UniversalTime) timebuffer.millitm / 1000.0 + kUniversalTimeInterval1970To2001;
#endif

	time += sOffsetInterval;

	return time;
}

//----------------------------------------------------------------------------------------------------------------------
void CTimeInfo::setCurrentUniversalTime(UniversalTime time)
//----------------------------------------------------------------------------------------------------------------------
{
#if TARGET_OS_MACOS || TARGET_OS_IOS
	// CFAbsoluteTimeGetCurrent is relative to 1/1/2001
	sOffsetInterval = time - (UniversalTime) ::CFAbsoluteTimeGetCurrent();
#elif TARGET_OS_LINUX
	// (needs verification) gettimeofday is relative to 1/1/2001
	Float64	timeI;
	UniversalTime	timeD = modf(time, &timeI);

	timeval	tv;
	tv.tv_sec = timeI;
	tv.tv_usec = timeD * 1000000.0;
	settimeofday(&tv, nil);
	
	printf("Setting HW clock\n");
	system("/sbin/hwclock -w");
	
	printf("Verifying HW clock\n");
	system("/sbin/hwclock -s");
	system("date");
#elif TARGET_OS_WINDOWS
	// _ftime64_s is relative to 1/1/1970
	struct	_timeb timebuffer;
	::_ftime64_s(&timebuffer);

	sOffsetInterval =
			time - (timebuffer.time + (UniversalTime)timebuffer.millitm / 1000.0 + kUniversalTimeInterval1970To2001);
#endif
}

//----------------------------------------------------------------------------------------------------------------------
CString CTimeInfo::getStringForUniversalTime(UniversalTime time, ETimeInfoStringStyle dateStyle,
		ETimeInfoStringStyle timeStyle)
//----------------------------------------------------------------------------------------------------------------------
{
	if (time == kUniversalTimeNever)
		return CString(OSSTR("Never"));
	else {
#if (TARGET_OS_MACOS && !TARGET_LINUX_SIMULATOR) || TARGET_OS_IOS
		// Create date formatter
		CFLocaleRef			localeRef = CFLocaleCopyCurrent();
		CFDateFormatterRef	dateFormatterRef =
									::CFDateFormatterCreate(kCFAllocatorDefault, localeRef,
											sGetCFDateFormatterStyleForCTimeInfoStringStyle(dateStyle),
											sGetCFDateFormatterStyleForCTimeInfoStringStyle(timeStyle));
		::CFRelease(localeRef);
		
		// Create string
		CFStringRef	stringRef =
							::CFDateFormatterCreateStringWithAbsoluteTime(kCFAllocatorDefault, dateFormatterRef,
									(CFAbsoluteTime) time);
		::CFRelease(dateFormatterRef);
		
		CString	string(stringRef);
		::CFRelease(stringRef);
		
		return string;
#else
	static	CString			sJanString("Jan");
	static	CString			sFebString("Feb");
	static	CString			sMarString("Mar");
	static	CString			sAprString("Apr");
	static	CString			sMayString("May");
	static	CString			sJunString("Jun");
	static	CString			sJulString("Jul");
	static	CString			sAugString("Aug");
	static	CString			sSepString("Sep");
	static	CString			sOctString("Oct");
	static	CString			sNovString("Nov");
	static	CString			sDecString("Dec");

	static	CString			sJanuaryString("January");
	static	CString			sFebruaryString("February");
	static	CString			sMarchString("March");
	static	CString			sAprilString("April");
	static	CString			sJuneString("June");
	static	CString			sJulyString("July");
	static	CString			sAugustString("August");
	static	CString			sSeptemberString("September");
	static	CString			sOctoberString("October");
	static	CString			sNovemberString("November");
	static	CString			sDecemberString("December");

	static	CString			sSunString("Sunday");
	static	CString			sMonString("Monday");
	static	CString			sTueString("Tuesday");
	static	CString			sWedString("Wednesday");
	static	CString			sThuString("Thursday");
	static	CString			sFriString("Friday");
	static	CString			sSatString("Saturday");

	static	CString			sAMString("am");
	static	CString			sPMString("pm");

			CString*		day;
			CString*		month;
			CString			string;
			SGregorianDate	date = CTimeInfo::getGregorianDateForUniversalTime(time);
			UInt8			hour = (date.mHour == 0) ? 12 : ((date.mHour - 1) % 12 + 1);

	switch (dateStyle) {
		case kTimeInfoStringStyleNone:
			// None
			break;

		case kTimeInfoStringStyleShort:
			// Short - 1/1/1952
			string = CString(date.mMonth) + CString("/") + CString(date.mDay) + CString("/") + CString(date.mYear);
			break;

		case kTimeInfoStringStyleMedium:
			// Medium - Jan 12, 1952
			switch (date.mMonth) {
				case 1:		month = &sJanString;			break;
				case 2:		month = &sFebString;			break;
				case 3:		month = &sMarString;			break;
				case 4:		month = &sAprString;			break;
				case 5:		month = &sMayString;			break;
				case 6:		month = &sJunString;			break;
				case 7:		month = &sJulString;			break;
				case 8:		month = &sAugString;			break;
				case 9:		month = &sSepString;			break;
				case 10:	month = &sOctString;			break;
				case 11:	month = &sNovString;			break;
				case 12:	month = &sDecString;			break;
				default:	month = &CString::mEmpty;	break;
			}

			string = *month + CString(" ") + CString(date.mDay) + CString(", ") + CString(date.mYear);
			break;

		case kTimeInfoStringStyleLong:
			// Long - January 12, 1952
			switch (date.mMonth) {
				case 1:		month = &sJanuaryString;			break;
				case 2:		month = &sFebruaryString;			break;
				case 3:		month = &sMarchString;				break;
				case 4:		month = &sAprilString;				break;
				case 5:		month = &sMayString;				break;
				case 6:		month = &sJuneString;				break;
				case 7:		month = &sJulyString;				break;
				case 8:		month = &sAugustString;				break;
				case 9:		month = &sSeptemberString;			break;
				case 10:	month = &sOctoberString;			break;
				case 11:	month = &sNovemberString;			break;
				case 12:	month = &sDecemberString;			break;
				default:	month = &CString::mEmpty;	break;
			}

			string = *month + CString(" ") + CString(date.mDay) + CString(", ") + CString(date.mYear);
			break;

		case kTimeInfoStringStyleFull:
			// Full - Tuesday, April 12, 1952 AD
			switch (date.mDayOfWeek) {
				case 0:		day = &sSunString;				break;
				case 1:		day = &sMonString;				break;
				case 2:		day = &sTueString;				break;
				case 3:		day = &sWedString;				break;
				case 4:		day = &sThuString;				break;
				case 5:		day = &sFriString;				break;
				case 6:		day = &sSatString;				break;
				default:	day = &CString::mEmpty;	break;
			}

			switch (date.mMonth) {
				case 1:		month = &sJanuaryString;		break;
				case 2:		month = &sFebruaryString;		break;
				case 3:		month = &sMarchString;			break;
				case 4:		month = &sAprilString;			break;
				case 5:		month = &sMayString;			break;
				case 6:		month = &sJuneString;			break;
				case 7:		month = &sJulyString;			break;
				case 8:		month = &sAugustString;			break;
				case 9:		month = &sSeptemberString;		break;
				case 10:	month = &sOctoberString;		break;
				case 11:	month = &sNovemberString;		break;
				case 12:	month = &sDecemberString;		break;
				default:	month = &CString::mEmpty;	break;
			}

			string = *day + CString(", ") + *month + CString(" ") + CString(date.mDay) + CString(", ") + CString(date.mYear);
			break;
	}

	switch (timeStyle) {
		case kTimeInfoStringStyleNone:
			// None
			break;

		case kTimeInfoStringStyleShort:
			// Short - 3:30pm
			if (!string.isEmpty())
				string += CString(" ");
			string +=
					CString(hour) + CString(":") + CString(date.mMinute, 2, true) +
							((date.mHour >= 12) ? sPMString : sAMString);
			break;

		case kTimeInfoStringStyleMedium:
			// Medium - 3:30pm
			if (!string.isEmpty())
				string += CString(" ");
			string +=
					CString(hour) + CString(":") + CString(date.mMinute, 2, true) +
							((date.mHour >= 12) ? sPMString : sAMString);
			break;

		case kTimeInfoStringStyleLong:
			// Long - 3:30:32pm
			if (!string.isEmpty())
				string += CString(" ");
			string +=
					CString(hour) + CString(":") + CString(date.mMinute, 2, true) + CString(":") +
							CString((UInt16) date.mSecond, 2, true) + ((date.mHour >= 12) ? sPMString : sAMString);
			break;

		case kTimeInfoStringStyleFull:
			// Full - 3:30:32pm PST
			if (!string.isEmpty())
				string += CString(" ");
			string +=
					CString(hour) + CString(":") + CString(date.mMinute, 2, true) + CString(":") +
							CString((UInt16) date.mSecond, 2, true) + ((date.mHour >= 12) ? sPMString : sAMString) + CString(" GMT");
			break;
	}


		return string;
#endif
	}
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTime CTimeInfo::getUniversalTimeForString(const CString& string, ETimeInfoStringStyle dateStyle,
		ETimeInfoStringStyle timeStyle)
//----------------------------------------------------------------------------------------------------------------------
{
#if (TARGET_OS_MACOS && !TARGET_LINUX_SIMULATOR) || TARGET_OS_IOS
	// Create date formatter
	CFLocaleRef			localeRef = CFLocaleCopyCurrent();
	CFDateFormatterRef	dateFormatterRef =
								::CFDateFormatterCreate(kCFAllocatorDefault, localeRef,
										sGetCFDateFormatterStyleForCTimeInfoStringStyle(dateStyle),
										sGetCFDateFormatterStyleForCTimeInfoStringStyle(timeStyle));
	::CFRelease(localeRef);
	
	// Get time
	CFAbsoluteTime	time = ::CFAbsoluteTimeGetCurrent();
	CFStringRef		stringRef = eStringCopyCFStringRef(string);
	::CFDateFormatterGetAbsoluteTimeFromString(dateFormatterRef, stringRef, nil, &time);
	::CFRelease(stringRef);
	::CFRelease(dateFormatterRef);
	
	return (UniversalTime) time;
#elif TARGET_OS_LINUX
	// Unimplemented
#elif TARGET_OS_WINDOWS
	// Unimplemented
	return (UniversalTime) 0.0;
#endif
}

//----------------------------------------------------------------------------------------------------------------------
SGregorianDate CTimeInfo::getGregorianDateForUniversalTime(UniversalTime time)
//----------------------------------------------------------------------------------------------------------------------
{
#if TARGET_OS_MACOS || TARGET_OS_IOS
	// macOS and iOS
	CFCalendarRef	calendarRef = ::CFCalendarCopyCurrent();
	int				year, month, day, hour, minute, second, dayOfWeek;
	::CFCalendarDecomposeAbsoluteTime(calendarRef, (CFAbsoluteTime) time, "yMdHmsE", &year, &month, &day, &hour,
			&minute, &second, &dayOfWeek);
	::CFRelease(calendarRef);

	return SGregorianDate(year, month, day, hour, minute, (Float32) second + time - floor(time), dayOfWeek);
#elif TARGET_OS_LINUX
	// Linux
	Float64	timeI;
	UniversalTime	timeD = modf(time, &timeI);

	time_t	theTimeT = timeI;
	tm*	theTM = localtime(&theTimeT);

	return SGregorianDate(theTM->tm_year + 1900, theTM->tm_mon + 1, theTM->tm_mday, theTM->tm_hour, theTM->tm_min,
			(Float32) theTM->tm_sec + timeD, theTM->tm_wday);
#elif TARGET_OS_WINDOWS
	// Windows
			__time64_t	time64 = (__time64_t) time;
	struct	tm			theTM;
	_localtime64_s(&theTM, &time64);

	return SGregorianDate(theTM.tm_year + 1900, theTM.tm_mon + 1, theTM.tm_mday, theTM.tm_hour, theTM.tm_min,
			(Float32) theTM.tm_sec + (Float32) fmod(time, 1), theTM.tm_wday);
#endif
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTime CTimeInfo::getUniversalTimeForGregorianDate(const SGregorianDate& date)
//----------------------------------------------------------------------------------------------------------------------
{
	UniversalTime	time;
	
#if TARGET_OS_MACOS || TARGET_OS_IOS
	// macOS and iOS
	CFCalendarRef	calendarRef = ::CFCalendarCopyCurrent();
	::CFCalendarComposeAbsoluteTime(calendarRef, &time, "yMdHms", date.mYear, date.mMonth, date.mDay, date.mHour,
			date.mMinute, (int) date.mSecond);
	time += date.mSecond - floor(date.mSecond);
	::CFRelease(calendarRef);
#elif TARGET_OS_LINUX
	// Linux
	tm	theTM;
	theTM.tm_year = date.mYear - 1900;
	theTM.tm_mon = date.mMonth - 1;
	theTM.tm_mday = date.mDay;
	theTM.tm_hour = date.mHour;
	theTM.tm_min = date.mMinute;
	theTM.tm_sec = date.mSecond;

	time_t	t = mktime(&theTM);

	Float32	timeI;
	Float32	timeD = modff(date.mSecond, &timeI);

	time = (UniversalTime) t + timeD + kUniversalTimeInterval1970To2001;
#elif TARGET_OS_WINDOWS
	// Windows
	struct	tm	theTM;
	theTM.tm_year = date.mYear - 1900;
	theTM.tm_mon = date.mMonth - 1;
	theTM.tm_mday = date.mDay;
	theTM.tm_hour = date.mHour;
	theTM.tm_min = date.mMinute;
	theTM.tm_sec = (int) date.mSecond;

	__time64_t	time64 = _mktime64(&theTM);

	time = (UniversalTime) time64 + fmod(date.mSecond, 1) + kUniversalTimeInterval1970To2001;
#endif

	return time;
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTime CTimeInfo::addGregorianUnitsToUniversalTime(UniversalTime time, const SGregorianUnits& units)
//----------------------------------------------------------------------------------------------------------------------
{
	// Convert to date
	SGregorianDate	date = getGregorianDateForUniversalTime(time);

	// Add units
	date.mYear += units.mYears;
	date.mMonth += units.mMonths;
	date.mDay += units.mDays;
	date.mHour += units.mHours;
	date.mMinute += units.mMinutes;
	date.mSecond += units.mSeconds;

	// Convert to time
	return getUniversalTimeForGregorianDate(date);
}

#if TARGET_OS_MACOS
//----------------------------------------------------------------------------------------------------------------------
UniversalTime CTimeInfo::getUniversalTimeForUTCDateTime(const UTCDateTime& utcDateTime)
//----------------------------------------------------------------------------------------------------------------------
{
	CFAbsoluteTime	time = ::CFAbsoluteTimeGetCurrent();
	::UCConvertUTCDateTimeToCFAbsoluteTime(&utcDateTime, &time);
	
	return (UniversalTime) time;
}
#endif

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

#if TARGET_OS_MACOS || TARGET_OS_IOS
//----------------------------------------------------------------------------------------------------------------------
CFDateFormatterStyle sGetCFDateFormatterStyleForCTimeInfoStringStyle(ETimeInfoStringStyle format)
//----------------------------------------------------------------------------------------------------------------------
{
	switch (format) {
		case kTimeInfoStringStyleShort:	return kCFDateFormatterShortStyle;
		case kTimeInfoStringStyleMedium:	return kCFDateFormatterMediumStyle;
		case kTimeInfoStringStyleLong:		return kCFDateFormatterLongStyle;
		case kTimeInfoStringStyleFull:		return kCFDateFormatterFullStyle;
		default:							return kCFDateFormatterNoStyle;
	}
}
#endif
