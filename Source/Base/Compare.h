//----------------------------------------------------------------------------------------------------------------------
//	Compare.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "PlatformDefinitions.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Compare result

enum ECompareResult {
	kCompareResultBefore		= -1,
	kCompareResultEquivalent	= 0,
	kCompareResultAfter			= 1,
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Compare procs

extern	ECompareResult	eCompare(Float32 value1, Float32 value2);
extern	ECompareResult	eCompare(Float64 value1, Float64 value2);
extern	ECompareResult	eCompare(UInt32 value1, UInt32 value2);
