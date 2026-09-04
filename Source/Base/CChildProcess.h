//----------------------------------------------------------------------------------------------------------------------
//	CChildProcess.h			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CFile.h"
#include "TimeAndDate.h"
#include "TResult.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CChildProcess

class CChildProcess {
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

	// Methods
	public:
									// Class methods
		static	TVResult<Result>	run(const CFile& executableFile, const TArray<CString>& arguments,
											UniversalTimeInterval timeout);
};
