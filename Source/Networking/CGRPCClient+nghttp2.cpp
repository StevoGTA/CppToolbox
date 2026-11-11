//----------------------------------------------------------------------------------------------------------------------
//	CGRPCClient+nghttp2.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CGRPCClient.h"

#include "CTCPConnection.h"

#define NGHTTP2_NO_SSIZE_T
#define NGHTTP2_STATICLIB
#include <nghttp2/nghttp2.h>

//	Implements CGRPCClient with nghttp2 1.71.0 and CTCPConnection.  A project using it adds:
//		nghttp2's lib/ sources:
//			nghttp2_alpn.c, nghttp2_buf.c, nghttp2_callbacks.c, nghttp2_debug.c, nghttp2_extpri.c, nghttp2_frame.c,
//			nghttp2_hd.c, nghttp2_hd_huffman.c, nghttp2_hd_huffman_data.c, nghttp2_helper.c, nghttp2_http.c,
//			nghttp2_map.c, nghttp2_mem.c, nghttp2_option.c, nghttp2_outbound_item.c, nghttp2_pq.c,
//			nghttp2_priority_spec.c, nghttp2_queue.c, nghttp2_ratelim.c, nghttp2_rcbuf.c, nghttp2_session.c,
//			nghttp2_stream.c, nghttp2_submit.c, nghttp2_time.c, nghttp2_version.c, sfparse.c
//		built with HAVE_CONFIG_H, BUILDING_NGHTTP2 and NGHTTP2_STATICLIB defined
//		nghttp2's lib/includes folder, and the folder with the platform's config.h, on the header search path
//		CTCPConnection-POSIX.cpp (Apple) or CTCPConnection-Windows.cpp (Windows)

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	const	CString	sGRPCErrorDomain(OSSTR("gRPC"));

static	const	CString	sErrorDomain(OSSTR("CGRPCClient"));
static	const	SError	sConnectionClosedError(sErrorDomain, 1,
								CString(OSSTR("The connection closed before the reply arrived")));
static	const	SError	sStreamResetError(sErrorDomain, 2, CString(OSSTR("The server reset the stream")));
static	const	SError	sHTTPStatusError(sErrorDomain, 3,
								CString(OSSTR("The server did not reply with HTTP status 200")));
static	const	SError	sMissingGRPCStatusError(sErrorDomain, 4, CString(OSSTR("The reply has no grpc-status")));
static	const	SError	sInvalidReplyError(sErrorDomain, 5, CString(OSSTR("The reply message is invalid")));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGRPCClient::Internals

class CGRPCClient::Internals {
	public:
								Internals(const CString& host, UInt16 port) :
									mHost(host), mPort(port), mRequestBodyByteIndex(0), mStreamID(-1),
											mIsStreamClosed(false), mStreamErrorCode(0)
									{}

		static	SError			errorFrom(int nghttp2Error)
									{
										// Setup
										const	char*	description = ::nghttp2_strerror(nghttp2Error);

										return SError(CString(OSSTR("nghttp2")), nghttp2Error,
												CString(description, ::strlen(description), CString::kEncodingUTF8));
									}

