//----------------------------------------------------------------------------------------------------------------------
//	CNotificationCenter.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CNotificationCenter

class CNotificationCenterInternals;
class CNotificationCenter {
	// Sender
	public:
		class Sender {
			public:
								// Lifecycle methods
								Sender() {}
				virtual			~Sender() {}

								// Instance methods
						bool	operator!=(const Sender& other) const
									{ return !operator==(other); }

			protected:
								// Subclass methods
				virtual	bool	operator==(const Sender& other) const = 0;
		};

	// TSender
	public:
		template <typename T> class ISender : public Sender {
			public:
								// Lifecycle methods
								ISender(const I<T>& i) : mI(i) {}

								// Instance methods
						T&		operator*() const
									{ return *mI; }

			protected:
								// Sender methods
						bool	operator==(const Sender& other) const
									{ return *mI == *((const ISender<T>&) other).mI; }

			// Properties
			private:
				I<T>	mI;
		};

	// Observer
	public:
		struct Observer {
			// Procs
			typedef	void	(*Proc)(const CString& notificationName, const OI<Sender>& sender, const CDictionary& info,
									void* userData);

					// Lifecycle methods
					Observer(const void* observerRef, Proc proc, void* userData) :
						mObserverRef(observerRef), mProc(proc), mUserData(userData)
						{}
					Observer(const Observer& other) :
						mObserverRef(other.mObserverRef), mProc(other.mProc), mUserData(other.mUserData)
						{}

					// Instance methods
			void	callProc(const CString& notificationName, const OI<Sender>& sender, const CDictionary& info) const
						{ mProc(notificationName, sender, info, mUserData); }

			// Properties
			const	void*	mObserverRef;
					Proc	mProc;
					void*	mUserData;
		};

	// Methods
	public:
						// Lifcycle methods
		virtual			~CNotificationCenter();

						// Instance methods
				void	registerObserver(const CString& notificationName, const OI<Sender>& sender,
								const Observer& observer);
				void	registerObserver(const CString& notificationName, const Observer& observer);
				void	unregisterObserver(const CString& notificationName, const void* observerRef);
				void	unregisterObserver(const void* observerRef);

		virtual	void	queue(const CString& notificationName, const OI<Sender>& sender = OI<Sender>(),
								const CDictionary& info = CDictionary::mEmpty) = 0;

	protected:
						// Lifcycle methods
						CNotificationCenter();

						// Instance methods
				void	send(const CString& notificationName, const OI<Sender>& sender, const CDictionary& info) const;

	// Properties
	private:
		CNotificationCenterInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: CImmediateNotificationCenter

class CImmediateNotificationCenter : public CNotificationCenter {
	// Methods
	public:
				// Lifecycle methods
				CImmediateNotificationCenter() {}

				// CNotificationCenter methods
		void	queue(const CString& notificationName, const OI<Sender>& sender = OI<Sender>(),
						const CDictionary& info = CDictionary::mEmpty)
					{ send(notificationName, sender, info); }
};
