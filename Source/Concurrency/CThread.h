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
		typedef	void*	Ref;

	// Procs:
	public:
		typedef void	(*Proc)(void* userData);
		typedef	void	(*ThreadProc)(CThread& thread, void* userData);

	// Options:
	public:
		enum Options {
			kOptionsNone		= 0,
			kOptionsAutoStart	= 1 << 0,
		};

	// Methods
	public:
								// Lifecycle methods
								CThread(ThreadProc threadProc, void* userData = nil,
										const CString& name = CString::mEmpty, Options options = kOptionsAutoStart);
				virtual			~CThread();

								// Instance methods
						Ref		getRef() const;

						void	start();
						bool	getIsRunning() const;

								// Class methods
		static			Ref		getCurrentRef();
		static			CString	getCurrentRefAsString()
									{ return CString(getCurrentRef()); }
		static			void	sleepFor(UniversalTimeInterval universalTimeInterval);

	protected:
								// Lifecycle methods
								CThread(const CString& name = CString::mEmpty, Options options = kOptionsAutoStart);

								// Subclass methods
				virtual	void	run()
									{}

	private:
		static			void	runThreadProc(CThread& thread, void* userData)
									{ thread.run(); }

	// Properties
	private:
		CThreadInternals*	mInternals;
};
