//----------------------------------------------------------------------------------------------------------------------
//	CCodec.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataSource.h"
#include "CMediaReader.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCodec

class CCodec {
	// DecodeInfo
	public:
		class DecodeInfo {
			// Methods
			public:
										// Lifecycle methods
										DecodeInfo() {}
				virtual					~DecodeInfo() {}

										// Instance methods
				virtual	I<CMediaReader>	createMediaReader(const I<CSeekableDataSource>& seekableDataSource) const = 0;
		};

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
				CCodec() {}
		virtual	~CCodec() {}
};
