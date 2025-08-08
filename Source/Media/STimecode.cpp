//----------------------------------------------------------------------------------------------------------------------
//	STimecode.cpp			©2023 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "STimecode.h"

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: STimecode::FrameRate

// MARK: Properties

STimecode::FrameRate	STimecode::FrameRate::mDefault(kKindNonDropFrame, 24);

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CDictionary STimecode::FrameRate::getInfo() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CDictionary	info;

	info.set(CString(OSSTR("kind")), mKind);
	info.set(CString(OSSTR("base")), mNonDropFrameBase);

	return info;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
OV<STimecode::FrameRate> STimecode::FrameRate::fromInfo(const CDictionary& info)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get info
	OV<UInt16>	kind = info.getOVUInt16(CString(OSSTR("kind")));
	if (!kind.hasValue())
		// No kind
		return OV<FrameRate>();

	// Check kind
	switch (*kind) {
		case kKindNonDropFrame: {
			// Non-Drop Frame
			OV<UInt32>	base = info.getOVUInt32(CString(OSSTR("base")));

			return base.hasValue() ? OV<FrameRate>(FrameRate(kKindNonDropFrame, *base)) : OV<FrameRate>();
		}

		case kKindDropFrame2997:
			// 29.97 Drop Frame
			return OV<FrameRate>(forDropFrame2997());

		case kKindDropFrame5994:
			// 59.94 Drop Frame
			return OV<FrameRate>(forDropFrame5994());

		default:
			// Sorry
			return OV<FrameRate>();
	}
}

//----------------------------------------------------------------------------------------------------------------------
const TArray<STimecode::FrameRate>& STimecode::FrameRate::getStandard()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	TSArray<FrameRate>*	sArray = nil;
	if (sArray == nil) {
		// Setup
		static	FrameRate	sFrameRates[] =
									{
										FrameRate(kKindNonDropFrame, 24),
										FrameRate(kKindNonDropFrame, 25),
										FrameRate(kKindDropFrame2997),
										FrameRate(kKindNonDropFrame, 30),
										FrameRate(kKindDropFrame5994),
										FrameRate(kKindNonDropFrame, 60),
									};
		sArray = new TSARRAY_FROM_C_ARRAY(FrameRate, sFrameRates);
	}

	return *sArray;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - STimecode

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
STimecode::STimecode(SInt32 hours, SInt32 minutes, SInt32 seconds, SInt32 frames, const FrameRate& frameRate) :
	mFrameRate(frameRate)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SInt32	base = (SInt32) mFrameRate.getBase();

	// Check FrameRate
	if (!mFrameRate.isDropFrame())
		// Non-Drop Frame
		mFrameIndex = (((hours * 60) + minutes) * 60 + seconds) * base + frames;
	else {
		// Drop Frame
		// From https://www.davidheidelberger.com/2010/06/10/drop-frame-timecode/
		SInt32	dropFrames =
						(SInt32)
								round(((mFrameRate.getKind() == FrameRate::kKindDropFrame2997) ? 29.97 : 59.94) / 15.0);

		SInt32	hourFrames = base * 60 * 60;
		SInt32	minuteFrames = base * 60;
		SInt32	totalMinutes = hours * 60 + minutes;
		mFrameIndex =
				hours * hourFrames + minutes * minuteFrames + seconds * base + frames -
						(dropFrames * (totalMinutes - (totalMinutes / 10)));
	}
}

