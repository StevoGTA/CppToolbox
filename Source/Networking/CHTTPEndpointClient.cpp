//----------------------------------------------------------------------------------------------------------------------
//	CHTTPEndpointClient.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CHTTPEndpointClient.h"

#include "CJSON.h"
#include "CLogServices.h"
#include "ConcurrencyPrimitives.h"
#include "TimeAndDate.h"
#include "TLockingValue.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CHTTPEndpointClient::Internals

class CHTTPEndpointClient::Internals {
	public:
		// State
		enum State {
			kStateQueued,
			kStateActive,
			kStateFinished,
		};

		class PerformInfo;

		class RequestInfo {
			public:
										RequestInfo(const I<CHTTPEndpointRequest>& request, const CString& identifier,
												Priority priority) :
											mRequest(request), mIdentifier(identifier), mPriority(priority),
													mTotalPerformInfosCount(0), mFinishedPerformInfosCount(0)
											{}

				TArray<I<PerformInfo> >	composePerformInfos(const I<RequestInfo>& requestInfo,
												const CString& serverPrefix, Options options, UInt32 maximumURLLength);
				void					noteFinished()
											{
												// Check if all finished
												if (mFinishedPerformInfosCount.add(1) == mTotalPerformInfosCount)
													// All responses received
													mRequest->noteAllResponsesReceived();
											}

				I<CHTTPEndpointRequest>	mRequest;
				CString					mIdentifier;
				Priority				mPriority;
				UInt32					mTotalPerformInfosCount;
				TLockingNumeric<UInt32>	mFinishedPerformInfosCount;
		};

		class PerformInfo {
			// Procs
			public:
				typedef	void	(*CompletedProc)(PerformInfo& performInfo,
										const TVResult<SHTTPEndpointResponse>& response, void* userData);

			// Methods
			public:
												PerformInfo(const I<RequestInfo>& requestInfo,
														const CHTTPTransport::Request& transportRequest) :
													mRequestInfo(requestInfo), mTransportRequest(transportRequest),
															mState(kStateQueued), mIndex(0), mStartTime(0.0),
															mCompletedProc(nil), mCompletedUserData(nil)
													{}

				const	CHTTPEndpointRequest&	getRequest() const
													{ return *mRequestInfo->mRequest; }
				const	CString&				getIdentifier() const
													{ return mRequestInfo->mIdentifier; }
						Priority				getPriority() const
													{ return mRequestInfo->mPriority; }
						bool					isCancelled() const
													{ return mRequestInfo->mRequest->isCancelled(); }

						void					transition(State state)
													{
														// Update
														mState = state;

														// Check state
														if (state == kStateFinished)
															// Inform request info
															mRequestInfo->noteFinished();
													}

						void					perform(CHTTPTransport& transport, CompletedProc completedProc,
														void* userData)
													{
														// Store
														mCompletedProc = completedProc;
														mCompletedUserData = userData;

														// Perform
														mStartTime = SUniversalTime::getCurrent();
														mTask.setValue(transport.perform(mTransportRequest,
																transportCompleted, this));
													}

				static	void					transportCompleted(const TVResult<SHTTPEndpointResponse>& response,
														void* userData)
													{
														// Setup
														PerformInfo&	performInfo = *((PerformInfo*) userData);

														// Call proc
														performInfo.mCompletedProc(performInfo, response,
																performInfo.mCompletedUserData);
													}

				I<RequestInfo>					mRequestInfo;
				CHTTPTransport::Request			mTransportRequest;
				State							mState;
				UInt32							mIndex;
				UniversalTime					mStartTime;
				OV<I<CHTTPTransport::Task> >	mTask;
				CompletedProc					mCompletedProc;
				void*							mCompletedUserData;
		};

						Internals(const CString& serverPrefix, const I<CHTTPTransport>& transport, Options options,
								UInt32 maximumURLLength, UInt32 maximumConcurrentRequests, LogOptions logOptions) :
							mServerPrefix(serverPrefix), mTransport(transport), mOptions(options),
									mMaximumURLLength(maximumURLLength),
									mMaximumConcurrentRequests(maximumConcurrentRequests), mLogOptions(logOptions),
									mNextIndex(0)
							{}

