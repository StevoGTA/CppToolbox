//----------------------------------------------------------------------------------------------------------------------
//	STimecode+Default.cpp			©2025 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "STimecode.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: STimecode::FrameRate

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CString STimecode::FrameRate::getDisplayString() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value
	switch (mKind) {
		case kKindNonDropFrame:
			// Non-Drop Frame
			return CString(*mNonDropFrameBase) + CString(OSSTR(" fps NDF"));

		case kKindDropFrame2997:
			// 29.97 Drop Frame
			return CString(OSSTR("29.97 fps DF"));

		case kKindDropFrame5994:
			// 59.94 Drop Frame
			return CString(OSSTR("59.94 fps DF"));
	}
}
