//----------------------------------------------------------------------------------------------------------------------
//	CHTTPEndpointRequest.h			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CHTTPTransport.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CHTTPEndpointRequest

class CHTTPEndpointRequest {
	// Method
	public:
		typedef	CHTTPTransport::Method	Method;

	// Options
	public:
		enum Options {
			kOptionsNone					= 0,
			kOptionsQueryContainsSecureInfo	= 1 << 0,
			kOptionsDeferBodyUntilRedirect	= 1 << 1,
		};

	// MultiValueQueryComponent
	public:
		struct MultiValueQueryComponent {
			// Methods
			public:
											// Lifecycle methods
											MultiValueQueryComponent(const CString& key,
													const TArray<CString>& values) :
												mKey(key), mValues(values)
												{}

											// Instance methods
				const	CString&			getKey() const
												{ return mKey; }
				const	TArray<CString>&	getValues() const
												{ return mValues; }

			// Properties
			private:
				CString			mKey;
				TArray<CString>	mValues;
		};

	// Classes
	private:
		class Internals;

	// Methods
	public:
														// Lifecycle methods
		virtual											~CHTTPEndpointRequest();

														// Instance methods
						Method							getMethod() const;
				const	CString&						getPath() const;
				const	OV<CDictionary>&				getQueryComponents() const;
				const	OV<MultiValueQueryComponent>&	getMultiValueQueryComponent() const;
				const	CDictionary&					getHeaders() const;
				const	OV<CData>&						getBody() const;
						UniversalTimeInterval			getTimeoutInterval() const;
						Options							getOptions() const;

						void							setHeader(const CString& field, const CString& value);

						void							cancel();
						bool							isCancelled() const;

						void							noteResponse(const TVResult<SHTTPEndpointResponse>& response);
						void							noteAllResponsesReceived();

	protected:
														// Lifecycle methods
														CHTTPEndpointRequest(Method method, const CString& path,
																const OV<CDictionary>& queryComponents,
																const OV<MultiValueQueryComponent>&
																		multiValueQueryComponent,
																const CDictionary& headers, const OV<CData>& body,
																UniversalTimeInterval timeoutInterval, Options options);
														CHTTPEndpointRequest(Method method, const CString& path,
																const OV<CDictionary>& queryComponents,
																const OV<MultiValueQueryComponent>&
																		multiValueQueryComponent,
																const CDictionary& headers, const CDictionary& jsonBody,
																UniversalTimeInterval timeoutInterval, Options options);

														// Subclass methods
		virtual			void							processResponse(const OV<SHTTPEndpointResponse>& response,
																const TVResult<CData>& body) = 0;
		virtual			void							processAllResponses(const TArray<SError>& errors) {}

	// Properties
	public:
		static	const	UniversalTimeInterval	mDefaultTimeoutInterval;

	private:
						Internals*				mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDataHTTPEndpointRequest

class CDataHTTPEndpointRequest : public CHTTPEndpointRequest {
	// Procs
	public:
		typedef	void	(*CompletionProc)(const OV<SHTTPEndpointResponse>& response, const TVResult<CData>& data,
								void* userData);

	// Methods
	public:
				// Lifecycle methods
				CDataHTTPEndpointRequest(Method method, const CString& path, const OV<CDictionary>& queryComponents,
						const CDictionary& headers, const OV<CData>& body, CompletionProc completionProc,
						void* userData, UniversalTimeInterval timeoutInterval = mDefaultTimeoutInterval,
						Options options = kOptionsNone) :
					CHTTPEndpointRequest(method, path, queryComponents, OV<MultiValueQueryComponent>(), headers, body,
									timeoutInterval, options),
							mCompletionProc(completionProc), mUserData(userData)
					{}

	protected:
				// CHTTPEndpointRequest methods
		void	processResponse(const OV<SHTTPEndpointResponse>& response, const TVResult<CData>& body)
					{ mCompletionProc(response, body, mUserData); }

	// Properties
	private:
		CompletionProc	mCompletionProc;
		void*			mUserData;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CHeadHTTPEndpointRequest

class CHeadHTTPEndpointRequest : public CHTTPEndpointRequest {
	// Procs
	public:
		typedef	void	(*CompletionProc)(const TVResult<SHTTPEndpointResponse>& response, void* userData);

