//----------------------------------------------------------------------------------------------------------------------
//	CChildProcess-Windows.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CChildProcess.h"

#include "SError-Windows.h"

#include <string>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local procs

//----------------------------------------------------------------------------------------------------------------------
static void sAppendQuotedArgument(std::wstring& commandLine, const TCHAR* argument)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if need quoting
	if ((*argument != 0) && (::wcspbrk(argument, L" \t\"") == NULL))
		// No quoting needed
		commandLine += argument;
	else {
		// Need quoting
		commandLine += L'"';
		for (const TCHAR* p = argument; ; p++) {
			// Count backslashes
			size_t	backslashCount = 0;
			while (*p == L'\\') {
				backslashCount++;
				p++;
			}

			// Check what follows them
			if (*p == 0) {
				// End; backslashes before the closing quote are doubled
				commandLine.append(backslashCount * 2, L'\\');
				break;
			} else if (*p == L'"') {
				// Quote; backslashes are doubled and the quote is escaped
				commandLine.append(backslashCount * 2 + 1, L'\\');
				commandLine += L'"';
			} else {
				// Other; backslashes are literal
				commandLine.append(backslashCount, L'\\');
				commandLine += *p;
			}
		}
		commandLine += L'"';
	}
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CChildProcess

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TVResult<CChildProcess::Result> CChildProcess::run(const CFile& executableFile, const TArray<CString>& arguments,
		UniversalTimeInterval timeout)
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose command line
	CString			executablePath = executableFile.getFilesystemPath().getString();
	std::wstring	commandLine;
	sAppendQuotedArgument(commandLine, executablePath.getOSString());
	for (TArray<CString>::Iterator iterator = arguments.getIterator(); iterator; iterator++) {
		// Add argument
		commandLine += L' ';
		sAppendQuotedArgument(commandLine, iterator->getOSString());
	}

	// Create pipe for the child's standard output and standard error
	SECURITY_ATTRIBUTES	securityAttributes = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
	HANDLE				readHandle;
	HANDLE				writeHandle;
	if (!::CreatePipe(&readHandle, &writeHandle, &securityAttributes, 0))
		// Error
		return TVResult<Result>(SErrorFromWindowsGetLastError());
	::SetHandleInformation(readHandle, HANDLE_FLAG_INHERIT, 0);

	// Create job so the child ends with us
	HANDLE									jobHandle = ::CreateJobObject(NULL, NULL);
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION	limitInformation = {};
	limitInformation.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	::SetInformationJobObject(jobHandle, JobObjectExtendedLimitInformation, &limitInformation,
			sizeof(limitInformation));

	// Create process, suspended so it can be placed in the job before it runs
	STARTUPINFO	startupInfo = {sizeof(STARTUPINFO)};
	startupInfo.dwFlags = STARTF_USESTDHANDLES;
	startupInfo.hStdInput = ::GetStdHandle(STD_INPUT_HANDLE);
	startupInfo.hStdOutput = writeHandle;
	startupInfo.hStdError = writeHandle;

	PROCESS_INFORMATION	processInformation;
	BOOL				created =
								::CreateProcess(executablePath.getOSString(), &commandLine[0], NULL, NULL, TRUE,
										CREATE_NO_WINDOW | CREATE_SUSPENDED, NULL, NULL, &startupInfo,
										&processInformation);
	::CloseHandle(writeHandle);
	if (!created) {
		// Error
		SError	error = SErrorFromWindowsGetLastError();
		::CloseHandle(readHandle);
		::CloseHandle(jobHandle);

		return TVResult<Result>(error);
	}
	::AssignProcessToJobObject(jobHandle, processInformation.hProcess);
	::ResumeThread(processInformation.hThread);
	::CloseHandle(processInformation.hThread);

	// Read output until the child closes its end or the timeout elapses
	UniversalTime	deadline = SUniversalTime::getCurrent() + timeout;
	CData			outputData;
	bool			timedOut = false;
	UInt8			buffer[4096];
	while (true) {
		// Check for output
		DWORD	availableByteCount = 0;
		if (!::PeekNamedPipe(readHandle, NULL, 0, NULL, &availableByteCount, NULL))
			// Pipe closed
			break;

		if (availableByteCount > 0) {
			// Read
			DWORD	byteCount = 0;
			if (!::ReadFile(readHandle, buffer,
					(availableByteCount < sizeof(buffer)) ? availableByteCount : (DWORD) sizeof(buffer), &byteCount,
					NULL))
				// Pipe closed
				break;
			outputData += CData(buffer, byteCount, false);
		} else if (::WaitForSingleObject(processInformation.hProcess, 10) == WAIT_OBJECT_0) {
			// Ended, collect whatever remains
			while (::PeekNamedPipe(readHandle, NULL, 0, NULL, &availableByteCount, NULL) && (availableByteCount > 0)) {
				// Read
				DWORD	byteCount = 0;
				if (!::ReadFile(readHandle, buffer,
						(availableByteCount < sizeof(buffer)) ? availableByteCount : (DWORD) sizeof(buffer), &byteCount,
						NULL))
					// Pipe closed
					break;
				outputData += CData(buffer, byteCount, false);
			}
			break;
		} else if (SUniversalTime::getCurrent() >= deadline) {
			// Timed out
			timedOut = true;
			break;
		}
	}
	::CloseHandle(readHandle);

	// Wait for the child to end, which may lag its output; kill it if the timeout elapses
	UniversalTimeInterval	remaining = deadline - SUniversalTime::getCurrent();
	DWORD					remainingMilliseconds = (remaining > 0.0) ? (DWORD) (remaining * 1000.0) : 0;
	if (timedOut || (::WaitForSingleObject(processInformation.hProcess, remainingMilliseconds) != WAIT_OBJECT_0)) {
		// Timed out
		timedOut = true;
		::TerminateProcess(processInformation.hProcess, 1);
		::WaitForSingleObject(processInformation.hProcess, INFINITE);
	}

	DWORD	exitCode = 0;
	::GetExitCodeProcess(processInformation.hProcess, &exitCode);
	::CloseHandle(processInformation.hProcess);
	::CloseHandle(jobHandle);

	// Check situation
	if (timedOut)
		return TVResult<Result>(Result(Result::kTerminationTimedOut, outputData));
	else if ((exitCode & 0xC0000000) == 0xC0000000)
		return TVResult<Result>(Result(Result::kTerminationCrashed, (SInt32) exitCode, outputData));
	else
		return TVResult<Result>(Result(Result::kTerminationExited, (SInt32) exitCode, outputData));
}
