//----------------------------------------------------------------------------------------------------------------------
//	CCodec.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataSource.h"
#include "SMediaPacket.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCodec

class CCodec {
	// EncodeSettings
	public:
		class EncodeSettings {
			// Methods
			public:
				// Lifecycle methods
				EncodeSettings() : mDummy(false) {}
				EncodeSettings(const EncodeSettings& other) : mDummy(other.mDummy) {}

			// Properties
			private:
				bool	mDummy;
		};

	// Methods
	public:
						// Lifecycle methods
		virtual			~CCodec() {}

						// Class methods
		static	SError	unsupportedError(const CString& codecDescriptor);
		static	SError	unsupportedConfigurationError(const CString& codecDescriptor);

	protected:
						// Lifecycle methods
						CCodec() {}

	// Properties
	public:
		static	const	SError	mErrorNoCodec;
};
