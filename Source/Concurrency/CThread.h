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
typedef void	(*CThreadRunOnMainThreadProc)(void* userData);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CThread

class CThreadInternals;
class CThread {
	// Methods
	public:
									// Lifecycle methods
									CThread(CThreadProc threadProc, void* threadProcUserData = nil,
											const CString& name = CString::mEmpty);
				virtual				~CThread();

									// Instance methods
						CThreadRef	getThreadRef() const;
						bool		getIsRunning() const;

									// Class methods
		static			CThreadRef	getCurrentThreadRef();
		static			void		sleepFor(UniversalTimeInterval universalTimeInterval);
		static			void		runOnMainThread(CThreadRunOnMainThreadProc runOnMainThreadProc, void* userData);

	protected:
									// Lifecycle methods
									CThread(const CString& name = CString::mEmpty);

									// Subclass methods
				virtual	void		run()
										{}

	private:
		static			void		runThreadProc(CThread& thread, void* userData)
										{ thread.run(); }

	// Properties
	private:
		CThreadInternals*	mInternals;
};
