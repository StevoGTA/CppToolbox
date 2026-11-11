//----------------------------------------------------------------------------------------------------------------------
//	CTCPConnection-POSIX.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CTCPConnection.h"

#include "SError-POSIX.h"

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CTCPConnection::Internals

class CTCPConnection::Internals {
	public:
		Internals(const CString& host, UInt16 port) : mHost(host), mPort(port), mSocket(-1) {}

		CString	mHost;
		UInt16	mPort;
		int		mSocket;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CTCPConnection

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CTCPConnection::CTCPConnection(const CString& host, UInt16 port)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(host, port);
}

//----------------------------------------------------------------------------------------------------------------------
CTCPConnection::~CTCPConnection()
//----------------------------------------------------------------------------------------------------------------------
{
	// Cleanup
	close();

	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CTCPConnection::connect()
//----------------------------------------------------------------------------------------------------------------------
{
	// Resolve host
	struct addrinfo	hints = {0};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	struct addrinfo*	addressInfos;
	int					result =
								::getaddrinfo(*mInternals->mHost.getUTF8String(),
										*CString(mInternals->mPort).getUTF8String(), &hints, &addressInfos);
	if (result != 0)
		// Error
		return OV<SError>(
				SError(CString(OSSTR("getaddrinfo")), result,
						CString(::gai_strerror(result), ::strlen(::gai_strerror(result)), CString::kEncodingUTF8)));

	// Connect to the first address that accepts
	int	error = 0;
	for (struct addrinfo* addressInfo = addressInfos; addressInfo != nil; addressInfo = addressInfo->ai_next) {
		// Create socket
		int	socketDescriptor = ::socket(addressInfo->ai_family, addressInfo->ai_socktype, addressInfo->ai_protocol);
		if (socketDescriptor < 0) {
			// Error
			error = errno;
			continue;
		}

		// Connect
		if (::connect(socketDescriptor, addressInfo->ai_addr, addressInfo->ai_addrlen) == 0) {
			// Success
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
			// Report a write after the other end closes as an error rather than raising SIGPIPE
			int	value = 1;
			::setsockopt(socketDescriptor, SOL_SOCKET, SO_NOSIGPIPE, &value, sizeof(value));
#endif
			mInternals->mSocket = socketDescriptor;
			break;
		}

		// Error
		error = errno;
		::close(socketDescriptor);
	}
	::freeaddrinfo(addressInfos);

	return (mInternals->mSocket >= 0) ? OV<SError>() : OV<SError>(SErrorFromPOSIXerror(error));
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> CTCPConnection::read(UInt32 maximumByteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	CData	data((CData::ByteCount) maximumByteCount, 0);
	ssize_t	byteCount;
	do {
		// Receive
		byteCount = ::recv(mInternals->mSocket, *data.getMutableBuffer(0, maximumByteCount), maximumByteCount, 0);
	} while ((byteCount < 0) && (errno == EINTR));

	return (byteCount >= 0) ?
			TVResult<CData>(data.subData(0, byteCount)) : TVResult<CData>(SErrorFromPOSIXerror(errno));
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CTCPConnection::write(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	// Write all bytes
	TBuffer<const UInt8>	buffer = data.getUInt8Buffer();
	CData::ByteIndex		byteIndex = 0;
	while (byteIndex < data.getByteCount()) {
		// Send
		ssize_t	byteCount = ::send(mInternals->mSocket, *buffer + byteIndex, data.getByteCount() - byteIndex, 0);
		if (byteCount >= 0)
			// Sent
			byteIndex += byteCount;
		else if (errno != EINTR)
			// Error
			return OV<SError>(SErrorFromPOSIXerror(errno));
	}

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
void CTCPConnection::close()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if open
	if (mInternals->mSocket >= 0) {
		// Close
		::close(mInternals->mSocket);
		mInternals->mSocket = -1;
	}
}
