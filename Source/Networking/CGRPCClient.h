//----------------------------------------------------------------------------------------------------------------------
//	CGRPCClient.h			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "TResult.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGRPCClient

class CGRPCClient {
	// Classes
	private:
		class Internals;

	// Methods
	public:
						// Lifecycle methods
						CGRPCClient(const CString& host, UInt16 port);
						~CGRPCClient();

						// Instance methods
		TVResult<CData>	call(const CString& path, const CData& message);

	// Properties
	private:
		Internals*	mInternals;
};
