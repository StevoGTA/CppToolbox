//----------------------------------------------------------------------------------------------------------------------
//	STimecode.cpp			©20223 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "STimecode.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: STimecode

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
STimecode::STimecode(SInt32 hours, SInt32 minutes, SInt32 seconds, SInt32 frames, Base base)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mBase = base;

	// Check Base
	switch (mBase) {
		case kBase24FPSNonDropFrame:
			// 24 fps
			mFrameIndex = (((hours * 60) + minutes) * 60 + seconds) * 24 + frames;
			break;
	}
}

//----------------------------------------------------------------------------------------------------------------------
STimecode::STimecode(const CString& string, Base base)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mBase = base;

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
			// ???
			mFrameIndex = 0;

			return;
	}

	// Check Base
	switch (mBase) {
		case kBase24FPSNonDropFrame:
			// 24 fps
			mFrameIndex = (((hours * 60) + minutes) * 60 + seconds) * 24 + frames;
			break;
	}
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CString STimecode::getDisplayString() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SInt32	hours = mFrameIndex / (60 * 60 * 24);
	SInt32	minutes = (mFrameIndex / (60 * 24)) % 60;
	SInt32	seconds = (mFrameIndex / 24) % 60;
	SInt32	frames = mFrameIndex % 24;

	return CString::make(OSSTR("%02d:%02d:%02d:%02d"), hours, minutes, seconds, frames);
}
