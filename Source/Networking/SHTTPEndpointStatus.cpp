//----------------------------------------------------------------------------------------------------------------------
//	SHTTPEndpointStatus.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SHTTPEndpointStatus.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SHTTPEndpointStatus

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CString SHTTPEndpointStatus::getDescription() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check code
	switch (mCode) {
		case kCodeContinue:							return CString(OSSTR("Continue"));
		case kCodeSwitchingProtocol:				return CString(OSSTR("Switching Protocol"));
		case kCodeProcessing:						return CString(OSSTR("Processing"));
		case kCodeEarlyHints:						return CString(OSSTR("Early Hints"));

		case kCodeOK:								return CString(OSSTR("OK"));
		case kCodeCreated:							return CString(OSSTR("Created"));
		case kCodeAccepted:							return CString(OSSTR("Accepted"));
		case kCodeNonAuthoritativeInformation:		return CString(OSSTR("Non-Authoritative Information"));
		case kCodeNoContent:						return CString(OSSTR("No Content"));
		case kCodeResetContent:						return CString(OSSTR("Reset Content"));
		case kCodePartialContent:					return CString(OSSTR("Partial Content"));
		case kCodeMultiStatus:						return CString(OSSTR("Multi-Status (WebDAV)"));
		case kCodeAlreadyReported:					return CString(OSSTR("Already Reported (WebDAV)"));
		case kCodeIMUsed:							return CString(OSSTR("IM Used (HTTP Delta encoding)"));

		case kCodeMultipleChoice:					return CString(OSSTR("Multiple Choice"));
		case kCodeMovedPermanently:					return CString(OSSTR("Moved Permanently"));
		case kCodeFound:							return CString(OSSTR("Found"));
		case kCodeSeeOther:							return CString(OSSTR("See Other"));
		case kCodeNotModified:						return CString(OSSTR("Not Modified"));
		case kCodeUseProxy:							return CString(OSSTR("Use Proxy"));
		case kCodeUnused:							return CString(OSSTR("Unused"));
		case kCodeTemporaryRedirect:				return CString(OSSTR("Temporary Redirect"));
		case kCodePermanentRedirect:				return CString(OSSTR("Permanent Redirect"));

		case kCodeBadRequest:						return CString(OSSTR("Bad Request"));
		case kCodeUnauthorized:						return CString(OSSTR("Unauthorized"));
		case kCodePaymentRequired:					return CString(OSSTR("Payment Required"));
		case kCodeForbidden:						return CString(OSSTR("Forbidden"));
		case kCodeNotFound:							return CString(OSSTR("Not Found"));
		case kCodeMethodNotAllowed:					return CString(OSSTR("Method Not Allowed"));
		case kCodeNotAcceptable:					return CString(OSSTR("Not Acceptable"));
		case kCodeProxyAuthenticationRequired:		return CString(OSSTR("Proxy Authentication Required"));
		case kCodeTimeout:							return CString(OSSTR("Request Timeout"));
		case kCodeConflict:							return CString(OSSTR("Conflict"));
		case kCodeGone:								return CString(OSSTR("Gone"));
		case kCodeLengthRequired:					return CString(OSSTR("Length Required"));
		case kCodePreconditionFailed:				return CString(OSSTR("Precondition Failed"));
		case kCodePayloadTooLarge:					return CString(OSSTR("Payload Too Large"));
		case kCodeURITooLong:						return CString(OSSTR("URI Too Long"));
		case kCodeUnsupportedMediaType:				return CString(OSSTR("Unsupported Media Type"));
		case kCodeRangeNotSatisfiable:				return CString(OSSTR("Range Not Satisfiable"));
		case kCodeExpectationFailed:				return CString(OSSTR("Expectation Failed"));
		case kCodeImATeapot:						return CString(OSSTR("I'm a teapot"));
		case kCodeMisdirectedRequest:				return CString(OSSTR("Misdirected Request"));
		case kCodeUnprocessableEntity:				return CString(OSSTR("Unprocessable Entity (WebDAV)"));
		case kCodeLocked:							return CString(OSSTR("Locked (WebDAV)"));
		case kCodeFailedDependency:					return CString(OSSTR("Failed Dependency (WebDAV)"));
		case kCodeTooEarly:							return CString(OSSTR("Too Early"));
		case kCodeUpgradeRequired:					return CString(OSSTR("Upgrade Required"));
		case kCodePreconditionRequired:				return CString(OSSTR("Precondition Required"));
		case kCodeTooManyRequests:					return CString(OSSTR("Too Many Requests"));
		case kCodeRequestHeaderFieldsTooLarge:		return CString(OSSTR("Request Header Fields Too Large"));
		case kCodeUnavailableForLegalReasons:		return CString(OSSTR("Unavailable For Legal Reasons"));

		case kCodeInternalServerError:				return CString(OSSTR("Internal Server Error"));
		case kCodeNotImplemented:					return CString(OSSTR("Not Implemented"));
		case kCodeBadGateway:						return CString(OSSTR("Bad Gateway"));
		case kCodeServiceUnavailable:				return CString(OSSTR("Service Unavailable"));
		case kCodeGatewayTimeout:					return CString(OSSTR("Gateway Timeout"));
		case kCodeHTTPVersionNotSupported:			return CString(OSSTR("HTTP Version Not Supported"));
		case kCodeVariantAlsoNegotiates:			return CString(OSSTR("Variant Also Negotiates"));
		case kCodeInsufficientStorage:				return CString(OSSTR("Insufficient Storage (WebDAV)"));
		case kCodeLoopDetected:						return CString(OSSTR("Loop Detected (WebDAV)"));
		case kCodeNotExtended:						return CString(OSSTR("Not Extended"));
		case kCodeNetworkAuthenticationRequired:	return CString(OSSTR("Network Authentication Required"));

		default:									return CString(OSSTR("Other"));
	}
}
