//----------------------------------------------------------------------------------------------------------------------
//	CWorkItem.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CUUID.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CWorkItem

class CWorkItem : public CHashable {
	// Enums
	public:
		enum Priority {
			kPriorityBackground,
			kPriorityNormal,
			kPriorityHigh,
		};

		enum State {
			kStateWaiting,
			kStateActive,
			kStateCompleted,
			kStateCancelled,
		};

	// Types
	typedef	void	(*CompletedProc)(void* userData);
	typedef	void	(*CancelledProc)(void* userData);
	typedef	void	(*Proc)(const I<CWorkItem>& workItem, void* userData);

	// Classes
	private:
		class Internals;

	// Methods
	public:
										// Lifecycle methods
										CWorkItem(const CString& id = CUUID().getBase64String(),
												const OV<CString>& reference = OV<CString>(),
												CompletedProc completedProc = nil, CancelledProc cancelledProc = nil,
												void* userData = nil);
		virtual							~CWorkItem();

										// CEquatable methods
						bool			operator==(const CEquatable& other) const
											{ return getID() == ((const CWorkItem&) other).getID(); }

										// CHashable methods
						void			hashInto(CHashable::HashCollector& hashableHashCollector) const
											{ getID().hashInto(hashableHashCollector); }

										// Instance methods
				const	CString&		getID() const;
				const	OV<CString>&	getReference() const;

						State			getState() const;
						bool			isWaiting() const
											{ return getState() == kStateWaiting; }
						bool			isActive() const
											{ return getState() == kStateActive; }
						bool			isCompleted() const
											{ return getState() == kStateCompleted; }
						bool			isCancelled() const
											{ return getState() == kStateCancelled; }

										// Subclass methods
		virtual			void			perform(const I<CWorkItem>& workItem) = 0;

										// Internal-use only methods
						void			transitionTo(State state);

	protected:
										// Subclass methods
		virtual			void			completed() const {}
		virtual			void			cancelled() const {}

	// Properties
	private:
		Internals*	mInternals;
};