	// Methods
	public:
				// Lifecycle methods
				CHeadHTTPEndpointRequest(const CString& path, const OV<CDictionary>& queryComponents,
						const CDictionary& headers, CompletionProc completionProc, void* userData,
						UniversalTimeInterval timeoutInterval = mDefaultTimeoutInterval,
						Options options = kOptionsNone) :
					CHTTPEndpointRequest(CHTTPTransport::kMethodHead, path, queryComponents,
									OV<MultiValueQueryComponent>(), headers, OV<CData>(), timeoutInterval, options),
							mCompletionProc(completionProc), mUserData(userData)
					{}

	protected:
				// CHTTPEndpointRequest methods
		void	processResponse(const OV<SHTTPEndpointResponse>& response, const TVResult<CData>& body);

	// Properties
	private:
		CompletionProc	mCompletionProc;
		void*			mUserData;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CIntegerHTTPEndpointRequest

class CIntegerHTTPEndpointRequest : public CHTTPEndpointRequest {
	// Procs
	public:
		typedef	void	(*CompletionProc)(const OV<SHTTPEndpointResponse>& response, const TVResult<SInt64>& value,
								void* userData);

	// Methods
	public:
				// Lifecycle methods
				CIntegerHTTPEndpointRequest(Method method, const CString& path, const OV<CDictionary>& queryComponents,
						const CDictionary& headers, const OV<CData>& body, CompletionProc completionProc,
						void* userData, UniversalTimeInterval timeoutInterval = mDefaultTimeoutInterval,
						Options options = kOptionsNone) :
					CHTTPEndpointRequest(method, path, queryComponents, OV<MultiValueQueryComponent>(), headers, body,
									timeoutInterval, options),
							mCompletionProc(completionProc), mUserData(userData)
					{}

	protected:
				// CHTTPEndpointRequest methods
		void	processResponse(const OV<SHTTPEndpointResponse>& response, const TVResult<CData>& body);

	// Properties
	private:
		CompletionProc	mCompletionProc;
		void*			mUserData;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CStringHTTPEndpointRequest

class CStringHTTPEndpointRequest : public CHTTPEndpointRequest {
	// Procs
	public:
		typedef	void	(*CompletionProc)(const OV<SHTTPEndpointResponse>& response, const TVResult<CString>& string,
								void* userData);

	// Methods
	public:
				// Lifecycle methods
				CStringHTTPEndpointRequest(Method method, const CString& path, const OV<CDictionary>& queryComponents,
						const CDictionary& headers, const OV<CData>& body, CompletionProc completionProc,
						void* userData, UniversalTimeInterval timeoutInterval = mDefaultTimeoutInterval,
						Options options = kOptionsNone) :
					CHTTPEndpointRequest(method, path, queryComponents, OV<MultiValueQueryComponent>(), headers, body,
									timeoutInterval, options),
							mCompletionProc(completionProc), mUserData(userData)
					{}

	protected:
				// CHTTPEndpointRequest methods
		void	processResponse(const OV<SHTTPEndpointResponse>& response, const TVResult<CData>& body);

	// Properties
	private:
		CompletionProc	mCompletionProc;
		void*			mUserData;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSuccessHTTPEndpointRequest

class CSuccessHTTPEndpointRequest : public CHTTPEndpointRequest {
	// Procs
	public:
		typedef	void	(*CompletionProc)(const OV<SHTTPEndpointResponse>& response, const OV<SError>& error,
								void* userData);

	// Methods
	public:
				// Lifecycle methods
				CSuccessHTTPEndpointRequest(Method method, const CString& path, const OV<CDictionary>& queryComponents,
						const CDictionary& headers, const OV<CData>& body, CompletionProc completionProc,
						void* userData, UniversalTimeInterval timeoutInterval = mDefaultTimeoutInterval,
						Options options = kOptionsNone) :
					CHTTPEndpointRequest(method, path, queryComponents, OV<MultiValueQueryComponent>(), headers, body,
									timeoutInterval, options),
							mCompletionProc(completionProc), mUserData(userData)
					{}
				CSuccessHTTPEndpointRequest(Method method, const CString& path, const OV<CDictionary>& queryComponents,
						const CDictionary& headers, const CDictionary& jsonBody, CompletionProc completionProc,
						void* userData, UniversalTimeInterval timeoutInterval = mDefaultTimeoutInterval,
						Options options = kOptionsNone) :
					CHTTPEndpointRequest(method, path, queryComponents, OV<MultiValueQueryComponent>(), headers,
									jsonBody, timeoutInterval, options),
							mCompletionProc(completionProc), mUserData(userData)
					{}

	protected:
				// CHTTPEndpointRequest methods
		void	processResponse(const OV<SHTTPEndpointResponse>& response, const TVResult<CData>& body)
					{ mCompletionProc(response, body.hasError() ? OV<SError>(body.getError()) : OV<SError>(),
							mUserData); }

