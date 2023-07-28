//----------------------------------------------------------------------------------------------------------------------
//	CNotificationCenter.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CNotificationCenter

class CNotificationCenter {
	// Sender
	public:
		class Sender {
			// Methods
			public:
								// Lifecycle methods
								Sender() {}
				virtual			~Sender() {}

								// Subclass methods
				virtual	bool	operator==(const Sender& other) const = 0;
		};

	// ISender - Pass pointer to sender that will have delete called at cleanup
	public:
		template <typename T> class ISender : public Sender {
			// Methods
			public:
								// Lifecycle methods
								ISender(const T* t) : Sender(), mT(t) {}
								~ISender() { Delete(mT); }

								// Instance methods
				const	T&		operator*() const
									{ return *mT; }

			protected:
								// Sender methods
						bool	operator==(const Sender& other) const
									{ return *mT == *((const ISender<T>&) other).mT; }

			// Properties
			private:
				const	T*	mT;
		};

	// RSender - Pass reference to sender
	public:
		template <typename T> class RSender : public Sender {
			// Methods
			public:
								// Lifecycle methods
								RSender(const T& t) : Sender(), mT(t) {}

								// Instance methods
						T&		operator*() const
									{ return *mT; }

			protected:
								// Sender methods
						bool	operator==(const Sender& other) const
									{ return mT == ((const RSender<T>&) other).mT; }

			// Properties
			private:
				const	T&	mT;
		};

	// NoSender - Internal use only
	private:
		class NoSender : public Sender {
			// Methods
				private:
							// Lifecycle methods
							NoSender() : Sender() {}

				protected:
							// Sender methods
					bool	operator==(const Sender& other) const
								{ return false; }

			// Properties
			public:
				static	const	NoSender	mNoSender;
		};

	// Observer
	public:
		struct Observer {
			// Procs
			typedef	void	(*Proc)(const CString& notificationName, const Sender& sender, const CDictionary& info,
									void* userData);

					// Lifecycle methods
					Observer(const void* observerRef, Proc proc, void* userData) :
						mObserverRef(observerRef), mProc(proc), mUserData(userData)
						{}
					Observer(const Observer& other) :
						mObserverRef(other.mObserverRef), mProc(other.mProc), mUserData(other.mUserData)
						{}

					// Instance methods
			void	callProc(const CString& notificationName, const Sender& sender, const CDictionary& info) const
						{ mProc(notificationName, sender, info, mUserData); }

			// Properties
			const	void*	mObserverRef;
					Proc	mProc;
					void*	mUserData;
		};

	// Classes
	private:
		class Internals;

	// Methods
	public:
						// Lifcycle methods
		virtual			~CNotificationCenter();

						// Instance methods
				void	registerObserver(const CString& notificationName, const Sender& sender,
								const Observer& observer);
				void	registerObserver(const CString& notificationName, const Observer& observer)
							{ registerObserver(notificationName, NoSender::mNoSender, observer); }
				void	unregisterObserver(const CString& notificationName, const void* observerRef);
				void	unregisterObserver(const void* observerRef);

		virtual	void	queue(const CString& notificationName, const Sender& sender, const CDictionary& info) = 0;
				void	queue(const CString& notificationName, const Sender& sender)
							{ queue(notificationName, sender, CDictionary::mEmpty); }
				void	queue(const CString& notificationName, const CDictionary& info)
							{ queue(notificationName, NoSender::mNoSender, info); }
				void	queue(const CString& notificationName)
							{ queue(notificationName, NoSender::mNoSender, CDictionary::mEmpty); }

	protected:
						// Lifcycle methods
						CNotificationCenter();

						// Instance methods
				void	send(const CString& notificationName, const Sender& sender, const CDictionary& info) const;

	// Properties
	private:
		Internals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CImmediateNotificationCenter

class CImmediateNotificationCenter : public CNotificationCenter {
	// Methods
	public:
				// Lifecycle methods
				CImmediateNotificationCenter() {}

				// CNotificationCenter methods
		void	queue(const CString& notificationName, const Sender& sender, const CDictionary& info)
					{ send(notificationName, sender, info); }
		void	queue(const CString& notificationName, const Sender& sender)
					{ CNotificationCenter::queue(notificationName, sender); }
		void	queue(const CString& notificationName, const CDictionary& info)
					{ CNotificationCenter::queue(notificationName, info); }
		void	queue(const CString& notificationName)
					{ CNotificationCenter::queue(notificationName); }
};
