//----------------------------------------------------------------------------------------------------------------------
//	CLogServices.h			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	LogWarning(warning, when)	CLogServices::logWarning(warning, when, __FILE__, __func__, __LINE__)

#define	LogError(error, when)	CLogServices::logError(error, when, __FILE__, __func__, __LINE__)

#define LogErrorAndReturnValue(error, when, value)										\
			CLogServices::logError(error, when, __FILE__, __func__, __LINE__);			\
			return value;																\

#define LogIfError(error, when)															\
			{																			\
				if (error.hasValue())													\
					CLogServices::logError(*error, when, __FILE__, __func__, __LINE__);	\
			}

#define	LogIfErrorAndReturn(error, when)												\
			{																			\
				if (error.hasValue()) {													\
					CLogServices::logError(*error, when, __FILE__, __func__, __LINE__);	\
					return;																\
				}																		\
			}

#define LogIfErrorAndReturnError(error, when)											\
			{																			\
				if (error.hasValue()) {													\
					CLogServices::logError(*error, when, __FILE__, __func__, __LINE__);	\
					return error;														\
				}																		\
			}

#define	LogIfErrorAndReturnValue(error, when, value)									\
			{																			\
				if (error.hasValue()) {													\
					CLogServices::logError(*error, when, __FILE__, __func__, __LINE__);	\
					return value;														\
				}																		\
			}

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CLogServices

class CLogServices {
	// Procs
	public:
		typedef	void	(*LogProc)(const CString& string, void* userData);

	// Methods
	public:
						// Class methods

		static	void	logMessage(const CString& string);
		static	void	logMessages(const TArray<CString>& strings);
		static	void	logDebugMessage(const CString& string);
		static	void	logWarning(const CString& warning, const CString& when, const char* file, const char* proc,
								UInt32 line);
		static	void	logWarning(const CString& string);
		static	void	logError(const CString& error, const CString& when, const char* file, const char* proc,
								UInt32 line);
		static	void	logError(const CString& string);
		static	void	logError(const SError& error, const CString& message, const char* file, const char* proc,
								UInt32 line)
							{ logError(error.getDescription(), message, file, proc, line); }

		static	void	addLogMessageProc(LogProc logProc, void* userData = nil);
		static	void	addLogWarningProc(LogProc logProc, void* userData = nil);
		static	void	addLogErrorProc(LogProc logProc, void* userData = nil);

		static	void	enableConsoleLogging();
};