				void	updatePerformInfos();
				void	logRequest(const PerformInfo& performInfo) const;
				void	logResponse(const PerformInfo& performInfo, const TVResult<SHTTPEndpointResponse>& response)
								const;

		static	bool	isBeforeByPriority(const I<PerformInfo>& performInfo1, const I<PerformInfo>& performInfo2,
								void* userData)
							{ return performInfo1->getPriority() < performInfo2->getPriority(); }
		static	bool	isFinished(const I<PerformInfo>& performInfo, void* userData)
							{ return performInfo->mState == kStateFinished; }
		static	bool	hasIdentifier(const I<PerformInfo>& performInfo, void* userData)
							{ return performInfo->getIdentifier() == *((CString*) userData); }
		static	void	performInfoCompleted(PerformInfo& performInfo, const TVResult<SHTTPEndpointResponse>& response,
								void* userData);

		CString						mServerPrefix;
		I<CHTTPTransport>			mTransport;
		Options						mOptions;
		UInt32						mMaximumURLLength;
		UInt32						mMaximumConcurrentRequests;
		LogOptions					mLogOptions;

		CLock						mLock;
		TNArray<I<PerformInfo> >	mQueuedPerformInfos;
		TNArray<I<PerformInfo> >	mActivePerformInfos;
		UInt32						mNextIndex;
};

