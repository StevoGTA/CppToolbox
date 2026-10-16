//----------------------------------------------------------------------------------------------------------------------
//	CMessageChannel.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMessageChannel.h"

#include "CBinaryPropertyList.h"

#if defined(TARGET_OS_MACOS)
	#include "SError-POSIX.h"

	#include <fcntl.h>
	#include <poll.h>
	#include <unistd.h>
#elif defined(TARGET_OS_WINDOWS)
	#include "SError-Windows.h"
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sErrorDomain(OSSTR("CMessageChannel"));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMessageChannel::Internals

#if defined(TARGET_OS_MACOS)
class CMessageChannel::Internals {
	public:
							Internals(Handle readHandle, Handle writeHandle) :
								mReadHandle(readHandle), mWriteHandle(writeHandle)
								{
									// A write after the other end has closed reports an error instead of ending this
									//	process
									::fcntl(mWriteHandle, F_SETNOSIGPIPE, 1);

									// A write returns what fits instead of blocking, so a send can stop at its deadline
									::fcntl(mWriteHandle, F_SETFL, ::fcntl(mWriteHandle, F_GETFL) | O_NONBLOCK);
								}
							~Internals()
								{ close(); }

				bool		isOpen() const
								{ return mReadHandle != -1; }
				void		close()
								{
									// Check if open
									if (mReadHandle == -1)
										// Already closed
										return;

									// Close
									if (mWriteHandle != mReadHandle)
										// Write handle
										::close(mWriteHandle);
									::close(mReadHandle);

									mReadHandle = -1;
									mWriteHandle = -1;
								}

				OV<SError>	read(void* buffer, UInt64 byteCount, const OV<UniversalTime>& deadline)
								{
									// Read all bytes
									UInt8*	bytePtr = (UInt8*) buffer;
									while (byteCount > 0) {
										// Wait for bytes
										OV<SError>	error = waitFor(mReadHandle, POLLIN, deadline);
										ReturnErrorIfError(error);

										// Read
										ssize_t	readByteCount = ::read(mReadHandle, bytePtr, byteCount);
										if (readByteCount > 0) {
											// Advance
											bytePtr += readByteCount;
											byteCount -= readByteCount;
										} else if (readByteCount == 0)
											// Other end has closed
											return OV<SError>(mClosedError);
										else if ((errno != EINTR) && (errno != EAGAIN))
											// Error
											return OV<SError>(SErrorFromPOSIXerror(errno));
									}

									return OV<SError>();
								}
				OV<SError>	write(const CData& data, const OV<UniversalTime>& deadline)
								{
									// Write all bytes
									const	UInt8*				bytePtr = *data.getUInt8Buffer();
											CData::ByteCount	byteCount = data.getByteCount();
									while (byteCount > 0) {
										// Wait for room
										OV<SError>	error = waitFor(mWriteHandle, POLLOUT, deadline);
										ReturnErrorIfError(error);

										// Write
										ssize_t	writtenByteCount = ::write(mWriteHandle, bytePtr, byteCount);
										if (writtenByteCount >= 0) {
											// Advance
											bytePtr += writtenByteCount;
											byteCount -= writtenByteCount;
										} else if (errno == EPIPE)
											// Other end has closed
											return OV<SError>(mClosedError);
										else if ((errno != EINTR) && (errno != EAGAIN))
											// Error
											return OV<SError>(SErrorFromPOSIXerror(errno));
									}

									return OV<SError>();
								}

