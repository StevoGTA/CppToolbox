//----------------------------------------------------------------------------------------------------------------------
//	CLogServices.h			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CppToolboxError.h"
#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Procs

typedef	void	(*CLogProc)(const CString& string, bool silentWarningsAndErrors, void* userData);

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

#if defined(__OBJC__)
	#define	LogNSError(error, when)															\
				CLogServices::logError(CString((CFStringRef) [error localizedDescription]),	\
						when, __FILE__, __func__, __LINE__);
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CLogServices

//class CFolderX;
//class CFileX;

class CLogServices {

	// Methods
	public:
								// Class methods
//		static	const	CFileX&	setup(const CString& filename, const CString& productAndVersion,
//										bool startNewFile = true);
//		static	const	CFileX&	getLogFile();
		
		static			void	logMessage(const CString& string);
		static			void	logWarning(const CString& warning, const CString& when, const char* file,
										const char* proc, UInt32 line);
		static			void	logWarning(const CString& string);
		static			void	logError(const CString& error, const CString& when, const char* file,
										const char* proc, UInt32 line);
		static			void	logError(const CString& string);
		static			void	logError(UError error, const CString& message, const char* file, const char* proc,
										UInt32 line)
									{ logError(CErrorRegistry::getStringForError(error), message, file, proc, line); }

//		static			void	log(const CFileX& file);
//		static			void	log(const CFolderX& folder);

		static			void	setLogMessageProc(CLogProc logProc, void* logProcUserData = nil);
		static			void	setLogWarningProc(CLogProc logProc, void* logProcUserData = nil);
		static			void	setLogErrorProc(CLogProc logProc, void* logProcUserData = nil);

		static			bool	getSilentWarningsAndErrors();
		static			void	setSilentWarningsAndErrors(bool silentWarningsAndErrors);
};
