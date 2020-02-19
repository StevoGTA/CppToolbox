//----------------------------------------------------------------------------------------------------------------------
//	CThread.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"
#include "TimeAndDate.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Types
typedef	void*	CThreadRef;

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
							CThread(CThreadProc proc, void* userData = nil, const CString& name = CString::mEmpty);
							~CThread();

							// Instance methods
				CThreadRef	getThreadRef() const;

							// Class methods
//		static	void		sleepFor(UniversalTimeInterval timeInterval);
		static	CThreadRef	getCurrentThreadRef();

	// Properties
	private:
		CThreadInternals*	mInternals;
};