//----------------------------------------------------------------------------------------------------------------------
TArray<I<CHTTPEndpointClient::Internals::PerformInfo> >
		CHTTPEndpointClient::Internals::RequestInfo::composePerformInfos(
				const I<RequestInfo>& requestInfo, const CString& serverPrefix, Options options,
				UInt32 maximumURLLength)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	CHTTPEndpointRequest&		request = *mRequest;
			CHTTPTransport::Method		method = request.getMethod();
	const	CDictionary&				headers = request.getHeaders();
			OV<CData>					body =
												(request.getOptions() &
																CHTTPEndpointRequest::kOptionsDeferBodyUntilRedirect) ?
														OV<CData>() : request.getBody();
			UniversalTimeInterval		timeoutInterval = request.getTimeoutInterval();

	// Compose perform infos
	TNArray<I<PerformInfo> >	performInfos;
	if (request.getPath().hasPrefix(CString(OSSTR("http")))) {
		// Already have fully-formed URL
		performInfos +=
				I<PerformInfo>(
						new PerformInfo(requestInfo,
								CHTTPTransport::Request(method, request.getPath(), headers, body, timeoutInterval)));
		mTotalPerformInfosCount = 1;
	} else {
		// Compose query
		bool	encodePlusCharacter = options & kOptionsPercentEncodePlusCharacter;
		CString	queryString;
		if (request.getQueryComponents().hasValue())
			// Iterate query components
			for (CDictionary::Iterator iterator = request.getQueryComponents()->getIterator(); iterator; iterator++) {
				// Setup
						CString	key = iterator.getKey().percentEncodedForURLQuery(encodePlusCharacter);
				const	SValue&	value = iterator.getValue();

				// Collect values
				TNArray<CString>	values;
				switch (value.getType()) {
					case SValue::kTypeArrayOfStrings:
						// Array of strings
						values += value.getArrayOfStrings();
						break;

					case SValue::kTypeString:
						// String
						values += value.getString();
						break;

					case SValue::kTypeBool:
						// Bool
						values += value.getBool() ? CString(OSSTR("true")) : CString(OSSTR("false"));
						break;

					case SValue::kTypeFloat32:
					case SValue::kTypeFloat64:
						// Float32 and Float64
						values += CString(value.getFloat64());
						break;

					default:
						// Everything else
						values += CString(value.getSInt64());
						break;
				}

				// Append
				for (TArray<CString>::Iterator valueIterator = values.getIterator(); valueIterator; valueIterator++)
					// Append
					queryString +=
							(!queryString.isEmpty() ?
									CString(OSSTR("&")) :
									CString::mEmpty) + key + CString(OSSTR("=")) +
											valueIterator->percentEncodedForURLQuery(encodePlusCharacter);
			}

		const	OV<CHTTPEndpointRequest::MultiValueQueryComponent>&	multiValueQueryComponent =
																			request.getMultiValueQueryComponent();
				bool												hasQuery =
																			!queryString.isEmpty() ||
																					multiValueQueryComponent.hasValue();
				CString												urlRoot =
																			serverPrefix + request.getPath() +
																					(hasQuery ?
																							CString(OSSTR("?")) :
																							CString::mEmpty) +
																					queryString;

		// Check multi-value query component
		if (multiValueQueryComponent.hasValue() && !multiValueQueryComponent->getValues().isEmpty()) {
			// Split across as many URLs as needed to stay within the maximum URL length
			CString	key = multiValueQueryComponent->getKey().percentEncodedForURLQuery(encodePlusCharacter);
			bool	useComma = options & kOptionsMultiValueQueryUseComma;
			CString	urlBase =
							useComma ?
									(urlRoot + (!queryString.isEmpty() ?
											CString(OSSTR("&")) : CString(OSSTR("?"))) + key + CString(OSSTR("="))) :
											(urlRoot + (!queryString.isEmpty() ?
													CString(OSSTR("&")) : CString::mEmpty));
			CString	queryComponent;
			for (TArray<CString>::Iterator iterator = multiValueQueryComponent->getValues().getIterator(); iterator;
					iterator++) {
				// Compose with next value
				CString	value = iterator->percentEncodedForURLQuery(encodePlusCharacter);
				CString	queryComponentTry =
								useComma ?
										(!queryComponent.isEmpty() ?
												queryComponent + CString(OSSTR(",")) + value : value) :
												(!queryComponent.isEmpty() ?
														queryComponent + CString(OSSTR("&")) + key +
																CString(OSSTR("=")) + value :
														key + CString(OSSTR("=")) + value);
				if ((urlBase.getLength() + queryComponentTry.getLength()) <= maximumURLLength)
					// Fits
					queryComponent = queryComponentTry;
				else {
					// Does not fit, flush what we have
					performInfos +=
							I<PerformInfo>(
									new PerformInfo(requestInfo,
											CHTTPTransport::Request(method, urlBase + queryComponent, headers, body,
													timeoutInterval)));

					// Restart
					queryComponent = useComma ? value : key + CString(OSSTR("=")) + value;
				}
			}

			// Add final
			performInfos +=
					I<PerformInfo>(
							new PerformInfo(requestInfo,
									CHTTPTransport::Request(method, urlBase + queryComponent, headers, body,
											timeoutInterval)));
		} else
			// Single URL
			performInfos +=
					I<PerformInfo>(
							new PerformInfo(requestInfo,
									CHTTPTransport::Request(method, urlRoot, headers, body, timeoutInterval)));

		// Store
		mTotalPerformInfosCount = performInfos.getCount();
	}

	return performInfos;
}

