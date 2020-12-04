//----------------------------------------------------------------------------------------------------------------------
//	CNotificationCenter.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CNotificationCenter

class CNotificationCenterInternals;
class CNotificationCenter {
	// Structures
	public:
		struct ObserverInfo {
			// Procs
			typedef	void	(*Proc)(const CString& notificationName, const void* senderRef, const CDictionary& info,
									void* userData);

					// Lifecycle methods
					ObserverInfo(const void* observerRef, Proc proc, void* userData) :
						mObserverRef(observerRef), mProc(proc), mUserData(userData)
						{}
					ObserverInfo(const ObserverInfo& other) :
						mObserverRef(other.mObserverRef), mProc(other.mProc), mUserData(other.mUserData)
						{}

					// Instance methods
			void	callProc(const CString& notificationName, const void* senderRef, const CDictionary& info) const
						{ mProc(notificationName, senderRef, info, mUserData); }

			// Properties
			const	void*	mObserverRef;
					Proc	mProc;
					void*	mUserData;
		};

	// Methods
	public:
						// Lifcycle methods
						CNotificationCenter();
		virtual			~CNotificationCenter();

						// Instance methods
				void	registerObserver(const CString& notificationName, const void* senderRef,
								const ObserverInfo& observerInfo);
				void	registerObserver(const CString& notificationName, const ObserverInfo& observerInfo);
				void	unregisterObserver(const CString& notificationName, const void* observerRef);
				void	unregisterObserver(const void* observerRef);

				void	send(const CString& notificationName, const void* senderRef = nil,
								const CDictionary& info = CDictionary::mEmpty) const;
//				void	postOnMainThread(const CString& notificationName, const void* senderRef = nil,
//								const CDictionary& info = CDictionary::mEmpty) const;

	// Properties
	private:
				CNotificationCenterInternals*	mInternals;

	public:
		static	CNotificationCenter				mStandard;
};
