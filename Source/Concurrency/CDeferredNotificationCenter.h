//----------------------------------------------------------------------------------------------------------------------
//	CDeferredNotificationCenter.h			©2023 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CNotificationCenter.h"

#if defined(TARGET_OS_WINDOWS)
	#include "winrt\Microsoft.UI.Dispatching.h"

	using namespace winrt::Microsoft::UI::Dispatching;
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDeferredNotificationCenter

class CDeferredNotificationCenter : public CNotificationCenter {
	// Classes
	private:
		class Internals;

	// Methods
	public:
				// Lifecycle methods
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
				CDeferredNotificationCenter();
#elif defined(TARGET_OS_WINDOWS)
				CDeferredNotificationCenter(const DispatcherQueue& dispatcherQueue);
#endif
				~CDeferredNotificationCenter();

				// CNotificationCenter methods
		void	post(const CString& notificationName, const Sender& sender, const CDictionary& info);
		void	post(const CString& notificationName, const CDictionary& info);

	// Properties
	private:
		Internals*	mInternals;
};
