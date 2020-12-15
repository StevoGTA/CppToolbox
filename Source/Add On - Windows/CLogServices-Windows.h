//----------------------------------------------------------------------------------------------------------------------
//	CLogServices-Windows.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CLogServices.h"

#define	LOG_HRESULT(result, method)														\
				CLogServices::logError(													\
						CString(method) + CString(OSSTR(" returned ")) +				\
								SErrorFromHRESULT(result).getDescription())
#define	LOG_HRESULT_IF_FAILED(result, method)											\
				{																		\
					if (FAILED(result))													\
						CLogServices::logError(											\
								CString(method) + CString(OSSTR(" returned ")) +		\
										SErrorFromHRESULT(result).getDescription());	\
				}

#define LOG_WINDOWS_ERROR(method)														\
				{																		\
					DWORD	error = GetLastError();										\
					CLogServices::logError(												\
							CString(method) + CString(OSSTR(" returned ")) +			\
									SErrorFromWindowsError(error).getDescription());	\
				}
#define LOG_WINDOWS_ERROR_IF(condition, method)											\
				if (condition) {														\
					DWORD	error = GetLastError();										\
					CLogServices::logError(												\
							CString(method) + CString(OSSTR(" returned ")) +			\
									SErrorFromWindowsError(error).getDescription());	\
				}
