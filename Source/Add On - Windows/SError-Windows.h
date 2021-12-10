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

#define	ReturnError(result, method)														\
				{																		\
					SError	error = SErrorFromHRESULT(result);							\
					CLogServices::logError(												\
							CString(method) + CString(OSSTR(" returned ")) +			\
									error.getDescription());							\
																						\
					return OI<SError>(error);											\
				}
#define	ReturnErrorIfFailed(result, method)												\
				{																		\
					if (FAILED(result)) {												\
						SError	error = SErrorFromHRESULT(result);						\
						CLogServices::logError(											\
								CString(method) + CString(OSSTR(" returned ")) +		\
										error.getDescription());						\
																						\
						return OI<SError>(error);										\
					}																	\
				}
#define	ReturnValueIfFailed(result, method, value)										\
				{																		\
					if (FAILED(result)) {												\
						SError	error = SErrorFromHRESULT(result);						\
						CLogServices::logError(											\
								CString(method) + CString(OSSTR(" returned ")) +		\
										error.getDescription());						\
																						\
						return value;										\
					}																	\
				}
