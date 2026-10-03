//----------------------------------------------------------------------------------------------------------------------
//	CHTTPEndpointRequest.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CHTTPEndpointRequest.h"

#include "CJSON.h"
#include "ConcurrencyPrimitives.h"
#include "TLockingValue.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	const	CString	sStatusErrorDomain(OSSTR("HTTPEndpointStatus"));
static	const	CString	sErrorDomain(OSSTR("CHTTPEndpointRequest"));
static	const	SError	sUnableToProcessResponseBodyError(sErrorDomain, 1,
								CString(OSSTR("Unable to process response body")));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CHTTPEndpointRequest::Internals

class CHTTPEndpointRequest::Internals {
	public:
		Internals(Method method, const CString& path, const OV<CDictionary>& queryComponents,
				const OV<MultiValueQueryComponent>& multiValueQueryComponent, const CDictionary& headers,
				const OV<CData>& body, UniversalTimeInterval timeoutInterval, Options options,
				const CString& contentType) :
			mMethod(method), mPath(path), mQueryComponents(queryComponents),
					mMultiValueQueryComponent(multiValueQueryComponent), mHeaders(headers), mBody(body),
					mTimeoutInterval(timeoutInterval), mOptions(options),
					mIsCancelled(false)
			{
				// Update headers
				mHeaders.set(CString(OSSTR("Content-Type")), contentType);
			}

		Method							mMethod;
		CString							mPath;
		OV<CDictionary>					mQueryComponents;
		OV<MultiValueQueryComponent>	mMultiValueQueryComponent;
		CDictionary						mHeaders;
		OV<CData>						mBody;
		UniversalTimeInterval			mTimeoutInterval;
		Options							mOptions;

		SLockingBoolean					mIsCancelled;

		CLock							mErrorsLock;
		TNArray<SError>					mErrors;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CHTTPEndpointRequest

// MARK: Properties

const	UniversalTimeInterval	CHTTPEndpointRequest::mDefaultTimeoutInterval = 60.0;

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CHTTPEndpointRequest::CHTTPEndpointRequest(Method method, const CString& path, const OV<CDictionary>& queryComponents,
		const OV<MultiValueQueryComponent>& multiValueQueryComponent, const CDictionary& headers, const OV<CData>& body,
		UniversalTimeInterval timeoutInterval, Options options)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals =
			new Internals(method, path, queryComponents, multiValueQueryComponent, headers, body, timeoutInterval,
					options, CString(OSSTR("application/octet-stream")));
}

//----------------------------------------------------------------------------------------------------------------------
CHTTPEndpointRequest::CHTTPEndpointRequest(Method method, const CString& path, const OV<CDictionary>& queryComponents,
		const OV<MultiValueQueryComponent>& multiValueQueryComponent, const CDictionary& headers,
		const CDictionary& jsonBody, UniversalTimeInterval timeoutInterval, Options options)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals =
			new Internals(method, path, queryComponents, multiValueQueryComponent, headers,
					OV<CData>(*CJSON::dataFrom(jsonBody)), timeoutInterval, options,
					CString(OSSTR("application/json")));
}

//----------------------------------------------------------------------------------------------------------------------
CHTTPEndpointRequest::~CHTTPEndpointRequest()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CHTTPEndpointRequest::Method CHTTPEndpointRequest::getMethod() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mMethod;
}

//----------------------------------------------------------------------------------------------------------------------
const CString& CHTTPEndpointRequest::getPath() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mPath;
}

//----------------------------------------------------------------------------------------------------------------------
const OV<CDictionary>& CHTTPEndpointRequest::getQueryComponents() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mQueryComponents;
}

//----------------------------------------------------------------------------------------------------------------------
const OV<CHTTPEndpointRequest::MultiValueQueryComponent>& CHTTPEndpointRequest::getMultiValueQueryComponent() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mMultiValueQueryComponent;
}

//----------------------------------------------------------------------------------------------------------------------
const CDictionary& CHTTPEndpointRequest::getHeaders() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mHeaders;
}

//----------------------------------------------------------------------------------------------------------------------
const OV<CData>& CHTTPEndpointRequest::getBody() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mBody;
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTimeInterval CHTTPEndpointRequest::getTimeoutInterval() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mTimeoutInterval;
}

//----------------------------------------------------------------------------------------------------------------------
CHTTPEndpointRequest::Options CHTTPEndpointRequest::getOptions() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mOptions;
}

//----------------------------------------------------------------------------------------------------------------------
void CHTTPEndpointRequest::setHeader(const CString& field, const CString& value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mHeaders.set(field, value);
}

//----------------------------------------------------------------------------------------------------------------------
void CHTTPEndpointRequest::cancel()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mIsCancelled = true;
}

//----------------------------------------------------------------------------------------------------------------------
bool CHTTPEndpointRequest::isCancelled() const
//----------------------------------------------------------------------------------------------------------------------
{
	return *mInternals->mIsCancelled;
}

