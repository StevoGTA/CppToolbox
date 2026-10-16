//----------------------------------------------------------------------------------------------------------------------
//	CProcess.h				©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CFile.h"
#include "CMessageChannel.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CProcess

class CProcess {
	// Methods
	public:
											// Class methods
		static	TIResult<CMessageChannel>	getMessageChannel();
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CChildProcess

class CChildProcess {
	// Types
	public:
#if defined(TARGET_OS_MACOS)
		using	ProcessRef = int;	// pid_t
#elif defined(TARGET_OS_WINDOWS)
		using	ProcessRef = void*;	// HANDLE
#endif

	// Result
	public:
		struct Result {
			// Termination
			public:
				enum Termination {
					kTerminationExited,		// Exited normally; code is the exit code
					kTerminationCrashed,	// Ended by a signal or an exception; code is the signal or exception code
					kTerminationTimedOut,	// Killed after the timeout elapsed; code is 0
				};

			// Methods
			public:
									// Lifecycle methods
									Result(Termination termination, SInt32 code, const CData& outputData) :
										mTermination(termination), mCode(code), mOutputData(outputData)
										{}
									Result(Termination termination, const CData& outputData) :
										mTermination(termination), mCode(0), mOutputData(outputData)
										{}
									Result(const Result& other) :
										mTermination(other.mTermination), mCode(other.mCode),
												mOutputData(other.mOutputData)
										{}

									// Instance methods
						Termination	getTermination() const
										{ return mTermination; }

						SInt32		getCode() const
										{ return mCode; }

				const	CData&		getOutputData() const
										{ return mOutputData; }
						CString		getOutputAsString() const
										{ return CString(mOutputData, CString::kEncodingUTF8); }

			// Properties
			private:
				Termination	mTermination;
				SInt32		mCode;
				CData		mOutputData;
		};

	// Classes
	private:
		class Internals;

	// Methods
	public:
											// Lifecycle methods
											~CChildProcess();

											// Instance methods
				CMessageChannel&			getMessageChannel();

				Result						finish(UniversalTimeInterval timeout);

											// Class methods
		static	TIResult<CChildProcess>		launch(const CFile& executableFile, const TArray<CString>& arguments);
		static	TVResult<Result>			run(const CFile& executableFile, const TArray<CString>& arguments,
													UniversalTimeInterval timeout);

	private:
											// Lifecycle methods
											CChildProcess(ProcessRef processRef,
													CMessageChannel::Handle outputReadHandle,
													CMessageChannel::Handle messageChannelReadHandle,
													CMessageChannel::Handle messageChannelWriteHandle);

	// Properties
	private:
		Internals*	mInternals;
};
