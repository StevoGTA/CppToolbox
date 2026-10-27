//----------------------------------------------------------------------------------------------------------------------
//	SVideo.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SVideo.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SVideo::FrameRate

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CString SVideo::FrameRate::getDisplayString(Float64 frameRate)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt32	frameRateThousandths = (UInt32) ::round(frameRate * 1000.0);
	CString	displayString;
	if ((frameRateThousandths % 10) != 0)
		// 3 decimal places
		displayString = CString(frameRate, 0, 3);
	else if ((frameRateThousandths % 100) != 0)
		// 2 decimal places
		displayString = CString(frameRate, 0, 2);
	else if ((frameRateThousandths % 1000) != 0)
		// 1 decimal place
		displayString = CString(frameRate, 0, 1);
	else
		// No decimal places
		displayString = CString((UInt32) ::round(frameRate));

	return displayString + CString(OSSTR(" fps"));
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SVideo::Format

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CString SVideo::Format::getDescription() const
//----------------------------------------------------------------------------------------------------------------------
{
	return CString(mFrameSize.mWidth) + CString(OSSTR("x")) + CString(mFrameSize.mHeight) + CString(OSSTR(" at ")) +
			FrameRate::getDisplayString(mFrameRate);
}
