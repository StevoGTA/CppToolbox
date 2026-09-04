//----------------------------------------------------------------------------------------------------------------------
//	CChildProcess-POSIX.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CChildProcess.h"

#include "CThread.h"
#include "SError-POSIX.h"

#include <cstdlib>
#include <poll.h>
#include <signal.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

extern	char**	environ;

//----------------------------------------------------------------------------------------------------------------------
// MARK: CChildProcess

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TVResult<CChildProcess::Result> CChildProcess::run(const CFile& executableFile, const TArray<CString>& arguments,
		UniversalTimeInterval timeout)
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose argv
	CArray::ItemCount	argumentCount = arguments.getCount();
	char**				argv = new char*[argumentCount + 2];
	argv[0] = ::strdup(*executableFile.getFilesystemPath().getString().getUTF8String());
	for (CArray::ItemIndex i = 0; i < argumentCount; i++)
		// Add argument
		argv[i + 1] = ::strdup(*arguments[i].getUTF8String());
	argv[argumentCount + 1] = nil;

	// Create pipe for the child's standard output and standard error
	int	pipeFileDescriptors[2];
	if (::pipe(pipeFileDescriptors) != 0) {
		// Error
		SError	error = SErrorFromPOSIXerror(errno);
		for (CArray::ItemIndex i = 0; i < (argumentCount + 1); i++)
			// Cleanup
			::free(argv[i]);
		DeleteArray(argv);

		return TVResult<Result>(error);
	}

	// Spawn
	posix_spawn_file_actions_t	fileActions;
	::posix_spawn_file_actions_init(&fileActions);
	::posix_spawn_file_actions_adddup2(&fileActions, pipeFileDescriptors[1], STDOUT_FILENO);
	::posix_spawn_file_actions_adddup2(&fileActions, pipeFileDescriptors[1], STDERR_FILENO);
	::posix_spawn_file_actions_addclose(&fileActions, pipeFileDescriptors[0]);
	::posix_spawn_file_actions_addclose(&fileActions, pipeFileDescriptors[1]);

	pid_t	processID;
	int		spawnResult = ::posix_spawn(&processID, argv[0], &fileActions, nil, argv, environ);

	::posix_spawn_file_actions_destroy(&fileActions);
	::close(pipeFileDescriptors[1]);
	for (CArray::ItemIndex i = 0; i < (argumentCount + 1); i++)
		// Cleanup
		::free(argv[i]);
	DeleteArray(argv);

	if (spawnResult != 0) {
		// Error
		::close(pipeFileDescriptors[0]);

		return TVResult<Result>(SErrorFromPOSIXerror(spawnResult));
	}

	// Read output until the child closes its end or the timeout elapses
	UniversalTime	deadline = SUniversalTime::getCurrent() + timeout;
	CData			outputData;
	bool			timedOut = false;
	UInt8			buffer[4096];
	while (true) {
		// Check time remaining
		UniversalTimeInterval	remaining = deadline - SUniversalTime::getCurrent();
		if (remaining <= 0.0) {
			// Timed out
			timedOut = true;
			break;
		}

		// Wait for output
		struct pollfd	pollFileDescriptor = {pipeFileDescriptors[0], POLLIN, 0};
		int				pollResult = ::poll(&pollFileDescriptor, 1, (int) (remaining * 1000.0) + 1);
		if ((pollResult < 0) && (errno == EINTR))
			// Interrupted, try again
			continue;
		else if (pollResult < 0)
			// Error, treat as end of output
			break;
		else if (pollResult == 0) {
			// Timed out
			timedOut = true;
			break;
		}

		// Read
		ssize_t	byteCount = ::read(pipeFileDescriptors[0], buffer, sizeof(buffer));
		if (byteCount > 0)
			// Collect
			outputData += CData(buffer, (CData::ByteCount) byteCount, false);
		else if ((byteCount < 0) && (errno == EINTR))
			// Interrupted, try again
			continue;
		else
			// End of output or error
			break;
	}
	::close(pipeFileDescriptors[0]);

	// Wait for the child to end, which may lag its output; kill it if the timeout elapses
	int	status = 0;
	while (!timedOut) {
		// Check
		pid_t	waitResult = ::waitpid(processID, &status, WNOHANG);
		if (waitResult == processID)
			// Ended
			break;
		else if ((waitResult < 0) && (errno != EINTR))
			// Error, nothing more to wait for
			break;
		else if (SUniversalTime::getCurrent() >= deadline)
			// Timed out
			timedOut = true;
		else
			// Wait a bit
			CThread::sleepFor(0.01);
	}

	// Check situation
	if (timedOut) {
		// Timed out
		::kill(processID, SIGKILL);
		while ((::waitpid(processID, &status, 0) < 0) && (errno == EINTR)) ;

		return TVResult<Result>(Result(Result::kTerminationTimedOut, outputData));
	} else if (WIFEXITED(status))
		// Normal
		return TVResult<Result>(Result(Result::kTerminationExited, WEXITSTATUS(status), outputData));
	else if (WIFSIGNALED(status))
		// Signaled
		return TVResult<Result>(Result(Result::kTerminationCrashed, WTERMSIG(status), outputData));
	else
		// Other crash
		return TVResult<Result>(Result(Result::kTerminationCrashed, outputData));
}
