//----------------------------------------------------------------------------------------------------------------------
//	CThread.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Procs

class CThread;
typedef	void	(*CThreadProc)(const CThread& thread, void* userData);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CThread

class CThreadInternals;
class CThread {
	// Methods
	public:
		// Lifecycle methods
		CThread(CThreadProc proc, void* userData, const CString& name = CString::mEmpty);
		~CThread();

	// Properties
	private:
		CThreadInternals*	mInternals;
};