		static	OV<SError>	waitFor(Handle handle, short events, const OV<UniversalTime>& deadline)
								{
									// Wait until ready, or until the deadline if have one
									while (true) {
										// Compose timeout (-1 waits forever)
										int	timeout = -1;
										if (deadline.hasValue()) {
											// Until the deadline
											UniversalTimeInterval	remaining =
																			*deadline - SUniversalTime::getCurrent();
											timeout = (remaining > 0.0) ? (int) (remaining * 1000.0) + 1 : 0;
										}

										// Wait
										struct	pollfd	pollFileDescriptor = {handle, events, 0};
												int		pollResult = ::poll(&pollFileDescriptor, 1, timeout);
										if (pollResult > 0)
											// Ready, or the other end has closed, which the read or write then reports
											return OV<SError>();
										else if (pollResult == 0)
											// Timed out
											return OV<SError>(mTimedOutError);
										else if (errno != EINTR)
											// Error
											return OV<SError>(SErrorFromPOSIXerror(errno));
									}
								}

		Handle	mReadHandle;
		Handle	mWriteHandle;
};
#elif defined(TARGET_OS_WINDOWS)
class CMessageChannel::Internals {
	public:
							Internals(Handle readHandle, Handle writeHandle) :
								mReadHandle(readHandle), mWriteHandle(writeHandle),
										mReadEvent(::CreateEvent(NULL, TRUE, FALSE, NULL)),
										mWriteEvent(::CreateEvent(NULL, TRUE, FALSE, NULL))
								{}
							~Internals()
								{
									// Cleanup
									close();
									::CloseHandle(mReadEvent);
									::CloseHandle(mWriteEvent);
								}

				bool		isOpen() const
								{ return mReadHandle != NULL; }
				void		close()
								{
									// Check if open
									if (mReadHandle == NULL)
										// Already closed
										return;

									// Close
									if (mWriteHandle != mReadHandle)
										// Write handle
										::CloseHandle(mWriteHandle);
									::CloseHandle(mReadHandle);

									mReadHandle = NULL;
									mWriteHandle = NULL;
								}

				OV<SError>	read(void* buffer, UInt64 byteCount, const OV<UniversalTime>& deadline)
								{
									// Read all bytes
									UInt8*	bytePtr = (UInt8*) buffer;
									while (byteCount > 0) {
										// Start read
										OVERLAPPED	overlapped = {};
										overlapped.hEvent = mReadEvent;
										if (!::ReadFile(mReadHandle, bytePtr,
														(byteCount < 0x40000000) ? (DWORD) byteCount : 0x40000000,
														NULL, &overlapped) &&
												(::GetLastError() != ERROR_IO_PENDING))
											// Error
											return OV<SError>(getError(::GetLastError()));

										// Wait for it to complete
										DWORD		readByteCount = 0;
										OV<SError>	error = waitFor(mReadHandle, overlapped, readByteCount, deadline);
										ReturnErrorIfError(error);

										// Advance
										bytePtr += readByteCount;
										byteCount -= readByteCount;
									}

									return OV<SError>();
								}
				OV<SError>	write(const CData& data, const OV<UniversalTime>& deadline)
								{
									// Write all bytes
									const	UInt8*				bytePtr = *data.getUInt8Buffer();
											CData::ByteCount	byteCount = data.getByteCount();
									while (byteCount > 0) {
										// Start write
										OVERLAPPED	overlapped = {};
										overlapped.hEvent = mWriteEvent;
										if (!::WriteFile(mWriteHandle, bytePtr,
														(byteCount < 0x40000000) ? (DWORD) byteCount : 0x40000000,
														NULL, &overlapped) &&
												(::GetLastError() != ERROR_IO_PENDING))
											// Error
											return OV<SError>(getError(::GetLastError()));

										// Wait for it to complete
										DWORD		writtenByteCount = 0;
										OV<SError>	error = waitFor(mWriteHandle, overlapped, writtenByteCount, deadline);
										ReturnErrorIfError(error);

										// Advance
										bytePtr += writtenByteCount;
										byteCount -= writtenByteCount;
									}

									return OV<SError>();
								}

