//----------------------------------------------------------------------------------------------------------------------
//	CMessageChannel.h			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"
#include "TimeAndDate.h"
#include "TResult.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMessageChannel

class CMessageChannel {
	// Types
	public:
#if defined(TARGET_OS_MACOS)
		using	Handle = int;		// File descriptor
#elif defined(TARGET_OS_WINDOWS)
		using	Handle = void*;		// HANDLE
#endif

	// Classes
	private:
		class Internals;

	// Methods
	public:
								// Lifecycle methods
								CMessageChannel(Handle readHandle, Handle writeHandle);
								~CMessageChannel();

								// Instance methods
		OV<SError>				send(const CDictionary& message,
										const OV<UniversalTimeInterval>& timeout = OV<UniversalTimeInterval>());
		TVResult<CDictionary>	receive(const OV<UniversalTimeInterval>& timeout = OV<UniversalTimeInterval>());

		void					close();

	// Properties
	public:
		static	const	SError		mClosedError;
		static	const	SError		mTimedOutError;
		static	const	SError		mInvalidMessageError;

	private:
						Internals*	mInternals;
};
