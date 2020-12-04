//----------------------------------------------------------------------------------------------------------------------
//	CQueue.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "TBuffer.h"
#include "TOptional.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSRSWBIPQueue

class CSRSWBIPQueueInternals;
class CSRSWBIPQueue {
	// Structs
	public:
		struct SReadBufferInfo {
					// Lifecycle methods
					SReadBufferInfo(const void* buffer, UInt32 size) : mBuffer(buffer), mSize(size) {}
					SReadBufferInfo() : mBuffer(nil), mSize(0) {}
					SReadBufferInfo(const SReadBufferInfo& other) : mBuffer(other.mBuffer), mSize(other.mSize) {}

					// Instance methods
			bool	hasBuffer() const
						{ return mSize > 0; }

			// Properties
			const	void*	mBuffer;
					UInt32	mSize;
		};

		struct SWriteBufferInfo {
					// Lifecycle methods
					SWriteBufferInfo(void* buffer, UInt32 size) : mBuffer(buffer), mSize(size) {}
					SWriteBufferInfo() : mBuffer(nil), mSize(0) {}
					SWriteBufferInfo(const SWriteBufferInfo& other) : mBuffer(other.mBuffer), mSize(other.mSize) {}

					// Instance methods
			bool	hasBuffer() const
						{ return mSize > 0; }

			// Properties
			void*	mBuffer;
			UInt32	mSize;
		};

	// Methods
	public:
									// Instance methods
				void				reset();

	protected:
									// Lifecycle methods
									CSRSWBIPQueue(UInt32 size);
		virtual						~CSRSWBIPQueue();

									// Instance methods
				SReadBufferInfo		requestRead() const;
		virtual	void				commitRead(UInt32 size);

				SWriteBufferInfo	requestWrite(UInt32 maxSize) const;
		virtual	void				commitWrite(UInt32 size);

	// Properties
	private:
		CSRSWBIPQueueInternals*	mInternals;
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
		OR<const TBuffer<T> >	requestRead() const
									{
										// Check situation
										SReadBufferInfo	readBufferInfo = CSRSWBIPQueue::requestRead();
										if (readBufferInfo.hasBuffer()) {
											// Can read
											mReadBuffer =
													TBuffer<T>((T*) readBufferInfo.mBuffer, readBufferInfo.mSize);

											return OR<const TBuffer<T> >(mReadBuffer);
										} else
											// Nothing to read
											return OR<const TBuffer<T> >();
									}
		void					commitRead(UInt32 elementCount)
									{ CSRSWBIPQueue::commitRead(elementCount * sizeof(T)); }

		OR<TBuffer<T> >			requestWrite(UInt32 elementCount) const
									{
										// Check situation
										SWriteBufferInfo	writeBufferInfo =
																	CSRSWBIPQueue::requestWrite(
																			elementCount * sizeof(T));
										if (writeBufferInfo.hasBuffer()) {
											// Can write
											mWriteBuffer =
													TBuffer<T>((T*) writeBufferInfo.mBuffer, writeBufferInfo.mSize);

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

class CSRSWBIPSegmentedQueueInternals;
class CSRSWBIPSegmentedQueue {
	// Structs
	public:
		struct SReadBufferInfo {
					// Lifecycle methods
					SReadBufferInfo(const void* buffer, UInt32 segmentSize, UInt32 size) :
						mBuffer(buffer), mSegmentSize(segmentSize), mSize(size)
						{}
					SReadBufferInfo() : mBuffer(nil), mSegmentSize(0), mSize(0) {}
					SReadBufferInfo(const SReadBufferInfo& other) :
						mBuffer(other.mBuffer), mSegmentSize(other.mSegmentSize), mSize(other.mSize)
						{}

					// Instance methods
			bool	hasBuffer() const
						{ return mSize > 0; }
			void*	bufferAtIndex(UInt32 index) const
						{ return (UInt8*) mBuffer + index * mSegmentSize; }

			// Properties
			const	void*	mBuffer;
					UInt32	mSegmentSize;
					UInt32	mSize;
		};

		struct SWriteBufferInfo {
					// Lifecycle methods
					SWriteBufferInfo(void* buffer, UInt32 segmentSize, UInt32 size) :
						mBuffer(buffer), mSegmentSize(segmentSize), mSize(size)
						{}
					SWriteBufferInfo() : mBuffer(nil), mSegmentSize(0), mSize(0) {}
					SWriteBufferInfo(const SWriteBufferInfo& other) :
						mBuffer(other.mBuffer), mSegmentSize(other.mSegmentSize), mSize(other.mSize)
						{}

					// Instance methods
			bool	hasBuffer() const
						{ return mSize > 0; }

			// Properties
			void*	mBuffer;
			UInt32	mSegmentSize;
			UInt32	mSize;
		};

	// Methods
	public:
									// Lifecycle methods
									CSRSWBIPSegmentedQueue(UInt32 segmentSize, UInt32 segmentCount);
		virtual						~CSRSWBIPSegmentedQueue();

									// Instance methods
				UInt32				getSegmentCount() const;
				
				SReadBufferInfo		requestRead() const;
		virtual	void				commitRead(UInt32 size);

				SWriteBufferInfo	requestWrite(UInt32 maxSize) const;
		virtual	void				commitWrite(UInt32 size);

				void				reset();

	// Properties
	private:
		CSRSWBIPSegmentedQueueInternals*	mInternals;
 };