//----------------------------------------------------------------------------------------------------------------------
STimecode::STimecode(SInt32 hours, SInt32 minutes, Float32 seconds, const FrameRate& frameRate) : mFrameRate(frameRate)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SInt32	base = (SInt32) mFrameRate.getBase();

	// Check FrameRate
	if (!mFrameRate.isDropFrame())
		// Non-Drop Frame
		mFrameIndex = (((hours * 60) + minutes) * 60) * base + (SInt32) (::round(seconds * (Float32) base));
	else {
		// Drop Frame
		// From https://www.davidheidelberger.com/2010/06/10/drop-frame-timecode/
		SInt32	dropFrames =
						(SInt32)
								round(((mFrameRate.getKind() == FrameRate::kKindDropFrame2997) ? 29.97 : 59.94) / 15.0);

		SInt32	hourFrames = base * 60 * 60;
		SInt32	minuteFrames = base * 60;
		SInt32	totalMinutes = hours * 60 + minutes;
		mFrameIndex =
				hours * hourFrames + minutes * minuteFrames + (SInt32) (::round(seconds * (Float32) base)) -
						(dropFrames * (totalMinutes - (totalMinutes / 10)));
	}
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
STimecode::HMSF STimecode::getHMSF() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SInt32	base = (SInt32) mFrameRate.getBase();

	// Check FrameRate
	if (!mFrameRate.isDropFrame()) {
		// Non-Drop Frame
		SInt32	hours = mFrameIndex / (60 * 60 * base);
		SInt32	minutes = (mFrameIndex / (60 * base)) % 60;
		SInt32	seconds = (mFrameIndex / base) % 60;
		SInt32	frames = mFrameIndex % base;

		return HMSF(hours, minutes, seconds, frames);
	} else {
		// Drop Frame
		Float32	framerate = (mFrameRate.getKind() == FrameRate::kKindDropFrame2997) ? 29.97f : 59.94f;

		// From https://www.davidheidelberger.com/2010/06/10/drop-frame-timecode/
		SInt32	dropFrames = (SInt32) round(framerate * 0.066666);
		SInt32	framesPerHour = (SInt32) round(framerate * 60.0 * 60.0);
		SInt32	framesPer24Hours = framesPerHour * 24;
		SInt32	framesPer10Minutes = (SInt32) round(framerate * 60.0 * 10.0);
		SInt32	framesPerMinute = (SInt32) round(framerate) * 60 - dropFrames;

		// Get frameIndex as a positive number within 0 - 24 hours
		SInt32	frameIndex = mFrameIndex;
		while (frameIndex < 0) { frameIndex += framesPer24Hours; }
		frameIndex %= framesPer24Hours;

		// Update frameIndex for dropped frames
		SInt32	d = frameIndex / framesPer10Minutes;
		SInt32	m = frameIndex % framesPer10Minutes;
		frameIndex +=
				(m > dropFrames) ?
						(dropFrames * 9 * d) + dropFrames * ((m - dropFrames) / framesPerMinute) : dropFrames * 9 * d;

		// Calculate timecode components
		SInt32	hours = frameIndex / (60 * 60 * base);
		SInt32	minutes = (frameIndex / (60 * base)) % 60;
		SInt32	seconds = (frameIndex / base) % 60;
		SInt32	frames = frameIndex % base;

		return HMSF(hours, minutes, seconds, frames);
	}
}

//----------------------------------------------------------------------------------------------------------------------
CString STimecode::getDisplayString() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	HMSF	hmsf = getHMSF();

	return !mFrameRate.isDropFrame() ?
			CString::make(OSSTR("%02d:%02d:%02d:%02d"), hmsf.getHours(), hmsf.getMinutes(), hmsf.getSeconds(),
					hmsf.getFrames()) :
			CString::make(OSSTR("%02d:%02d:%02d;%02d"), hmsf.getHours(), hmsf.getMinutes(), hmsf.getSeconds(),
					hmsf.getFrames());
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary STimecode::getInfo() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CDictionary	info;
	info.set(CString(OSSTR("frameIndex")), mFrameIndex);
	info.set(CString(OSSTR("framerate")), mFrameRate.getInfo());

	return info;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
OV<STimecode> STimecode::fromInfo(const CDictionary& info)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	OV<SInt32>		frameIndex = info.getOVSInt32(CString(OSSTR("frameIndex")));
	OV<FrameRate>	frameRate = FrameRate::fromInfo(info.getDictionary(CString(OSSTR("frameRate"))));

	return (frameIndex.hasValue() && frameRate.hasValue()) ?
			OV<STimecode>(STimecode(*frameIndex, *frameRate)) : OV<STimecode>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<STimecode> STimecode::fromString(const CString& string, const FrameRate& frameRate)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TArray<CString>	components =
							string.replacingSubStrings(CString::mSemiColon, CString::mColon)
									.components(CString::mColon);
	SInt32			hours, minutes, seconds, frames;
	switch (components.getCount()) {
		case 4:
			// HH:MM:SS::FF
			hours = components[0].getSInt32();
			minutes = components[1].getSInt32();
			seconds = components[2].getSInt32();
			frames = components[3].getSInt32();
			break;

		case 3:
			// MM:SS::FF
			hours = 0;
			minutes = components[0].getSInt32();
			seconds = components[1].getSInt32();
			frames = components[2].getSInt32();
			break;

		case 2:
			// SS::FF
			hours = 0;
			minutes = 0;
			seconds = components[0].getSInt32();
			frames = components[1].getSInt32();
			break;

		case 1:
			// FF
			hours = 0;
			minutes = 0;
			seconds = 0;
			frames = components[0].getSInt32();
			break;

		default:
			// Sorry
			return OV<STimecode>();
	}

	return OV<STimecode>(STimecode(hours, minutes, seconds, frames, frameRate));
}