//----------------------------------------------------------------------------------------------------------------------
void CHTTPEndpointRequest::noteResponse(const TVResult<SHTTPEndpointResponse>& response)
//----------------------------------------------------------------------------------------------------------------------
{
	// Classify
	OV<SHTTPEndpointResponse>	endpointResponse;
	OV<SError>					error;
	if (response.hasError())
		// Transport error
		error.setValue(response.getError());
	else {
		// Have response
		endpointResponse.setValue(*response);

		if (!response->getStatus().isSuccess())
			// Endpoint error
			error.setValue(
					SError(sStatusErrorDomain, response->getStatus().getCode(),
							response->getBody().hasValue() ?
									response->getStatus().getDescription() + CString(OSSTR(": ")) +
											CString(*response->getBody(), CString::kEncodingUTF8) :
									response->getStatus().getDescription()));
	}

	// Process
	processResponse(endpointResponse,
			error.hasValue() ?
					TVResult<CData>(*error) : TVResult<CData>(endpointResponse->getBody().getValue(CData::mEmpty)));

	// Note error
	if (error.hasValue()) {
		// Add
		mInternals->mErrorsLock.lock();
		mInternals->mErrors += *error;
		mInternals->mErrorsLock.unlock();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CHTTPEndpointRequest::noteAllResponsesReceived()
//----------------------------------------------------------------------------------------------------------------------
{
	processAllResponses(mInternals->mErrors);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CHeadHTTPEndpointRequest

// MARK: CHTTPEndpointRequest methods

//----------------------------------------------------------------------------------------------------------------------
void CHeadHTTPEndpointRequest::processResponse(const OV<SHTTPEndpointResponse>& response, const TVResult<CData>& body)
//----------------------------------------------------------------------------------------------------------------------
{
	// Call proc
	mCompletionProc(
			body.hasError() ?
					TVResult<SHTTPEndpointResponse>(body.getError()) : TVResult<SHTTPEndpointResponse>(*response),
			mUserData);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CIntegerHTTPEndpointRequest

// MARK: CHTTPEndpointRequest methods

//----------------------------------------------------------------------------------------------------------------------
void CIntegerHTTPEndpointRequest::processResponse(const OV<SHTTPEndpointResponse>& response,
		const TVResult<CData>& body)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check body
	if (body.hasError()) {
		// Error
		mCompletionProc(response, TVResult<SInt64>(body.getError()), mUserData);

		return;
	}

	// Convert
	CString	string(*body, CString::kEncodingUTF8);
	mCompletionProc(response,
			!string.isEmpty() ?
					TVResult<SInt64>(string.getSInt64()) : TVResult<SInt64>(sUnableToProcessResponseBodyError),
			mUserData);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CStringHTTPEndpointRequest

// MARK: CHTTPEndpointRequest methods

//----------------------------------------------------------------------------------------------------------------------
void CStringHTTPEndpointRequest::processResponse(const OV<SHTTPEndpointResponse>& response,
		const TVResult<CData>& body)
//----------------------------------------------------------------------------------------------------------------------
{
	// Call proc
	mCompletionProc(response,
			body.hasError() ?
					TVResult<CString>(body.getError()) : TVResult<CString>(CString(*body, CString::kEncodingUTF8)),
			mUserData);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CJSONHTTPEndpointRequest

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CJSONHTTPEndpointRequest::CJSONHTTPEndpointRequest(Method method, const CString& path,
		const OV<CDictionary>& queryComponents, const OV<MultiValueQueryComponent>& multiValueQueryComponent,
		const CDictionary& headers, const OV<CData>& body, PartialResultsProc partialResultsProc,
		AllResultsCompletionProc allResultsCompletionProc, void* userData, UniversalTimeInterval timeoutInterval,
		Options options) :
		CHTTPEndpointRequest(method, path, queryComponents, multiValueQueryComponent, headers, body, timeoutInterval,
						options),
				mCompletionProc(nil), mPartialResultsProc(partialResultsProc),
				mAllResultsCompletionProc(allResultsCompletionProc), mUserData(userData)
//----------------------------------------------------------------------------------------------------------------------
{
	setHeader(CString(OSSTR("Accept")), CString(OSSTR("application/json")));
}

//----------------------------------------------------------------------------------------------------------------------
CJSONHTTPEndpointRequest::CJSONHTTPEndpointRequest(Method method, const CString& path,
		const OV<CDictionary>& queryComponents, const CDictionary& headers, const OV<CData>& body,
		CompletionProc completionProc, void* userData, UniversalTimeInterval timeoutInterval, Options options) :
		CHTTPEndpointRequest(method, path, queryComponents, OV<MultiValueQueryComponent>(), headers, body,
						timeoutInterval, options),
				mCompletionProc(completionProc), mPartialResultsProc(nil), mAllResultsCompletionProc(nil),
				mUserData(userData)
//----------------------------------------------------------------------------------------------------------------------
{
	setHeader(CString(OSSTR("Accept")), CString(OSSTR("application/json")));
}

//----------------------------------------------------------------------------------------------------------------------
CJSONHTTPEndpointRequest::CJSONHTTPEndpointRequest(Method method, const CString& path,
		const OV<CDictionary>& queryComponents, const CDictionary& headers, const CDictionary& jsonBody,
		CompletionProc completionProc, void* userData, UniversalTimeInterval timeoutInterval, Options options) :
		CHTTPEndpointRequest(method, path, queryComponents, OV<MultiValueQueryComponent>(), headers, jsonBody,
						timeoutInterval, options),
				mCompletionProc(completionProc), mPartialResultsProc(nil), mAllResultsCompletionProc(nil),
				mUserData(userData)
//----------------------------------------------------------------------------------------------------------------------
{
	setHeader(CString(OSSTR("Accept")), CString(OSSTR("application/json")));
}

// MARK: CHTTPEndpointRequest methods

//----------------------------------------------------------------------------------------------------------------------
void CJSONHTTPEndpointRequest::processResponse(const OV<SHTTPEndpointResponse>& response,
		const TVResult<CData>& body)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if have completion proc
	if (mCompletionProc != nil)
		// Single response
		mCompletionProc(response,
				body.hasError() ? TVResult<CDictionary>(body.getError()) : CJSON::dictionaryFrom(*body), mUserData);
	else
		// One of possibly several responses
		mPartialResultsProc(response,
				body.hasError() ? TVResult<CDictionary>(body.getError()) : CJSON::dictionaryFrom(*body), mUserData);
}

//----------------------------------------------------------------------------------------------------------------------
void CJSONHTTPEndpointRequest::processAllResponses(const TArray<SError>& errors)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check proc
	if (mAllResultsCompletionProc != nil)
		// Call proc
		mAllResultsCompletionProc(errors, mUserData);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CJSONsHTTPEndpointRequest

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CJSONsHTTPEndpointRequest::CJSONsHTTPEndpointRequest(Method method, const CString& path,
		const OV<CDictionary>& queryComponents, const OV<MultiValueQueryComponent>& multiValueQueryComponent,
		const CDictionary& headers, const OV<CData>& body, PartialResultsProc partialResultsProc,
		AllResultsCompletionProc allResultsCompletionProc, void* userData, UniversalTimeInterval timeoutInterval,
		Options options) :
		CHTTPEndpointRequest(method, path, queryComponents, multiValueQueryComponent, headers, body, timeoutInterval,
						options),
				mCompletionProc(nil), mPartialResultsProc(partialResultsProc),
				mAllResultsCompletionProc(allResultsCompletionProc), mUserData(userData)
//----------------------------------------------------------------------------------------------------------------------
{
	setHeader(CString(OSSTR("Accept")), CString(OSSTR("application/json")));
}

//----------------------------------------------------------------------------------------------------------------------
CJSONsHTTPEndpointRequest::CJSONsHTTPEndpointRequest(Method method, const CString& path,
		const OV<CDictionary>& queryComponents, const CDictionary& headers, const OV<CData>& body,
		CompletionProc completionProc, void* userData, UniversalTimeInterval timeoutInterval, Options options) :
		CHTTPEndpointRequest(method, path, queryComponents, OV<MultiValueQueryComponent>(), headers, body,
						timeoutInterval, options),
				mCompletionProc(completionProc), mPartialResultsProc(nil), mAllResultsCompletionProc(nil),
				mUserData(userData)
//----------------------------------------------------------------------------------------------------------------------
{
	setHeader(CString(OSSTR("Accept")), CString(OSSTR("application/json")));
}

//----------------------------------------------------------------------------------------------------------------------
CJSONsHTTPEndpointRequest::CJSONsHTTPEndpointRequest(Method method, const CString& path,
		const OV<CDictionary>& queryComponents, const CDictionary& headers, const CDictionary& jsonBody,
		CompletionProc completionProc, void* userData, UniversalTimeInterval timeoutInterval, Options options) :
		CHTTPEndpointRequest(method, path, queryComponents, OV<MultiValueQueryComponent>(), headers, jsonBody,
						timeoutInterval, options),
				mCompletionProc(completionProc), mPartialResultsProc(nil), mAllResultsCompletionProc(nil),
				mUserData(userData)
//----------------------------------------------------------------------------------------------------------------------
{
	setHeader(CString(OSSTR("Accept")), CString(OSSTR("application/json")));
}

// MARK: CHTTPEndpointRequest methods

//----------------------------------------------------------------------------------------------------------------------
void CJSONsHTTPEndpointRequest::processResponse(const OV<SHTTPEndpointResponse>& response,
		const TVResult<CData>& body)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if have completion proc
	if (mCompletionProc != nil)
		// Single response
		mCompletionProc(response,
				body.hasError() ?
						TVResult<TArray<CDictionary> >(body.getError()) : CJSON::arrayOfDictionariesFrom(*body),
				mUserData);
	else
		// One of possibly several responses
		mPartialResultsProc(response,
				body.hasError() ?
						TVResult<TArray<CDictionary> >(body.getError()) : CJSON::arrayOfDictionariesFrom(*body),
				mUserData);
}

//----------------------------------------------------------------------------------------------------------------------
void CJSONsHTTPEndpointRequest::processAllResponses(const TArray<SError>& errors)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check proc
	if (mAllResultsCompletionProc != nil)
		// Call proc
		mAllResultsCompletionProc(errors, mUserData);
}
