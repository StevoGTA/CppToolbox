//----------------------------------------------------------------------------------------------------------------------
//	CHTTPEndpointClient.h			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CHTTPEndpointRequest.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CHTTPEndpointClient

class CHTTPEndpointClient {
	// Options
	public:
		enum Options {
			kOptionsNone						= 0,
			kOptionsMultiValueQueryUseComma		= 1 << 0,
			kOptionsPercentEncodePlusCharacter	= 1 << 1,
		};

	// Priority
	public:
		enum Priority {
			kPriorityNormal,
			kPriorityBackground,
		};

	// LogOptions
	public:
		enum LogOptions {
			kLogOptionsNone					= 0,
			kLogOptionsRequestAndResponse	= 1 << 0,
			kLogOptionsRequestQuery			= 1 << 1,
			kLogOptionsRequestHeaders		= 1 << 2,
			kLogOptionsRequestBody			= 1 << 3,
			kLogOptionsRequestBodySize		= 1 << 4,
			kLogOptionsResponseHeaders		= 1 << 5,
			kLogOptionsResponseBody			= 1 << 6,
		};

	// Classes
	private:
		class Internals;

	// Methods
	public:
				// Lifecycle methods
				CHTTPEndpointClient(const CString& serverPrefix, const I<CHTTPTransport>& transport,
						Options options = kOptionsNone, UInt32 maximumURLLength = 1024,
						UInt32 maximumConcurrentRequests = 6, LogOptions logOptions = kLogOptionsNone);
				~CHTTPEndpointClient();

				// Instance methods
		void	queue(const I<CHTTPEndpointRequest>& request, const CString& identifier = CString::mEmpty,
						Priority priority = kPriorityNormal);
		void	cancel(const CString& identifier);

	// Properties
	private:
		Internals*	mInternals;
};
