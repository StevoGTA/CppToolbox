//----------------------------------------------------------------------------------------------------------------------
//	Compare.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "Compare.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Compare procs

//----------------------------------------------------------------------------------------------------------------------
ECompareResult eCompare(Float32 value1, Float32 value2)
//----------------------------------------------------------------------------------------------------------------------
{
	if (value1 < value2)
		return kCompareResultBefore;
	else if (value1 > value2)
		return kCompareResultAfter;
	else
		return kCompareResultEquivalent;
}

//----------------------------------------------------------------------------------------------------------------------
ECompareResult eCompare(Float64 value1, Float64 value2)
//----------------------------------------------------------------------------------------------------------------------
{
	if (value1 < value2)
		return kCompareResultBefore;
	else if (value1 > value2)
		return kCompareResultAfter;
	else
		return kCompareResultEquivalent;
}

//----------------------------------------------------------------------------------------------------------------------
ECompareResult eCompare(UInt32 value1, UInt32 value2)
//----------------------------------------------------------------------------------------------------------------------
{
	if (value1 < value2)
		return kCompareResultBefore;
	else if (value1 > value2)
		return kCompareResultAfter;
	else
		return kCompareResultEquivalent;
}
