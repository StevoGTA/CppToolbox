//----------------------------------------------------------------------------------------------------------------------
//	CDeferredNotificationCenter.h			©2023 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CNotificationCenter.h"
#include "CQueue.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDeferredNotificationCenter

class CDeferredNotificationCenter : public CNotificationCenter {
	// Classes
	private:
		class Internals;

	// Methods
	public:
							// Lifecycle methods
							CDeferredNotificationCenter();
							~CDeferredNotificationCenter();

							// CNotificationCenter methods
		void				queue(const CString& notificationName, const OI<Sender>& sender = OI<Sender>(),
									const CDictionary& info = CDictionary::mEmpty);

							// Instance methods
		CSRSWMessageQueue&	getMessageQueue() const;

		void				flush();

	// Properties
	private:
		Internals*	mInternals;
};