		static	OV<SError>	waitFor(Handle handle, OVERLAPPED& overlapped, DWORD& byteCount,
									const OV<UniversalTime>& deadline)
								{
									// Compose timeout
									DWORD	timeout = INFINITE;
									if (deadline.hasValue()) {
										// Until the deadline
										UniversalTimeInterval	remaining = *deadline - SUniversalTime::getCurrent();
										timeout = (remaining > 0.0) ? (DWORD) (remaining * 1000.0) : 0;
									}

									// Wait for the operation to complete; cancel it if the deadline passes
									if (::WaitForSingleObject(overlapped.hEvent, timeout) == WAIT_TIMEOUT) {
										// Timed out
										::CancelIoEx(handle, &overlapped);
										::GetOverlappedResult(handle, &overlapped, &byteCount, TRUE);

										return OV<SError>(mTimedOutError);
									} else if (!::GetOverlappedResult(handle, &overlapped, &byteCount, FALSE))
										// Error
										return OV<SError>(getError(::GetLastError()));

									return OV<SError>();
								}
		static	SError		getError(DWORD error)
								{ return ((error == ERROR_BROKEN_PIPE) || (error == ERROR_NO_DATA) ||
												(error == ERROR_PIPE_NOT_CONNECTED)) ?
										mClosedError : SErrorFromWindowsError(error); }

		Handle	mReadHandle;
		Handle	mWriteHandle;

		HANDLE	mReadEvent;
		HANDLE	mWriteEvent;
};
#endif

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMessageChannel

// MARK: Properties

const	SError	CMessageChannel::mClosedError(sErrorDomain, 1, CString(OSSTR("Closed")));
const	SError	CMessageChannel::mTimedOutError(sErrorDomain, 2, CString(OSSTR("Timed out")));
const	SError	CMessageChannel::mInvalidMessageError(sErrorDomain, 3, CString(OSSTR("Invalid message")));

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CMessageChannel::CMessageChannel(Handle readHandle, Handle writeHandle)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(readHandle, writeHandle);
}

//----------------------------------------------------------------------------------------------------------------------
CMessageChannel::~CMessageChannel()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CMessageChannel::send(const CDictionary& message, const OV<UniversalTimeInterval>& timeout)
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose data
	TVResult<CData>	data = CBinaryPropertyList::dataFrom(message);
	ReturnErrorIfResultError(data);

	// Check if open
	if (!mInternals->isOpen())
		// Closed
		return OV<SError>(mClosedError);

	// Write byte count and data together
	UInt32	storedByteCount = EndianU32_NtoB((UInt32) data->getByteCount());

	return mInternals->write(CData(&storedByteCount, sizeof(UInt32)) + *data,
			timeout.hasValue() ? OV<UniversalTime>(SUniversalTime::getCurrent() + *timeout) : OV<UniversalTime>());
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CDictionary> CMessageChannel::receive(const OV<UniversalTimeInterval>& timeout)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if open
	if (!mInternals->isOpen())
		// Closed
		return TVResult<CDictionary>(mClosedError);

	// Setup
	OV<UniversalTime>	deadline =
								timeout.hasValue() ?
										OV<UniversalTime>(SUniversalTime::getCurrent() + *timeout) :
										OV<UniversalTime>();

	// Read byte count
	UInt32		storedByteCount;
	OV<SError>	error = mInternals->read(&storedByteCount, sizeof(UInt32), deadline);
	ReturnValueIfError(error, TVResult<CDictionary>(*error));

	// Read data
	UInt32	byteCount = EndianU32_BtoN(storedByteCount);
	CData	data;
	error = mInternals->read(*data.getMutableBuffer(byteCount), byteCount, deadline);
	ReturnValueIfError(error, TVResult<CDictionary>(*error));

	// Compose message
	TVResult<CDictionary>	message = CBinaryPropertyList::dictionaryFrom(data);

	return message.hasError() ? TVResult<CDictionary>(mInvalidMessageError) : message;
}

//----------------------------------------------------------------------------------------------------------------------
void CMessageChannel::close()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->close();
}