		static	nghttp2_ssize	readRequestBody(nghttp2_session* session, int32_t streamID, uint8_t* buffer,
										size_t length, uint32_t* dataFlags, nghttp2_data_source* source, void* userData)
									{
										// Setup
										Internals&	internals = *((Internals*) userData);
										size_t		byteCount =
															std::min<size_t>(length,
																	internals.mRequestBody.getByteCount() -
																			internals.mRequestBodyByteIndex);

										// Copy
										internals.mRequestBody.copyBytes(buffer, internals.mRequestBodyByteIndex,
												byteCount);
										internals.mRequestBodyByteIndex += byteCount;
										if (internals.mRequestBodyByteIndex == internals.mRequestBody.getByteCount())
											// All sent
											*dataFlags |= NGHTTP2_DATA_FLAG_EOF;

										return byteCount;
									}
		static	int				onHeader(nghttp2_session* session, const nghttp2_frame* frame,
										const uint8_t* name, size_t nameLength, const uint8_t* value,
										size_t valueLength, uint8_t flags, void* userData)
									{
										// Setup
										Internals&	internals = *((Internals*) userData);
										if (frame->hd.stream_id != internals.mStreamID)
											// Not our stream
											return 0;

										// Store the values we use from the headers and trailers
										CString	nameString(name, nameLength, CString::kEncodingUTF8);
										CString	valueString(value, valueLength, CString::kEncodingUTF8);
										if (nameString == CString(OSSTR(":status")))
											// HTTP status
											internals.mHTTPStatus.setValue(valueString);
										else if (nameString == CString(OSSTR("grpc-status")))
											// gRPC status
											internals.mGRPCStatus.setValue(valueString);
										else if (nameString == CString(OSSTR("grpc-message")))
											// gRPC message
											internals.mGRPCMessage.setValue(valueString);

										return 0;
									}
		static	int				onDataChunkReceived(nghttp2_session* session, uint8_t flags, int32_t streamID,
										const uint8_t* data, size_t length, void* userData)
									{
										// Setup
										Internals&	internals = *((Internals*) userData);

										// Check if our stream
										if (streamID == internals.mStreamID)
											// Append
											internals.mResponseBody.append(data, length);

										return 0;
									}
		static	int				onStreamClose(nghttp2_session* session, int32_t streamID, uint32_t errorCode,
										void* userData)
									{
										// Setup
										Internals&	internals = *((Internals*) userData);

										// Check if our stream
										if (streamID == internals.mStreamID) {
											// Closed
											internals.mIsStreamClosed = true;
											internals.mStreamErrorCode = errorCode;
										}

										return 0;
									}

		CString		mHost;
		UInt16		mPort;

		CData		mRequestBody;
		UInt64		mRequestBodyByteIndex;
		int32_t		mStreamID;
		bool		mIsStreamClosed;
		uint32_t	mStreamErrorCode;
		OV<CString>	mHTTPStatus;
		OV<CString>	mGRPCStatus;
		OV<CString>	mGRPCMessage;
		CData		mResponseBody;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGRPCClient

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGRPCClient::CGRPCClient(const CString& host, UInt16 port)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(host, port);
}

