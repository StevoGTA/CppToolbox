//----------------------------------------------------------------------------------------------------------------------
//	CCodec.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataSource.h"
#include "SMediaPacket.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCodec

class CCodec {
	// DecodeInfo
	public:
		class DecodeInfo {
			// Methods
			public:
						// Lifecycle methods
				virtual	~DecodeInfo() {}

			protected:
						// Lifecycle methods
						DecodeInfo() {}
		};

	// MediaPacketSourceDecodeInfo
	public:
		class MediaPacketSourceDecodeInfo : public DecodeInfo {
			// Methods
			public:
										// Lifecycle methods
										MediaPacketSourceDecodeInfo(const I<CMediaPacketSource>& mediaPacketSource) :
											DecodeInfo(), mMediaPacketSource(mediaPacketSource)
											{}

										// Instance methods
				I<CMediaPacketSource>	getMediaPacketSource() const
											{ return mMediaPacketSource; }

			// Properties
			private:
				I<CMediaPacketSource>	mMediaPacketSource;
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
		virtual	~CCodec() {}

	protected:
				// Lifecycle methods
				CCodec() {}

	// Properties
	public:
		static	const	SError	mErrorUnsupported;
};
