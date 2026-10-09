//----------------------------------------------------------------------------------------------------------------------
//	CUIThreadQueue.h			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include <functional>

#if defined(TARGET_OS_WINDOWS)
	#include "winrt\Microsoft.UI.Dispatching.h"

	using namespace winrt::Microsoft::UI::Dispatching;
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: CUIThreadQueue

class CUIThreadQueue {
	// Types
	public:
		using Proc = void (*)(void* userData);

	// Classes
	private:
		class Internals;

	// Methods
	public:
				// Lifecycle methods
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
				CUIThreadQueue();
#elif defined(TARGET_OS_WINDOWS)
				CUIThreadQueue(const DispatcherQueue& dispatcherQueue);
#endif
				~CUIThreadQueue();

				// Instance methods
		void	add(Proc proc, void* userData, bool isRequired);
		void	add(const std::function<void()>& proc, bool isRequired);

	// Properties
	private:
		Internals*	mInternals;
};