//----------------------------------------------------------------------------------------------------------------------
CGRPCClient::~CGRPCClient()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> CGRPCClient::call(const CString& path, const CData& message)
//----------------------------------------------------------------------------------------------------------------------
{
	// Connect
	CTCPConnection	connection(mInternals->mHost, mInternals->mPort);
	OV<SError>		error = connection.connect();
	ReturnValueIfError(error, TVResult<CData>(*error));

	// Create session
	nghttp2_session_callbacks*	callbacks;
	int							result = ::nghttp2_session_callbacks_new(&callbacks);
	if (result != 0)
		// Error
		return TVResult<CData>(Internals::errorFrom(result));
	::nghttp2_session_callbacks_set_on_header_callback(callbacks, Internals::onHeader);
	::nghttp2_session_callbacks_set_on_data_chunk_recv_callback(callbacks, Internals::onDataChunkReceived);
	::nghttp2_session_callbacks_set_on_stream_close_callback(callbacks, Internals::onStreamClose);

	nghttp2_session*	session;
	result = ::nghttp2_session_client_new(&session, callbacks, mInternals);
	::nghttp2_session_callbacks_del(callbacks);
	if (result != 0)
		// Error
		return TVResult<CData>(Internals::errorFrom(result));

	// Setup request.  The body is the message with a 5-byte prefix: a byte for not compressed, then the message length
	//	in big endian.
	UInt32	messageByteCount = EndianU32_NtoB((UInt32) message.getByteCount());
	mInternals->mRequestBody =
			CData((CData::ByteCount) 0, 0).append((UInt8) 0).append(messageByteCount).append(message);
	mInternals->mRequestBodyByteIndex = 0;
	mInternals->mIsStreamClosed = false;
	mInternals->mStreamErrorCode = 0;
	mInternals->mHTTPStatus.removeValue();
	mInternals->mGRPCStatus.removeValue();
	mInternals->mGRPCMessage.removeValue();
	mInternals->mResponseBody = CData((CData::ByteCount) 0, 0);

	CString::C	pathC = path.getUTF8String();
	CString::C	authorityC = (mInternals->mHost + CString(OSSTR(":")) + CString(mInternals->mPort)).getUTF8String();
	nghttp2_nv	headers[] = {
					{(uint8_t*) ":method", (uint8_t*) "POST", 7, 4, NGHTTP2_NV_FLAG_NONE},
					{(uint8_t*) ":scheme", (uint8_t*) "http", 7, 4, NGHTTP2_NV_FLAG_NONE},
					{(uint8_t*) ":path", (uint8_t*) *pathC, 5, ::strlen(*pathC), NGHTTP2_NV_FLAG_NONE},
					{(uint8_t*) ":authority", (uint8_t*) *authorityC, 10, ::strlen(*authorityC), NGHTTP2_NV_FLAG_NONE},
					{(uint8_t*) "content-type", (uint8_t*) "application/grpc", 12, 16, NGHTTP2_NV_FLAG_NONE},
					{(uint8_t*) "te", (uint8_t*) "trailers", 2, 8, NGHTTP2_NV_FLAG_NONE},
				};

	nghttp2_data_provider2	dataProvider;
	dataProvider.source.ptr = nil;
	dataProvider.read_callback = Internals::readRequestBody;

	// Submit
	result = ::nghttp2_submit_settings(session, NGHTTP2_FLAG_NONE, nil, 0);
	if (result == 0) {
		// Submit request
		mInternals->mStreamID =
				::nghttp2_submit_request2(session, nil, headers, sizeof(headers) / sizeof(nghttp2_nv), &dataProvider,
						nil);
		if (mInternals->mStreamID < 0)
			// Error
			result = mInternals->mStreamID;
	}
	if (result != 0) {
		// Error
		::nghttp2_session_del(session);

		return TVResult<CData>(Internals::errorFrom(result));
	}

	// Exchange frames until the stream closes
	while (!error.hasValue() && !mInternals->mIsStreamClosed) {
		// Send pending frames
		const	uint8_t*		bytes;
				nghttp2_ssize	byteCount = 0;
		while (!error.hasValue() && ((byteCount = ::nghttp2_session_mem_send2(session, &bytes)) > 0))
			// Write
			error = connection.write(CData(bytes, byteCount, false));
		if (error.hasValue())
			// Error
			break;
		if (byteCount < 0) {
			// Error
			error.setValue(Internals::errorFrom((int) byteCount));
			break;
		}

		// Receive
		TVResult<CData>	data = connection.read(16384);
		if (!data.hasValue())
			// Error
			error.setValue(data.getError());
		else if (data->isEmpty())
			// Closed
			error.setValue(sConnectionClosedError);
		else {
			// Process
			byteCount = ::nghttp2_session_mem_recv2(session, *data->getUInt8Buffer(), data->getByteCount());
			if (byteCount < 0)
				// Error
				error.setValue(Internals::errorFrom((int) byteCount));
		}
	}

	// Cleanup
	::nghttp2_session_del(session);
	ReturnValueIfError(error, TVResult<CData>(*error));

	// Check status
	if (mInternals->mStreamErrorCode != 0)
		// Stream was reset
		return TVResult<CData>(sStreamResetError);
	if (!mInternals->mHTTPStatus.hasValue() || (*mInternals->mHTTPStatus != CString(OSSTR("200"))))
		// HTTP error
		return TVResult<CData>(sHTTPStatusError);
	if (!mInternals->mGRPCStatus.hasValue())
		// No gRPC status
		return TVResult<CData>(sMissingGRPCStatusError);
	if (*mInternals->mGRPCStatus != CString(OSSTR("0")))
		// gRPC error
		return TVResult<CData>(
				SError(sGRPCErrorDomain, mInternals->mGRPCStatus->getSInt32(),
						mInternals->mGRPCMessage.getValue(CString::mEmpty)));

	// Get reply message
	const	CData&	responseBody = mInternals->mResponseBody;
	if (responseBody.getByteCount() < 5)
		// Invalid
		return TVResult<CData>(sInvalidReplyError);
	UInt32	replyByteCount;
	responseBody.copyBytes(&replyByteCount, 1, sizeof(UInt32));
	replyByteCount = EndianU32_BtoN(replyByteCount);
	if (replyByteCount > (responseBody.getByteCount() - 5))
		// Invalid
		return TVResult<CData>(sInvalidReplyError);

	return TVResult<CData>(responseBody.subData(5, replyByteCount));
}
