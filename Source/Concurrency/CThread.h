//----------------------------------------------------------------------------------------------------------------------
//	CThread.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"
#include "TimeAndDate.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CThread

class CThreadInternals;
class CThread {
	// Types
	public:
		typedef	void*	CThreadRef;

	// Procs:
	public:
		typedef void	(*Proc)(void* userData);
		typedef	void	(*ThreadProc)(CThread& thread, void* userData);

	// Methods
	public:
									// Lifecycle methods
									CThread(ThreadProc threadProc, void* userData = nil,
											const CString& name = CString::mEmpty);
				virtual				~CThread();

									// Instance methods
						CThreadRef	getThreadRef() const;
						bool		getIsRunning() const;

									// Class methods
		static			CThreadRef	getCurrentThreadRef();
		static			void		sleepFor(UniversalTimeInterval universalTimeInterval);
		static			void		runOnMain(Proc proc, void* userData);

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
