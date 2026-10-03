//----------------------------------------------------------------------------------------------------------------------
//	SHTTPEndpointStatus.h			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SHTTPEndpointStatus

struct SHTTPEndpointStatus {
	// Code
	public:
		enum Code {
			kCodeContinue						= 100,
			kCodeSwitchingProtocol				= 101,
			kCodeProcessing						= 102,
			kCodeEarlyHints						= 103,

			kCodeOK								= 200,
			kCodeCreated						= 201,
			kCodeAccepted						= 202,
			kCodeNonAuthoritativeInformation	= 203,
			kCodeNoContent						= 204,
			kCodeResetContent					= 205,
			kCodePartialContent					= 206,
			kCodeMultiStatus					= 207,
			kCodeAlreadyReported				= 208,
			kCodeIMUsed							= 226,

			kCodeMultipleChoice					= 300,
			kCodeMovedPermanently				= 301,
			kCodeFound							= 302,
			kCodeSeeOther						= 303,
			kCodeNotModified					= 304,
			kCodeUseProxy						= 305,
			kCodeUnused							= 306,
			kCodeTemporaryRedirect				= 307,
			kCodePermanentRedirect				= 308,

			kCodeBadRequest						= 400,
			kCodeUnauthorized					= 401,
			kCodePaymentRequired				= 402,
			kCodeForbidden						= 403,
			kCodeNotFound						= 404,
			kCodeMethodNotAllowed				= 405,
			kCodeNotAcceptable					= 406,
			kCodeProxyAuthenticationRequired	= 407,
			kCodeTimeout						= 408,
			kCodeConflict						= 409,
			kCodeGone							= 410,
			kCodeLengthRequired					= 411,
			kCodePreconditionFailed				= 412,
			kCodePayloadTooLarge				= 413,
			kCodeURITooLong						= 414,
			kCodeUnsupportedMediaType			= 415,
			kCodeRangeNotSatisfiable			= 416,
			kCodeExpectationFailed				= 417,
			kCodeImATeapot						= 418,
			kCodeMisdirectedRequest				= 421,
			kCodeUnprocessableEntity			= 422,
			kCodeLocked							= 423,
			kCodeFailedDependency				= 424,
			kCodeTooEarly						= 425,
			kCodeUpgradeRequired				= 426,
			kCodePreconditionRequired			= 428,
			kCodeTooManyRequests				= 429,
			kCodeRequestHeaderFieldsTooLarge	= 431,
			kCodeUnavailableForLegalReasons		= 451,

			kCodeInternalServerError			= 500,
			kCodeNotImplemented					= 501,
			kCodeBadGateway						= 502,
			kCodeServiceUnavailable				= 503,
			kCodeGatewayTimeout					= 504,
			kCodeHTTPVersionNotSupported		= 505,
			kCodeVariantAlsoNegotiates			= 506,
			kCodeInsufficientStorage			= 507,
			kCodeLoopDetected					= 508,
			kCodeNotExtended					= 510,
			kCodeNetworkAuthenticationRequired	= 511,
		};

	// Methods
	public:
				// Lifecycle methods
				SHTTPEndpointStatus(UInt32 code) : mCode(code) {}

				// Instance methods
		UInt32	getCode() const
					{ return mCode; }
		bool	isSuccess() const
					{ return (mCode >= 200) && (mCode < 300); }
		CString	getDescription() const;

		bool	operator==(Code code) const
					{ return mCode == (UInt32) code; }
		bool	operator!=(Code code) const
					{ return mCode != (UInt32) code; }

	// Properties
	private:
		UInt32	mCode;
};
