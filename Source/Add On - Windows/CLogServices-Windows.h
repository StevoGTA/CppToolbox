//----------------------------------------------------------------------------------------------------------------------
//	CLogServices-Windows.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CLogServices.h"

#define	LogHRESULT(result, method)														\
				CLogServices::logError(													\
						CString(method) + CString(OSSTR(" returned ")) +				\
								SErrorFromHRESULT(result).getDescription())
#define	LogHRESULTIfFailed(result, method)												\
				{																		\
					if (FAILED(result))													\
						CLogServices::logError(											\
								CString(method) + CString(OSSTR(" returned ")) +		\
										SErrorFromHRESULT(result).getDescription());	\
				}

#define LogWindowsError(method)															\
				{																		\
					DWORD	error = GetLastError();										\
					CLogServices::logError(												\
							CString(method) + CString(OSSTR(" returned ")) +			\
									SErrorFromWindowsError(error).getDescription());	\
				}
#define LogWindowsErrorIf(condition, method)											\
				if (condition) {														\
					DWORD	error = GetLastError();										\
					CLogServices::logError(												\
							CString(method) + CString(OSSTR(" returned ")) +			\
									SErrorFromWindowsError(error).getDescription());	\
				}
