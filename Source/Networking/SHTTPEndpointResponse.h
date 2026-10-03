//----------------------------------------------------------------------------------------------------------------------
//	SHTTPEndpointResponse.h			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CData.h"
#include "CDictionary.h"
#include "SHTTPEndpointStatus.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SHTTPEndpointResponse

struct SHTTPEndpointResponse {
	// Methods
	public:
										// Lifecycle methods
										SHTTPEndpointResponse(const SHTTPEndpointStatus& status,
												const CDictionary& headers, const OV<CData>& body) :
											mStatus(status), mHeaders(headers), mBody(body)
											{}

										// Instance methods
		const	SHTTPEndpointStatus&	getStatus() const
											{ return mStatus; }
		const	CDictionary&			getHeaders() const
											{ return mHeaders; }
				OV<CString>				getHeaderValue(const CString& field) const
											{ return mHeaders.getOVString(field); }
		const	OV<CData>&				getBody() const
											{ return mBody; }

	// Properties
	private:
		SHTTPEndpointStatus	mStatus;
		CDictionary			mHeaders;
		OV<CData>			mBody;
};
