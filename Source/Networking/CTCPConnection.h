//----------------------------------------------------------------------------------------------------------------------
//	CTCPConnection.h			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "TResult.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CTCPConnection

class CTCPConnection {
	// Classes
	private:
		class Internals;

	// Methods
	public:
						// Lifecycle methods
						CTCPConnection(const CString& host, UInt16 port);
						~CTCPConnection();

						// Instance methods
		OV<SError>		connect();

		TVResult<CData>	read(UInt32 maximumByteCount);
		OV<SError>		write(const CData& data);

		void			close();

	// Properties
	private:
		Internals*	mInternals;
};
