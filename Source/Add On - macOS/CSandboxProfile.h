//----------------------------------------------------------------------------------------------------------------------
//	CSandboxProfile.h			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CFilesystemPath.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSandboxProfile

class CSandboxProfile {
	// Classes
	private:
		class Internals;

	// Methods
	public:
					// Lifecycle methods
					CSandboxProfile(const CString& name);
					~CSandboxProfile();

					// Instance methods
		OV<bool>	canAccess(const CFilesystemPath& filesystemPath) const;

	// Properties
	private:
		Internals*	mInternals;
};
