//----------------------------------------------------------------------------------------------------------------------
//	CQueue.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CEquatable.h"
#include "TBuffer.h"
#include "TWrappers.h"

/*
	Terminology:
		SR - Single Reader
		SW - Single Writer
		BIP - Bip Buffer
*/

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSRSWBIPQueue

class CSRSWBIPQueue : public CEquatable {
	// Structs
	public:
		struct ReadBufferInfo {
					// Lifecycle methods
					ReadBufferInfo(const void* buffer, UInt32 byteCount) : mBuffer(buffer), mByteCount(byteCount) {}
					ReadBufferInfo() : mBuffer(nil), mByteCount(0) {}
					ReadBufferInfo(const ReadBufferInfo& other) :
						mBuffer(other.mBuffer), mByteCount(other.mByteCount)
						{}

					// Instance methods
			bool	hasBuffer() const
						{ return mByteCount > 0; }

			// Properties
			const	void*	mBuffer;
					UInt32	mByteCount;
		};

		struct WriteBufferInfo {
					// Lifecycle methods
					WriteBufferInfo(void* buffer, UInt32 byteCount) : mBuffer(buffer), mByteCount(byteCount) {}
					WriteBufferInfo() : mBuffer(nil), mByteCount(0) {}
					WriteBufferInfo(const WriteBufferInfo& other) :
						mBuffer(other.mBuffer), mByteCount(other.mByteCount)
						{}

					// Instance methods
			bool	hasBuffer() const
						{ return mByteCount > 0; }

			// Properties
			void*	mBuffer;
			UInt32	mByteCount;
		};

	// Classes
	private:
		class Internals;

	// Methods
	public:
								// CEquatable methods
				bool			operator==(const CEquatable& other) const
									{ return this == &other; }

	protected:
								// Lifecycle methods
								CSRSWBIPQueue(UInt32 byteCount);
								CSRSWBIPQueue(const CSRSWBIPQueue& other);
		virtual					~CSRSWBIPQueue();

								// Instance methods
				ReadBufferInfo	requestRead() const;
		virtual	void			commitRead(UInt32 byteCount);

				WriteBufferInfo	requestWrite(UInt32 requiredByteCount) const;
		virtual	void			commitWrite(UInt32 byteCount);

				void			reset();

	// Properties
	private:
		Internals*	mInternals;
 };

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TSRSWBIPQueue

template <typename T> class TSRSWBIPQueue : protected CSRSWBIPQueue {
	// Methods
	public:
								// Lifecycle methods
								TSRSWBIPQueue(UInt32 elementCount) :
									CSRSWBIPQueue(elementCount * sizeof(T)), mReadBuffer(nil, 0), mWriteBuffer(nil, 0)
									{}

								// Instance methods
		OR<const TBuffer<T> >	requestRead()
									{
										// Check situation
										ReadBufferInfo	readBufferInfo = CSRSWBIPQueue::requestRead();
										if (readBufferInfo.hasBuffer()) {
											// Can read
											mReadBuffer =
													TBuffer<T>((T*) readBufferInfo.mBuffer, readBufferInfo.mByteCount);

											return OR<const TBuffer<T> >(mReadBuffer);
										} else
											// Nothing to read
											return OR<const TBuffer<T> >();
									}
		void					commitRead(UInt32 elementCount)
									{ CSRSWBIPQueue::commitRead(elementCount * sizeof(T)); }

		OR<TBuffer<T> >			requestWrite(UInt32 elementCount)
									{
										// Check situation
										WriteBufferInfo	writeBufferInfo =
																CSRSWBIPQueue::requestWrite(elementCount * sizeof(T));
										if (writeBufferInfo.hasBuffer()) {
											// Can write
											mWriteBuffer =
													TBuffer<T>((T*) writeBufferInfo.mBuffer,
															writeBufferInfo.mByteCount);

											return OR<TBuffer<T> >(mWriteBuffer);
										} else
											// Not enough space
											return OR<TBuffer<T> >();
									}
		void					commitWrite(UInt32 elementCount)
									{ CSRSWBIPQueue::commitWrite(elementCount * sizeof(T)); }

	// Properties
	private:
		TBuffer<T>	mReadBuffer;
		TBuffer<T>	mWriteBuffer;
 };

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSRSWBIPSegmentedQueue

class CSRSWBIPSegmentedQueue {
	// Structs
	public:
		struct ReadBufferInfo {
					// Lifecycle methods
					ReadBufferInfo(const void* buffer, UInt32 segmentByteCount, UInt32 byteCount) :
						mBuffer(buffer), mSegmentByteCount(segmentByteCount), mByteCount(byteCount)
						{}
					ReadBufferInfo() : mBuffer(nil), mSegmentByteCount(0), mByteCount(0) {}
					ReadBufferInfo(const ReadBufferInfo& other) :
						mBuffer(other.mBuffer), mSegmentByteCount(other.mSegmentByteCount), mByteCount(other.mByteCount)
						{}

					// Instance methods
			bool	hasBuffer() const
						{ return mByteCount > 0; }
			void*	bufferAtIndex(UInt32 index) const
						{ return (UInt8*) mBuffer + index * mSegmentByteCount; }

			// Properties
			const	void*	mBuffer;
					UInt32	mSegmentByteCount;
					UInt32	mByteCount;
		};

		struct WriteBufferInfo {
					// Lifecycle methods
					WriteBufferInfo(void* buffer, UInt32 segmentByteCount, UInt32 byteCount) :
						mBuffer(buffer), mSegmentByteCount(segmentByteCount), mByteCount(byteCount)
						{}
					WriteBufferInfo() : mBuffer(nil), mSegmentByteCount(0), mByteCount(0) {}
					WriteBufferInfo(const WriteBufferInfo& other) :
						mBuffer(other.mBuffer), mSegmentByteCount(other.mSegmentByteCount), mByteCount(other.mByteCount)
						{}

					// Instance methods
			bool	hasBuffer() const
						{ return mByteCount > 0; }

			// Properties
			void*	mBuffer;
			UInt32	mSegmentByteCount;
			UInt32	mByteCount;
		};

	// Classes
	private:
		class Internals;

	// Methods
	public:
								// Lifecycle methods
								CSRSWBIPSegmentedQueue(UInt32 segmentCount, UInt32 segmentByteCount);
		virtual					~CSRSWBIPSegmentedQueue();

								// Instance methods
				UInt32			getSegmentCount() const;
				
				ReadBufferInfo	requestRead() const;
		virtual	void			commitRead(UInt32 byteCount);

				WriteBufferInfo	requestWrite(UInt32 requiredByteCount) const;
		virtual	void			commitWrite(UInt32 byteCount);

				void			reset();

	// Properties
	private:
		Internals*	mInternals;
 };

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSRSWMessageQueue

class CSRSWMessageQueue : public CSRSWBIPQueue {
	// Structs
	public:
		class Message {
			// Methods
			public:
								// Lifecycle methods
								Message(OSType type, UInt32 byteCount) : mType(type), mByteCount(byteCount) {}
				virtual			~Message()
									{}

								// Instance methods
						OSType	getType() const
									{ return mType; }
						UInt32	getByteCount() const
									{ return mByteCount; }

			// Properties
			private:
				OSType	mType;
				UInt32	mByteCount;
		};

		class ProcMessage : public Message {
			// Procs
			public:
				typedef	void	(*Proc)(ProcMessage& message, void* userData);

			// Methods
			public:
								// Lifecycle methods
								ProcMessage(Proc proc, void* userData) :
									Message(mType, sizeof(ProcMessage)), mProc(proc), mUserData(userData)
									{}
								ProcMessage(UInt32 byteCount, Proc proc, void* userData) :
									Message(mType, byteCount), mProc(proc), mUserData(userData)
									{}

								// Instance methods
						void	perform()
									{ mProc(*this, mUserData); }
				virtual	void	cleanup()
									{}

			// Properties
			public:
				static	const	OSType	mType;

			private:
								Proc	mProc;
								void*	mUserData;
		};

	// Methods
	public:
						// Lifecycle methods
						CSRSWMessageQueue(UInt32 byteCount);
						CSRSWMessageQueue(const CSRSWMessageQueue& other);

						// Instance methods
				void	handleAll();

		virtual	void	submit(const Message& message);

						// Subclass methods
		virtual	void	handle(const Message& message);
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSRSWMessageQueues

class CSRSWMessageQueues {
	// Classes
	private:
		class Internals;

	// Methods
	public:
				// Lifecycle methods
				CSRSWMessageQueues();
				~CSRSWMessageQueues();

				// Instance methods
		void	add(CSRSWMessageQueue& messageQueue);
		void	remove(CSRSWMessageQueue& messageQueue);

		void	handleAll();

	// Properties
	private:
		Internals*	mInternals;
};
