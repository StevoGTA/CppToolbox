//----------------------------------------------------------------------------------------------------------------------
//	CTCPConnection-Windows.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CTCPConnection.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include "SError-Windows.h"

#pragma comment(lib, "ws2_32")

//----------------------------------------------------------------------------------------------------------------------
// MARK: CTCPConnection::Internals

class CTCPConnection::Internals {
	public:
		Internals(const CString& host, UInt16 port) : mHost(host), mPort(port), mSocket(INVALID_SOCKET) {}

		CString	mHost;
		UInt16	mPort;
		SOCKET	mSocket;
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
	// Start Winsock once for the process
	static	int	sStartupResult = -1;
	if (sStartupResult != 0) {
		// Start
		WSADATA	wsaData;
		sStartupResult = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (sStartupResult != 0)
			// Error
			return OV<SError>(SErrorFromWindowsError(sStartupResult));
	}

	// Resolve host
	ADDRINFOW	hints = {0};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	ADDRINFOW*	addressInfos;
	int			result =
						::GetAddrInfoW(mInternals->mHost.getOSString(), CString(mInternals->mPort).getOSString(),
								&hints, &addressInfos);
	if (result != 0)
		// Error
		return OV<SError>(SErrorFromWindowsError(result));

	// Connect to the first address that accepts
	int	error = 0;
	for (ADDRINFOW* addressInfo = addressInfos; addressInfo != nil; addressInfo = addressInfo->ai_next) {
		// Create socket
		SOCKET	socketDescriptor = ::socket(addressInfo->ai_family, addressInfo->ai_socktype, addressInfo->ai_protocol);
		if (socketDescriptor == INVALID_SOCKET) {
			// Error
			error = ::WSAGetLastError();
			continue;
		}

		// Connect
		if (::connect(socketDescriptor, addressInfo->ai_addr, (int) addressInfo->ai_addrlen) == 0) {
			// Success
			mInternals->mSocket = socketDescriptor;
			break;
		}

		// Error
		error = ::WSAGetLastError();
		::closesocket(socketDescriptor);
	}
	::FreeAddrInfoW(addressInfos);

	return (mInternals->mSocket != INVALID_SOCKET) ? OV<SError>() : OV<SError>(SErrorFromWindowsError(error));
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> CTCPConnection::read(UInt32 maximumByteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	CData	data((CData::ByteCount) maximumByteCount, 0);
	int		byteCount =
					::recv(mInternals->mSocket, (char*) *data.getMutableBuffer(0, maximumByteCount),
							(int) maximumByteCount, 0);

	return (byteCount != SOCKET_ERROR) ?
			TVResult<CData>(data.subData(0, byteCount)) : TVResult<CData>(SErrorFromWindowsError(::WSAGetLastError()));
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
		int	byteCount =
					::send(mInternals->mSocket, (const char*) *buffer + byteIndex,
							(int) (data.getByteCount() - byteIndex), 0);
		if (byteCount == SOCKET_ERROR)
			// Error
			return OV<SError>(SErrorFromWindowsError(::WSAGetLastError()));

		// Sent
		byteIndex += byteCount;
	}

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
void CTCPConnection::close()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if open
	if (mInternals->mSocket != INVALID_SOCKET) {
		// Close
		::closesocket(mInternals->mSocket);
		mInternals->mSocket = INVALID_SOCKET;
	}
}
