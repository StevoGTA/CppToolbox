//----------------------------------------------------------------------------------------------------------------------
//	CGPUProgramBuiltins.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CGPUProgramBuiltins.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPUOpaqueProgram

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CGPUOpaqueProgram& CGPUOpaqueProgram::getProgram()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	CGPUOpaqueProgram*	sProgram = nil;

	// Check if have shader
	if (sProgram == nil)
		// Create shader
		sProgram = new CGPUOpaqueProgram();

	return *sProgram;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUOpacityProgram

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CGPUOpacityProgram& CGPUOpacityProgram::getProgram()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	CGPUOpacityProgram*	sProgram = nil;

	// Check if have shader
	if (sProgram == nil)
		// Create shader
		sProgram = new CGPUOpacityProgram();

	return *sProgram;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUClipOpacityProgram

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CGPUClipOpacityProgram& CGPUClipOpacityProgram::getProgram()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	CGPUClipOpacityProgram*	sProgram = nil;

	// Check if have shader
	if (sProgram == nil)
		// Create shader
		sProgram = new CGPUClipOpacityProgram();

	return *sProgram;
}
