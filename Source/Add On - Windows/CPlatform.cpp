//----------------------------------------------------------------------------------------------------------------------
//	CPlatform.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CPlatform.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CPlatform

// MARK: String utilities

//----------------------------------------------------------------------------------------------------------------------
CString CPlatform::stringFrom(String^ platformString)
//----------------------------------------------------------------------------------------------------------------------
{
	return CString(platformString->Data());
}