//----------------------------------------------------------------------------------------------------------------------
//	CThread.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"
#include "TimeAndDate.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CThread

class CThread {
	// Types
	public:
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
		typedef	void*	Ref;
#elif defined(TARGET_OS_WINDOWS)
		typedef	DWORD	Ref;
#endif

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

	// Classes
	private:
		class Internals;

	// Methods
	public:
								// Lifecycle methods
								CThread(ThreadProc threadProc, void* userData = nil,
										const CString& name = CString::mEmpty, Options options = kOptionsNone);
				virtual			~CThread();

								// Instance methods
						Ref		getRef() const;

						void	start();
						bool	isRunning() const;
						void	waitUntilFinished() const
									{ while (isRunning()) sleepFor(0.001); }

								// Class methods
		static			Ref		getCurrentRef();
		static			CString	getCurrentRefAsString()
									{
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
										return CString(getCurrentRef());
#elif defined(TARGET_OS_WINDOWS)
										return CString(getCurrentRef(), false);
#endif
									}
		static			void	sleepFor(UniversalTimeInterval universalTimeInterval);

	protected:
								// Lifecycle methods
								CThread(const CString& name = CString::mEmpty, Options options = kOptionsNone);

								// Subclass methods
				virtual	void	run()
									{}

	private:
		static			void	runThreadProc(CThread& thread, void* userData)
									{ thread.run(); }

	// Properties
	private:
		Internals*	mInternals;
};
