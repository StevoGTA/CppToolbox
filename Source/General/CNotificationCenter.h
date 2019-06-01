//----------------------------------------------------------------------------------------------------------------------
//	CNotificationCenter.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Types

typedef	void	(*NotificationProc)(const CString& notificationName, const void* senderRef, const CDictionary& info,
						void* userData);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CNotificationCenter

class CNotificationCenterInternals;
class CNotificationCenter {
	// Methods
	public:
						// Lifcycle methods
						CNotificationCenter();
		virtual			~CNotificationCenter();

						// Instance methods
				void	registerObserver(const CString& notificationName, const void* observerRef,
								NotificationProc proc, void* userData = nil);
				UInt32	registerObserver(const CString& notificationName, NotificationProc proc,
								void* userData = nil);
				void	unregisterObserver(const CString& notificationName, const void* observerRef);
				void	unregisterObserver(const void* observerRef);
				void	unregisterObserver(UInt32 observerReference);

				void	send(const CString& notificationName, const void* senderRef = nil,
								const CDictionary& info = CDictionary::mEmpty);
//				void	postOnMainThread(const CString& notificationName, const void* senderRef = nil,
//								const CDictionary& info = CDictionary::mEmpty);

	// Properties
	private:
				CNotificationCenterInternals*	mInternals;

	public:
		static	CNotificationCenter				mStandard;
};
