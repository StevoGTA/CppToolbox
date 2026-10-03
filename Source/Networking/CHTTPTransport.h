//----------------------------------------------------------------------------------------------------------------------
//	CHTTPTransport.h			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SHTTPEndpointResponse.h"
#include "TimeAndDate.h"
#include "TResult.h"
#include "TWrappers.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CHTTPTransport

class CHTTPTransport {
	// Method
	public:
		enum Method {
			kMethodDelete,
			kMethodGet,
			kMethodHead,
			kMethodPatch,
			kMethodPost,
			kMethodPut,
		};

	// Request
	public:
		struct Request {
			// Methods
			public:
												// Lifecycle methods
												Request(Method method, const CString& urlString,
														const CDictionary& headers, const OV<CData>& body,
														UniversalTimeInterval timeoutInterval) :
													mMethod(method), mURLString(urlString), mHeaders(headers),
															mBody(body), mTimeoutInterval(timeoutInterval)
													{}

												// Instance methods
						Method					getMethod() const
													{ return mMethod; }
				const	CString&				getURLString() const
													{ return mURLString; }
				const	CDictionary&			getHeaders() const
													{ return mHeaders; }
				const	OV<CData>&				getBody() const
													{ return mBody; }
						UniversalTimeInterval	getTimeoutInterval() const
													{ return mTimeoutInterval; }

			// Properties
			private:
				Method					mMethod;
				CString					mURLString;
				CDictionary				mHeaders;
				OV<CData>				mBody;
				UniversalTimeInterval	mTimeoutInterval;
		};

	// Task
	public:
		class Task {
			// Methods
			public:
								// Lifecycle methods
				virtual			~Task() {}

								// Instance methods
				virtual	void	cancel() = 0;
		};

	// Procs
	public:
		typedef	void	(*CompletionProc)(const TVResult<SHTTPEndpointResponse>& response, void* userData);

	// Methods
	public:
						// Lifecycle methods
		virtual			~CHTTPTransport() {}

						// Instance methods
		virtual	I<Task>	perform(const Request& request, CompletionProc completionProc, void* userData) = 0;
};
