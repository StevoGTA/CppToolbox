//----------------------------------------------------------------------------------------------------------------------
//	SError-Windows.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SError.h"

#undef Delete
#include <Windows.h>
#define Delete(x)	{ delete x; x = nil; }

//----------------------------------------------------------------------------------------------------------------------
// MARK: Global methods

SError	SErrorFromHRESULT(HRESULT result);
SError	SErrorFromWindowsError(DWORD error);
SError	SErrorFromWindowsGetLastError();

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Macros

#define LogFailedHRESULT(result, method)																		\
				{																								\
					SError	_error = SErrorFromHRESULT(result);													\
					CLogServices::logError(																		\
							CString(method) + CString(OSSTR(" returned ")) + _error.getDefaultDescription());	\
				}
#define	ReturnError(result, method)																				\
				{																								\
					SError	_error = SErrorFromHRESULT(result);													\
					CLogServices::logError(																		\
							CString(method) + CString(OSSTR(" returned ")) + _error.getDefaultDescription());	\
																												\
					return OV<SError>(_error);																	\
				}
#define	ReturnErrorIfFailed(result, method)																			\
				{																									\
					if (FAILED(result)) {																			\
						SError	_error = SErrorFromHRESULT(result);													\
						CLogServices::logError(																		\
								CString(method) + CString(OSSTR(" returned ")) + _error.getDefaultDescription());	\
																													\
						return OV<SError>(_error);																	\
					}																								\
				}
#define	ReturnValueIfFailed(result, method, value)																	\
				{																									\
					if (FAILED(result)) {																			\
						SError	_error = SErrorFromHRESULT(result);													\
						CLogServices::logError(																		\
								CString(method) + CString(OSSTR(" returned ")) + _error.getDefaultDescription());	\
																													\
						return value;																				\
					}																								\
				}
