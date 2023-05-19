//----------------------------------------------------------------------------------------------------------------------
//	CLogFile.h			©2023 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CFile.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CLogFile

class CLogFile : public CFile {
	// Classes
	private:
		class Internals;

	// Methods
	public:
		// Lifecycle methods
		CLogFile(const CFilesystemPath& filesystemPath);
		~CLogFile();

	// Properties
	private:
		Internals*	mInternals;
};
