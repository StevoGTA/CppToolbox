//----------------------------------------------------------------------------------------------------------------------
//	CLogServices.h			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CppToolboxError.h"
#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Procs

typedef	void	(*CLogProc)(const CString& string, void* userData);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Macros

#define	LogWarning(warning, when)	CLogServices::logWarning(warning, when, __FILE__, __func__, __LINE__)
#define	LogError(error, when)	CLogServices::logError(error, when, __FILE__, __func__, __LINE__)

#define LogIfError(error, when)																\
		{																					\
			if (error != kNoError)															\
				CLogServices::logError(error, when, __FILE__, __func__, __LINE__);			\
		}

#define	LogIfErrorAndReturn(error, when)													\
		{																					\
			if (error != kNoError) {														\
				CLogServices::logError(error, when, __FILE__, __func__, __LINE__);			\
				return;																		\
			}																				\
		}

#define LogIfErrorAndReturnError(error, when)												\
		{																					\
			if (error != kNoError) {														\
				CLogServices::logError(error, when, __FILE__, __func__, __LINE__);			\
				return error;																\
			}																				\
		}

#define	LogIfErrorAndReturnValue(error, when, value)										\
		{																					\
			if (error != kNoError) {														\
				CLogServices::logError(error, when, __FILE__, __func__, __LINE__);			\
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
				void	logError(UError error, const CString& message, const char* file, const char* proc, UInt32 line)
							{ logError(CErrorRegistry::getStringForError(error), message, file, proc, line); }

	// Properties
	private:
		CLogFileInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CLogServices

class CLogServices {
	// Methods
	public:
								// Class methods
		static	void			setPrimaryLogFile(const CLogFile& logFile);
		static	OO<CLogFile>&	getPrimaryLogFile();

		static	void			logMessage(const CString& string);
		static	void			logDebugMessage(const CString& string);
		static	void			logWarning(const CString& warning, const CString& when, const char* file,
										const char* proc, UInt32 line);
		static	void			logWarning(const CString& string);
		static	void			logError(const CString& error, const CString& when, const char* file, const char* proc,
										UInt32 line);
		static	void			logError(const CString& string);
		static	void			logError(UError error, const CString& message, const char* file, const char* proc,
										UInt32 line)
									{ logError(CErrorRegistry::getStringForError(error), message, file, proc, line); }

		static	void			addLogMessageProc(CLogProc logProc, void* logProcUserData = nil);
		static	void			addLogWarningProc(CLogProc logProc, void* logProcUserData = nil);
		static	void			addLogErrorProc(CLogProc logProc, void* logProcUserData = nil);
};
