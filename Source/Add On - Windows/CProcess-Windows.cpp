//----------------------------------------------------------------------------------------------------------------------
//	CProcess-Windows.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CProcess.h"

#include "CThread.h"
#include "CUUID.h"
#include "SError-Windows.h"
#include "TLockingValue.h"

#include <string>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	const	TCHAR*	sMessageChannelEnvironmentVariableName = L"CProcessMessageChannel";

static	CString	sErrorDomain(OSSTR("CProcess"));
static	SError	sNoMessageChannelError(sErrorDomain, 1, CString(OSSTR("No message channel")));

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
static OV<SError> sCreateMessageChannelPipe(bool isToChild, HANDLE& parentHandle, HANDLE& childHandle)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create the parent's end
	CString	name = CString(OSSTR("\\\\.\\pipe\\CProcess-")) + CUUID().getHexString();
	parentHandle =
			::CreateNamedPipe(name.getOSString(),
					(isToChild ? PIPE_ACCESS_OUTBOUND : PIPE_ACCESS_INBOUND) | FILE_FLAG_FIRST_PIPE_INSTANCE |
							FILE_FLAG_OVERLAPPED,
					PIPE_TYPE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS, 1, 65536, 65536, 0, NULL);
	if (parentHandle == INVALID_HANDLE_VALUE)
		// Error
		return OV<SError>(SErrorFromWindowsGetLastError());

	// Open the child's end, which the child inherits
	SECURITY_ATTRIBUTES	securityAttributes = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
	childHandle =
			::CreateFile(name.getOSString(), isToChild ? GENERIC_READ : GENERIC_WRITE, 0, &securityAttributes,
					OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (childHandle == INVALID_HANDLE_VALUE) {
		// Error
		SError	error = SErrorFromWindowsGetLastError();
		::CloseHandle(parentHandle);

		return OV<SError>(error);
	}

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
static HANDLE sCreateJobHandle()
//----------------------------------------------------------------------------------------------------------------------
{
	// Create job whose processes end when this process ends
	HANDLE									jobHandle = ::CreateJobObject(NULL, NULL);
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION	limitInformation = {};
	limitInformation.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	::SetInformationJobObject(jobHandle, JobObjectExtendedLimitInformation, &limitInformation,
			sizeof(limitInformation));

	return jobHandle;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CProcess

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TIResult<CMessageChannel> CProcess::getMessageChannel()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if launched with a message channel
	TCHAR				value[64];
	DWORD				length =
								::GetEnvironmentVariable(sMessageChannelEnvironmentVariableName, value,
										sizeof(value) / sizeof(TCHAR));
	unsigned long long	readHandleValue;
	unsigned long long	writeHandleValue;
	if ((length == 0) || (length >= (sizeof(value) / sizeof(TCHAR))) ||
			(::swscanf_s(value, L"%llu,%llu", &readHandleValue, &writeHandleValue) != 2))
		// No
		return TIResult<CMessageChannel>(sNoMessageChannelError);

	// Keep the message channel from any child of this process
	HANDLE	readHandle = (HANDLE) (UINT_PTR) readHandleValue;
	HANDLE	writeHandle = (HANDLE) (UINT_PTR) writeHandleValue;
	::SetEnvironmentVariable(sMessageChannelEnvironmentVariableName, NULL);
	::SetHandleInformation(readHandle, HANDLE_FLAG_INHERIT, 0);
	::SetHandleInformation(writeHandle, HANDLE_FLAG_INHERIT, 0);

	return TIResult<CMessageChannel>(OI<CMessageChannel>(new CMessageChannel(readHandle, writeHandle)));
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CChildProcess::Internals

class CChildProcess::Internals {
	public:
						Internals(ProcessRef processRef, CMessageChannel::Handle outputReadHandle,
								CMessageChannel::Handle messageChannelReadHandle,
								CMessageChannel::Handle messageChannelWriteHandle) :
							mProcessRef(processRef), mOutputReadHandle(outputReadHandle),
									mMessageChannel(messageChannelReadHandle, messageChannelWriteHandle),
									mOutputThread(readOutput, this, CString(OSSTR("CChildProcess output")),
											CThread::kOptionsAutoStart)
							{}
						~Internals()
							{
								// Check if ended
								if (!*mHasEnded)
									// Kill
									kill();

								// Wait for output to finish
								mOutputThread.waitUntilFinished();
								::CloseHandle(mOutputReadHandle);
								::CloseHandle(mProcessRef);
							}

				void	kill()
							{
								// Kill
								::TerminateProcess(mProcessRef, 1);
								::WaitForSingleObject(mProcessRef, INFINITE);

								mHasEnded.set();
							}

		static	void	readOutput(CThread& thread, void* userData)
							{
								// Setup
								Internals&	internals = *((Internals*) userData);
								UInt8		buffer[4096];

								// Read until the output closes, or the child has ended and nothing remains
								while (true) {
									// Check for output
									DWORD	availableByteCount = 0;
									if (!::PeekNamedPipe(internals.mOutputReadHandle, NULL, 0, NULL,
											&availableByteCount, NULL))
										// Output closed
										break;
									else if (availableByteCount > 0) {
										// Read
										DWORD	byteCount = 0;
										if (!::ReadFile(internals.mOutputReadHandle, buffer,
												(availableByteCount < sizeof(buffer)) ?
														availableByteCount : (DWORD) sizeof(buffer),
												&byteCount, NULL))
											// Output closed
											break;
										internals.mOutputData += CData(buffer, byteCount, false);
									} else if (*internals.mHasEnded)
										// Ended with nothing remaining
										break;
									else
										// Wait a bit
										CThread::sleepFor(0.01);
								}
							}

		ProcessRef				mProcessRef;
		CMessageChannel::Handle	mOutputReadHandle;
		CMessageChannel			mMessageChannel;
		SLockingBoolean			mHasEnded;

		CThread					mOutputThread;

		CData					mOutputData;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CChildProcess

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CChildProcess::CChildProcess(ProcessRef processRef, CMessageChannel::Handle outputReadHandle,
		CMessageChannel::Handle messageChannelReadHandle, CMessageChannel::Handle messageChannelWriteHandle) :
	mInternals(new Internals(processRef, outputReadHandle, messageChannelReadHandle, messageChannelWriteHandle))
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
CChildProcess::~CChildProcess()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CMessageChannel& CChildProcess::getMessageChannel()
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mMessageChannel;
}

//----------------------------------------------------------------------------------------------------------------------
CChildProcess::Result CChildProcess::finish(UniversalTimeInterval timeout)
//----------------------------------------------------------------------------------------------------------------------
{
	// Close the message channel so the child sees the end of its input
	mInternals->mMessageChannel.close();

	// Wait for the child to end; kill it if the timeout elapses
	bool	timedOut = ::WaitForSingleObject(mInternals->mProcessRef, (DWORD) (timeout * 1000.0)) != WAIT_OBJECT_0;
	if (timedOut)
		// Timed out
		mInternals->kill();
	else
		// Ended
		mInternals->mHasEnded.set();

	DWORD	exitCode = 0;
	::GetExitCodeProcess(mInternals->mProcessRef, &exitCode);

	// Wait for output to finish
	mInternals->mOutputThread.waitUntilFinished();

	// Check situation
	if (timedOut)
		// Timed out
		return Result(Result::kTerminationTimedOut, mInternals->mOutputData);
	else if ((exitCode & 0xC0000000) == 0xC0000000)
		// Crashed
		return Result(Result::kTerminationCrashed, (SInt32) exitCode, mInternals->mOutputData);
	else
		// Normal
		return Result(Result::kTerminationExited, (SInt32) exitCode, mInternals->mOutputData);
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TIResult<CChildProcess> CChildProcess::launch(const CFile& executableFile, const TArray<CString>& arguments)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create pipe for the child's output
	SECURITY_ATTRIBUTES	securityAttributes = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
	HANDLE				outputReadHandle;
	HANDLE				outputWriteHandle;
	if (!::CreatePipe(&outputReadHandle, &outputWriteHandle, &securityAttributes, 0))
		// Error
		return TIResult<CChildProcess>(SErrorFromWindowsGetLastError());
	::SetHandleInformation(outputReadHandle, HANDLE_FLAG_INHERIT, 0);

	// Create pipes for the message channel
	HANDLE		messageChannelWriteHandle;
	HANDLE		childMessageChannelReadHandle;
	HANDLE		messageChannelReadHandle;
	HANDLE		childMessageChannelWriteHandle;
	OV<SError>	error = sCreateMessageChannelPipe(true, messageChannelWriteHandle, childMessageChannelReadHandle);
	if (error.hasValue()) {
		// Error
		::CloseHandle(outputReadHandle);
		::CloseHandle(outputWriteHandle);

		return TIResult<CChildProcess>(*error);
	}

	error = sCreateMessageChannelPipe(false, messageChannelReadHandle, childMessageChannelWriteHandle);
	if (error.hasValue()) {
		// Error
		::CloseHandle(outputReadHandle);
		::CloseHandle(outputWriteHandle);
		::CloseHandle(messageChannelWriteHandle);
		::CloseHandle(childMessageChannelReadHandle);

		return TIResult<CChildProcess>(*error);
	}

	// Compose command line
	CString			executablePath = executableFile.getFilesystemPath().getString();
	std::wstring	commandLine;
	sAppendQuotedArgument(commandLine, executablePath.getOSString());
	for (TArray<CString>::Iterator iterator = arguments.getIterator(); iterator; iterator++) {
		// Add argument
		commandLine += L' ';
		sAppendQuotedArgument(commandLine, iterator->getOSString());
	}

	// Compose environment, marking the message channel for the child
	std::wstring	environment;
	LPWCH			environmentStrings = ::GetEnvironmentStringsW();
	for (LPWCH entry = environmentStrings; *entry != 0; entry += ::wcslen(entry) + 1)
		// Add entry
		environment.append(entry, ::wcslen(entry) + 1);
	::FreeEnvironmentStringsW(environmentStrings);
	environment +=
			std::wstring(sMessageChannelEnvironmentVariableName) + L"=" +
					std::to_wstring((unsigned long long) (UINT_PTR) childMessageChannelReadHandle) + L"," +
					std::to_wstring((unsigned long long) (UINT_PTR) childMessageChannelWriteHandle);
	environment += L'\0';

	// Limit what the child inherits to its output and its message channel
	HANDLE	inheritedHandles[] = {outputWriteHandle, childMessageChannelReadHandle, childMessageChannelWriteHandle};
	SIZE_T	attributeListByteCount = 0;
	::InitializeProcThreadAttributeList(NULL, 1, 0, &attributeListByteCount);

	CData							attributeListData((CData::ByteCount) attributeListByteCount);
	LPPROC_THREAD_ATTRIBUTE_LIST	attributeList =
											(LPPROC_THREAD_ATTRIBUTE_LIST)
													*attributeListData.getMutableBuffer(attributeListByteCount);
	::InitializeProcThreadAttributeList(attributeList, 1, 0, &attributeListByteCount);
	::UpdateProcThreadAttribute(attributeList, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, inheritedHandles,
			sizeof(inheritedHandles), NULL, NULL);

	// Create process, suspended so it can be placed in the job before it runs
	STARTUPINFOEX	startupInfo = {};
	startupInfo.StartupInfo.cb = sizeof(STARTUPINFOEX);
	startupInfo.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
	startupInfo.StartupInfo.hStdOutput = outputWriteHandle;
	startupInfo.StartupInfo.hStdError = outputWriteHandle;
	startupInfo.lpAttributeList = attributeList;

	PROCESS_INFORMATION	processInformation;
	BOOL				created =
								::CreateProcess(executablePath.getOSString(), &commandLine[0], NULL, NULL, TRUE,
										CREATE_NO_WINDOW | CREATE_SUSPENDED | CREATE_UNICODE_ENVIRONMENT |
												EXTENDED_STARTUPINFO_PRESENT,
										&environment[0], NULL, &startupInfo.StartupInfo, &processInformation);
	OV<SError>			createError = created ? OV<SError>() : OV<SError>(SErrorFromWindowsGetLastError());

	// Cleanup
	::DeleteProcThreadAttributeList(attributeList);
	::CloseHandle(outputWriteHandle);
	::CloseHandle(childMessageChannelReadHandle);
	::CloseHandle(childMessageChannelWriteHandle);

	// Check result
	if (!created) {
		// Error
		::CloseHandle(outputReadHandle);
		::CloseHandle(messageChannelReadHandle);
		::CloseHandle(messageChannelWriteHandle);

		return TIResult<CChildProcess>(*createError);
	}

	// Place in the job so the child ends with this process, then start
	static	HANDLE	sJobHandle = sCreateJobHandle();
	::AssignProcessToJobObject(sJobHandle, processInformation.hProcess);
	::ResumeThread(processInformation.hThread);
	::CloseHandle(processInformation.hThread);

	return TIResult<CChildProcess>(
			OI<CChildProcess>(
					new CChildProcess(processInformation.hProcess, outputReadHandle, messageChannelReadHandle,
							messageChannelWriteHandle)));
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CChildProcess::Result> CChildProcess::run(const CFile& executableFile, const TArray<CString>& arguments,
		UniversalTimeInterval timeout)
//----------------------------------------------------------------------------------------------------------------------
{
	// Launch
	TIResult<CChildProcess>	childProcess = launch(executableFile, arguments);
	ReturnValueIfResultError(childProcess, TVResult<Result>(childProcess.getError()));

	return TVResult<Result>(childProcess.getOptionalInstance()->finish(timeout));
}
