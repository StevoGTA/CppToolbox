//----------------------------------------------------------------------------------------------------------------------
//	CProcess-Apple.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CProcess.h"

#include "CThread.h"
#include "SError-POSIX.h"
#include "TLockingValue.h"

#include <cstdlib>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <spawn.h>
#include <sys/event.h>
#include <sys/wait.h>
#include <unistd.h>

extern	char**	environ;

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	const	char*	sMessageChannelEnvironmentVariableName = "CProcessMessageChannel";
static	const	char*	sMessageChannelEnvironmentEntry = "CProcessMessageChannel=1";
static	const	int		sMessageChannelReadFileDescriptor = 3;
static	const	int		sMessageChannelWriteFileDescriptor = 4;

static	CString	sErrorDomain(OSSTR("CProcess"));
static	SError	sNoMessageChannelError(sErrorDomain, 1, CString(OSSTR("No message channel")));

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local procs

//----------------------------------------------------------------------------------------------------------------------
static void sWatchParent(CThread& thread, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if the parent is still running
	pid_t	parentProcessID = ::getppid();
	if (parentProcessID != 1) {
		// Wait for the parent to end
		struct kevent	change;
		EV_SET(&change, parentProcessID, EVFILT_PROC, EV_ADD | EV_ONESHOT, NOTE_EXIT, 0, nil);

		int				kqueueFileDescriptor = ::kqueue();
		struct kevent	event;
		while ((::kevent(kqueueFileDescriptor, &change, 1, &event, 1, nil) < 0) && (errno == EINTR)) ;
	}

	// Parent has ended
	::_exit(1);
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
	if (::getenv(sMessageChannelEnvironmentVariableName) == nil)
		// No
		return TIResult<CMessageChannel>(sNoMessageChannelError);

	// Keep the message channel from any child of this process
	::unsetenv(sMessageChannelEnvironmentVariableName);
	::fcntl(sMessageChannelReadFileDescriptor, F_SETFD, FD_CLOEXEC);
	::fcntl(sMessageChannelWriteFileDescriptor, F_SETFD, FD_CLOEXEC);

	// End this process when the parent ends (the thread lives as long as this process)
	new CThread(sWatchParent, nil, CString(OSSTR("CProcess parent watch")), CThread::kOptionsAutoStart);

	return TIResult<CMessageChannel>(
			OI<CMessageChannel>(
					new CMessageChannel(sMessageChannelReadFileDescriptor, sMessageChannelWriteFileDescriptor)));
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
								::close(mOutputReadHandle);
							}

				void	kill()
							{
								// Kill
								int	status;
								::kill(mProcessRef, SIGKILL);
								while ((::waitpid(mProcessRef, &status, 0) < 0) && (errno == EINTR)) ;

								mHasEnded.set();
							}

		static	void	readOutput(CThread& thread, void* userData)
							{
								// Setup
								Internals&	internals = *((Internals*) userData);
								UInt8		buffer[4096];

								// Read until the output closes, or the child has ended and nothing remains
								while (true) {
									// Wait for output
									struct pollfd	pollFileDescriptor = {internals.mOutputReadHandle, POLLIN, 0};
									int				pollResult = ::poll(&pollFileDescriptor, 1, 100);
									if (pollResult > 0) {
										// Read
										ssize_t	byteCount =
														::read(internals.mOutputReadHandle, buffer, sizeof(buffer));
										if (byteCount > 0)
											// Collect
											internals.mOutputData += CData(buffer, (CData::ByteCount) byteCount, false);
										else if ((byteCount == 0) || (errno != EINTR))
											// End of output or error
											break;
									} else if ((pollResult == 0) && *internals.mHasEnded)
										// Ended with nothing remaining
										break;
									else if ((pollResult < 0) && (errno != EINTR))
										// Error
										break;
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
	UniversalTime	deadline = SUniversalTime::getCurrent() + timeout;
	int				status = 0;
	bool			timedOut = false;
	while (true) {
		// Check
		pid_t	waitResult = ::waitpid(mInternals->mProcessRef, &status, WNOHANG);
		if ((waitResult == mInternals->mProcessRef) || ((waitResult < 0) && (errno != EINTR))) {
			// Ended, or nothing to wait for
			mInternals->mHasEnded.set();
			break;
		} else if (SUniversalTime::getCurrent() >= deadline) {
			// Timed out
			mInternals->kill();
			timedOut = true;
			break;
		} else
			// Wait a bit
			CThread::sleepFor(0.01);
	}

	// Wait for output to finish
	mInternals->mOutputThread.waitUntilFinished();

	// Check situation
	if (timedOut)
		// Timed out
		return Result(Result::kTerminationTimedOut, mInternals->mOutputData);
	else if (WIFEXITED(status))
		// Normal
		return Result(Result::kTerminationExited, WEXITSTATUS(status), mInternals->mOutputData);
	else if (WIFSIGNALED(status))
		// Signaled
		return Result(Result::kTerminationCrashed, WTERMSIG(status), mInternals->mOutputData);
	else
		// Other crash
		return Result(Result::kTerminationCrashed, mInternals->mOutputData);
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TIResult<CChildProcess> CChildProcess::launch(const CFile& executableFile, const TArray<CString>& arguments)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create pipes for the child's output, to the child, and from the child; keep them from any other child
	int	fileDescriptors[6];
	for (int i = 0; i < 6; i += 2) {
		// Create pipe
		if (::pipe(fileDescriptors + i) != 0) {
			// Error
			SError	error = SErrorFromPOSIXerror(errno);
			for (int j = 0; j < i; j++)
				// Cleanup
				::close(fileDescriptors[j]);

			return TIResult<CChildProcess>(error);
		}

		::fcntl(fileDescriptors[i], F_SETFD, FD_CLOEXEC);
		::fcntl(fileDescriptors[i + 1], F_SETFD, FD_CLOEXEC);
	}
	int	outputReadFileDescriptor = fileDescriptors[0];
	int	outputWriteFileDescriptor = fileDescriptors[1];
	int	messageChannelWriteFileDescriptor = fileDescriptors[3];
	int	messageChannelReadFileDescriptor = fileDescriptors[4];

	// Move the child's message channel ends above the descriptors they become in the child, so placing one cannot
	//	overwrite the other
	int	childMessageChannelReadFileDescriptor =
				::fcntl(fileDescriptors[2], F_DUPFD_CLOEXEC, sMessageChannelWriteFileDescriptor + 1);
	int	childMessageChannelWriteFileDescriptor =
				::fcntl(fileDescriptors[5], F_DUPFD_CLOEXEC, sMessageChannelWriteFileDescriptor + 1);
	::close(fileDescriptors[2]);
	::close(fileDescriptors[5]);

	// Compose argv
	CArray::ItemCount	argumentCount = arguments.getCount();
	char**				argv = new char*[argumentCount + 2];
	argv[0] = ::strdup(*executableFile.getFilesystemPath().getString().getUTF8String());
	for (CArray::ItemIndex i = 0; i < argumentCount; i++)
		// Add argument
		argv[i + 1] = ::strdup(*arguments[i].getUTF8String());
	argv[argumentCount + 1] = nil;

	// Compose environment, marking the message channel for the child
	CArray::ItemCount	environmentCount = 0;
	while (environ[environmentCount] != nil)
		// Count
		environmentCount++;
	char**	envp = new char*[environmentCount + 2];
	for (CArray::ItemIndex i = 0; i < environmentCount; i++)
		// Add entry
		envp[i] = environ[i];
	envp[environmentCount] = (char*) sMessageChannelEnvironmentEntry;
	envp[environmentCount + 1] = nil;

	// Spawn, passing only standard input, the output, and the message channel
	posix_spawnattr_t	attributes;
	::posix_spawnattr_init(&attributes);
	::posix_spawnattr_setflags(&attributes, POSIX_SPAWN_CLOEXEC_DEFAULT);

	posix_spawn_file_actions_t	fileActions;
	::posix_spawn_file_actions_init(&fileActions);
	::posix_spawn_file_actions_addopen(&fileActions, STDIN_FILENO, "/dev/null", O_RDONLY, 0);
	::posix_spawn_file_actions_adddup2(&fileActions, outputWriteFileDescriptor, STDOUT_FILENO);
	::posix_spawn_file_actions_adddup2(&fileActions, outputWriteFileDescriptor, STDERR_FILENO);
	::posix_spawn_file_actions_adddup2(&fileActions, childMessageChannelReadFileDescriptor,
			sMessageChannelReadFileDescriptor);
	::posix_spawn_file_actions_adddup2(&fileActions, childMessageChannelWriteFileDescriptor,
			sMessageChannelWriteFileDescriptor);

	pid_t	processID;
	int		spawnResult = ::posix_spawn(&processID, argv[0], &fileActions, &attributes, argv, envp);

	// Cleanup
	::posix_spawn_file_actions_destroy(&fileActions);
	::posix_spawnattr_destroy(&attributes);
	::close(outputWriteFileDescriptor);
	::close(childMessageChannelReadFileDescriptor);
	::close(childMessageChannelWriteFileDescriptor);
	for (CArray::ItemIndex i = 0; i < (argumentCount + 1); i++)
		// Cleanup
		::free(argv[i]);
	DeleteArray(argv);
	DeleteArray(envp);

	// Check result
	if (spawnResult != 0) {
		// Error
		::close(outputReadFileDescriptor);
		::close(messageChannelReadFileDescriptor);
		::close(messageChannelWriteFileDescriptor);

		return TIResult<CChildProcess>(SErrorFromPOSIXerror(spawnResult));
	}

	return TIResult<CChildProcess>(
			OI<CChildProcess>(
					new CChildProcess(processID, outputReadFileDescriptor, messageChannelReadFileDescriptor,
							messageChannelWriteFileDescriptor)));
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