//----------------------------------------------------------------------------------------------------------------------
void CHTTPEndpointClient::Internals::updatePerformInfos()
//----------------------------------------------------------------------------------------------------------------------
{
	// Collect what to activate under the lock
	TNArray<I<PerformInfo> >	performInfosToActivate;

	mLock.lock();

	// Remove finished
	mActivePerformInfos.remove(isFinished);

	// Sort queued
	mQueuedPerformInfos.sort(isBeforeByPriority);

	// Activate up to the maximum
	while (!mQueuedPerformInfos.isEmpty() && (mActivePerformInfos.getCount() < mMaximumConcurrentRequests)) {
		// Get first queued
		I<PerformInfo>	performInfo = mQueuedPerformInfos.popFirst(1).getFirst();
		if (performInfo->isCancelled())
			// Cancelled
			continue;

		// Make active
		performInfo->mIndex = mNextIndex++;
		performInfo->transition(kStateActive);
		mActivePerformInfos += performInfo;
		performInfosToActivate += performInfo;
	}

	mLock.unlock();

	// Perform
	for (TArray<I<PerformInfo> >::Iterator iterator = performInfosToActivate.getIterator(); iterator; iterator++) {
		// Log
		logRequest(**iterator);

		// Perform
		(*iterator)->perform(*mTransport, performInfoCompleted, this);
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CHTTPEndpointClient::Internals::logRequest(const PerformInfo& performInfo) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check log options
	if (!(mLogOptions & kLogOptionsRequestAndResponse))
		return;

	// Setup
	const	CHTTPTransport::Request&	transportRequest = performInfo.mTransportRequest;
			OV<SRange32>				queryRange = transportRequest.getURLString().findSubString(CString(OSSTR("?")));
			CString						urlInfo =
												(queryRange.hasValue() ?
														transportRequest.getURLString()
																.getSubString(0, queryRange->getStart()) :
														transportRequest.getURLString()) +
																CString(OSSTR(" (")) + CString(performInfo.mIndex) +
																CString(OSSTR(")"));
			bool						redact =
												performInfo.getRequest().getOptions() &
														CHTTPEndpointRequest::kOptionsQueryContainsSecureInfo;
			TNArray<CString>			messages;

	// Compose
	CString	methodString;
	switch (transportRequest.getMethod()) {
		case CHTTPTransport::kMethodDelete:	methodString = CString(OSSTR("DELETE"));	break;
		case CHTTPTransport::kMethodGet:	methodString = CString(OSSTR("GET"));		break;
		case CHTTPTransport::kMethodHead:	methodString = CString(OSSTR("HEAD"));		break;
		case CHTTPTransport::kMethodPatch:	methodString = CString(OSSTR("PATCH"));		break;
		case CHTTPTransport::kMethodPost:	methodString = CString(OSSTR("POST"));		break;
		case CHTTPTransport::kMethodPut:	methodString = CString(OSSTR("PUT"));		break;
	}
	messages += CString(OSSTR("CHTTPEndpointClient: ")) + methodString + CString(OSSTR(" to ")) + urlInfo;
	if ((mLogOptions & kLogOptionsRequestQuery) && queryRange.hasValue())
		messages +=
				CString(OSSTR("    Query: ")) +
						(redact ?
								CString(OSSTR("<redacted>")) :
								transportRequest.getURLString().getSubString(queryRange->getStart() + 1));
	if (mLogOptions & kLogOptionsRequestHeaders)
		messages +=
				CString(OSSTR("    Headers: ")) +
						CString(*CJSON::dataFrom(transportRequest.getHeaders()), CString::kEncodingUTF8);
	if ((mLogOptions & kLogOptionsRequestBody) && transportRequest.getBody().hasValue())
		messages +=
				CString(OSSTR("    Body: ")) +
						(redact ?
								CString(OSSTR("<redacted>")) :
								CString(*transportRequest.getBody(), CString::kEncodingUTF8));
	if ((mLogOptions & kLogOptionsRequestBodySize) && transportRequest.getBody().hasValue())
		messages +=
				CString(OSSTR("    Body size: ")) + CString((UInt64) transportRequest.getBody()->getByteCount()) +
						CString(OSSTR(" bytes"));

	// Log
	CLogServices::logMessages(messages);
}

//----------------------------------------------------------------------------------------------------------------------
void CHTTPEndpointClient::Internals::logResponse(const PerformInfo& performInfo,
		const TVResult<SHTTPEndpointResponse>& response) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check log options
	if (!(mLogOptions & kLogOptionsRequestAndResponse))
		return;

	// Setup
	const	CHTTPTransport::Request&	transportRequest = performInfo.mTransportRequest;
			OV<SRange32>				queryRange = transportRequest.getURLString().findSubString(CString(OSSTR("?")));
			CString						urlInfo =
												(queryRange.hasValue() ?
														transportRequest.getURLString()
																.getSubString(0, queryRange->getStart()) :
														transportRequest.getURLString()) +
																CString(OSSTR(" (")) + CString(performInfo.mIndex) +
																CString(OSSTR(")"));
			UniversalTimeInterval		deltaTime = SUniversalTime::getCurrent() - performInfo.mStartTime;
			TNArray<CString>			messages;

	// Compose
	if (response.hasValue()) {
		// Received response
		messages +=
				CString(OSSTR("    CHTTPEndpointClient received status ")) +
						CString(response->getStatus().getCode()) + CString(OSSTR(" for ")) + urlInfo +
						CString(OSSTR(" in ")) + CString(deltaTime, 0, 3) + CString(OSSTR("s"));
		if (mLogOptions & kLogOptionsResponseHeaders)
			messages +=
					CString(OSSTR("        Headers: ")) +
							CString(*CJSON::dataFrom(response->getHeaders()), CString::kEncodingUTF8);
		if ((mLogOptions & kLogOptionsResponseBody) && response->getBody().hasValue())
			messages +=
					CString(OSSTR("        Body: ")) + CString(*response->getBody(), CString::kEncodingUTF8);
	} else
		// Error
		messages +=
				CString(OSSTR("    CHTTPEndpointClient received error ")) + response.getError().getInternalDescription() +
						CString(OSSTR(" for ")) + urlInfo;

	// Log
	CLogServices::logMessages(messages);
}

