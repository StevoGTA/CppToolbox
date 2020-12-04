//----------------------------------------------------------------------------------------------------------------------
//	CLogServices.h			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	LogWarning(warning, when)	CLogServices::logWarning(warning, when, __FILE__, __func__, __LINE__)
#define	LogError(error, when)	CLogServices::logError(error, when, __FILE__, __func__, __LINE__)

#define LogIfError(error, when)																\
		{																					\
			if (error.hasInstance())														\
				CLogServices::logError(*error, when, __FILE__, __func__, __LINE__);			\
		}

#define	LogIfErrorAndReturn(error, when)													\
		{																					\
			if (error.hasInstance()) {														\
				CLogServices::logError(*error, when, __FILE__, __func__, __LINE__);			\
				return;																		\
			}																				\
		}

#define LogIfErrorAndReturnError(error, when)												\
		{																					\
			if (error.hasInstance()) {														\
				CLogServices::logError(*error, when, __FILE__, __func__, __LINE__);			\
				return error;																\
			}																				\
		}

#define	LogIfErrorAndReturnValue(error, when, value)										\
		{																					\
			if (error.hasInstance()) {														\
				CLogServices::logError(*error, when, __FILE__, __func__, __LINE__);			\
				return value;																\
			}																				\
		}

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CLogFile

class CFile;

class CLogFileInternals;
class CLogFile {
	// Methods
	public:
						// Lifecycle methods
						CLogFile(const CFile& file);
						CLogFile(const CLogFile& other);
						~CLogFile();

						// Instance methods
		const	CFile& getFile() const;

				void	logMessage(const CString& string) const;
				void	logWarning(const CString& warning, const CString& when, const char* file, const char* proc,
								UInt32 line) const;
				void	logWarning(const CString& string) const;
				void	logError(const CString& error, const CString& when, const char* file, const char* proc,
								UInt32 line) const;
				void	logError(const CString& string) const;
				void	logError(const SError& error, const CString& message, const char* file, const char* proc,
								UInt32 line)
							{ logError(error.getDescription(), message, file, proc, line); }

	// Properties
	private:
		CLogFileInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CLogServices

class CLogServices {
	// Procs
	public:
		typedef	void	(*LogProc)(const CString& string, void* userData);

	// Methods
	public:
								// Class methods
		static	void			setPrimaryLogFile(const CLogFile& logFile);
		static	OI<CLogFile>&	getPrimaryLogFile();

		static	void			logMessage(const CString& string);
		static	void			logDebugMessage(const CString& string);
		static	void			logWarning(const CString& warning, const CString& when, const char* file,
										const char* proc, UInt32 line);
		static	void			logWarning(const CString& string);
		static	void			logError(const CString& error, const CString& when, const char* file, const char* proc,
										UInt32 line);
		static	void			logError(const CString& string);
		static	void			logError(const SError& error, const CString& message, const char* file,
										const char* proc, UInt32 line)
									{ logError(error.getDescription(), message, file, proc, line); }

		static	void			addLogMessageProc(LogProc logProc, void* userData = nil);
		static	void			addLogWarningProc(LogProc logProc, void* userData = nil);
		static	void			addLogErrorProc(LogProc logProc, void* userData = nil);
};