	// Properties
	private:
		CompletionProc	mCompletionProc;
		void*			mUserData;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CJSONHTTPEndpointRequest

class CJSONHTTPEndpointRequest : public CHTTPEndpointRequest {
	// Procs
	public:
		typedef	void	(*CompletionProc)(const OV<SHTTPEndpointResponse>& response,
								const TVResult<CDictionary>& dictionary, void* userData);
		typedef	void	(*PartialResultsProc)(const OV<SHTTPEndpointResponse>& response,
								const TVResult<CDictionary>& dictionary, void* userData);
		typedef	void	(*AllResultsCompletionProc)(const TArray<SError>& errors, void* userData);

	// Methods
	public:
				// Lifecycle methods
				CJSONHTTPEndpointRequest(Method method, const CString& path,
						const OV<CDictionary>& queryComponents,
						const OV<MultiValueQueryComponent>& multiValueQueryComponent, const CDictionary& headers,
						const OV<CData>& body, PartialResultsProc partialResultsProc,
						AllResultsCompletionProc allResultsCompletionProc, void* userData,
						UniversalTimeInterval timeoutInterval = mDefaultTimeoutInterval,
						Options options = kOptionsNone);
				CJSONHTTPEndpointRequest(Method method, const CString& path,
						const OV<CDictionary>& queryComponents, const CDictionary& headers, const OV<CData>& body,
						CompletionProc completionProc, void* userData,
						UniversalTimeInterval timeoutInterval = mDefaultTimeoutInterval,
						Options options = kOptionsNone);
				CJSONHTTPEndpointRequest(Method method, const CString& path,
						const OV<CDictionary>& queryComponents, const CDictionary& headers, const CDictionary& jsonBody,
						CompletionProc completionProc, void* userData,
						UniversalTimeInterval timeoutInterval = mDefaultTimeoutInterval,
						Options options = kOptionsNone);

	protected:
				// CHTTPEndpointRequest methods
		void	processResponse(const OV<SHTTPEndpointResponse>& response, const TVResult<CData>& body);
		void	processAllResponses(const TArray<SError>& errors);

	// Properties
	private:
		CompletionProc				mCompletionProc;
		PartialResultsProc			mPartialResultsProc;
		AllResultsCompletionProc	mAllResultsCompletionProc;
		void*						mUserData;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CJSONsHTTPEndpointRequest

class CJSONsHTTPEndpointRequest : public CHTTPEndpointRequest {
	// Procs
	public:
		typedef	void	(*CompletionProc)(const OV<SHTTPEndpointResponse>& response,
								const TVResult<TArray<CDictionary> >& dictionaries, void* userData);
		typedef	void	(*PartialResultsProc)(const OV<SHTTPEndpointResponse>& response,
								const TVResult<TArray<CDictionary> >& dictionaries, void* userData);
		typedef	void	(*AllResultsCompletionProc)(const TArray<SError>& errors, void* userData);

	// Methods
	public:
				// Lifecycle methods
				CJSONsHTTPEndpointRequest(Method method, const CString& path,
						const OV<CDictionary>& queryComponents,
						const OV<MultiValueQueryComponent>& multiValueQueryComponent, const CDictionary& headers,
						const OV<CData>& body, PartialResultsProc partialResultsProc,
						AllResultsCompletionProc allResultsCompletionProc, void* userData,
						UniversalTimeInterval timeoutInterval = mDefaultTimeoutInterval,
						Options options = kOptionsNone);
				CJSONsHTTPEndpointRequest(Method method, const CString& path,
						const OV<CDictionary>& queryComponents, const CDictionary& headers, const OV<CData>& body,
						CompletionProc completionProc, void* userData,
						UniversalTimeInterval timeoutInterval = mDefaultTimeoutInterval,
						Options options = kOptionsNone);
				CJSONsHTTPEndpointRequest(Method method, const CString& path,
						const OV<CDictionary>& queryComponents, const CDictionary& headers, const CDictionary& jsonBody,
						CompletionProc completionProc, void* userData,
						UniversalTimeInterval timeoutInterval = mDefaultTimeoutInterval,
						Options options = kOptionsNone);

	protected:
				// CHTTPEndpointRequest methods
		void	processResponse(const OV<SHTTPEndpointResponse>& response, const TVResult<CData>& body);
		void	processAllResponses(const TArray<SError>& errors);

	// Properties
	private:
		CompletionProc				mCompletionProc;
		PartialResultsProc			mPartialResultsProc;
		AllResultsCompletionProc	mAllResultsCompletionProc;
		void*						mUserData;
};