//----------------------------------------------------------------------------------------------------------------------
void CHTTPEndpointClient::Internals::performInfoCompleted(PerformInfo& performInfo,
		const TVResult<SHTTPEndpointResponse>& response, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	Internals&	internals = *((Internals*) userData);

	// Log
	internals.logResponse(performInfo, response);

	// Check if cancelled
	if (!performInfo.isCancelled())
		// Deliver
		performInfo.mRequestInfo->mRequest->noteResponse(response);

	// Transition to finished (after delivery, so the last response is processed before the request hears that all
	//	responses are in)
	performInfo.transition(kStateFinished);

	// Update
	internals.updatePerformInfos();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CHTTPEndpointClient

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CHTTPEndpointClient::CHTTPEndpointClient(const CString& serverPrefix, const I<CHTTPTransport>& transport,
		Options options, UInt32 maximumURLLength, UInt32 maximumConcurrentRequests, LogOptions logOptions)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals =
			new Internals(serverPrefix, transport, options, maximumURLLength, maximumConcurrentRequests, logOptions);
}

//----------------------------------------------------------------------------------------------------------------------
CHTTPEndpointClient::~CHTTPEndpointClient()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CHTTPEndpointClient::queue(const I<CHTTPEndpointRequest>& request, const CString& identifier, Priority priority)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	I<Internals::RequestInfo>	requestInfo(new Internals::RequestInfo(request, identifier, priority));

	// Add to queue
	mInternals->mLock.lock();
	mInternals->mQueuedPerformInfos +=
			requestInfo->composePerformInfos(requestInfo, mInternals->mServerPrefix, mInternals->mOptions,
					mInternals->mMaximumURLLength);
	mInternals->mLock.unlock();

	// Update
	mInternals->updatePerformInfos();
}

//----------------------------------------------------------------------------------------------------------------------
void CHTTPEndpointClient::cancel(const CString& identifier)
//----------------------------------------------------------------------------------------------------------------------
{
	// One at a time please...
	mInternals->mLock.lock();

	// Cancel active
	for (TArray<I<Internals::PerformInfo> >::Iterator iterator = mInternals->mActivePerformInfos.getIterator();
			iterator; iterator++)
		// Check identifier
		if ((*iterator)->getIdentifier() == identifier) {
			// Cancel
			(*iterator)->mRequestInfo->mRequest->cancel();
			if ((*iterator)->mTask.hasValue())
				(*(*iterator)->mTask)->cancel();
		}

	// Cancel queued
	TNArray<I<Internals::PerformInfo> >	queuedPerformInfos =
												mInternals->mQueuedPerformInfos.filtered(Internals::hasIdentifier,
														(void*) &identifier);
	for (TArray<I<Internals::PerformInfo> >::Iterator iterator = queuedPerformInfos.getIterator(); iterator;
			iterator++)
		// Cancel
		(*iterator)->mRequestInfo->mRequest->cancel();
	mInternals->mQueuedPerformInfos.removeFrom(queuedPerformInfos);

	mInternals->mLock.unlock();
}
