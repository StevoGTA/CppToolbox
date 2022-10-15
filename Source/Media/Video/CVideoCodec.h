//----------------------------------------------------------------------------------------------------------------------
//	CVideoCodec.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CCodec.h"
#include "SVideoFormats.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CVideoCodec

class CVideoCodec : public CCodec {
	// Info
	public:
		struct Info {
								// Lifecycle methods
								Info(OSType id, const CString& name) : mID(id), mDecodeName(name) {}
								Info(const Info& other) : mID(other.mID), mDecodeName(other.mDecodeName) {}

								// Instance methods
					OSType		getID() const
									{ return mID; }
			const	CString&	getDecodeName() const
									{ return mDecodeName; }

			// Properties
			private:
				OSType	mID;
				CString	mDecodeName;
		};

	// Methods
	protected:
		// Lifecycle methods
		CVideoCodec() : CCodec() {}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDecodeVideoCodec

class CDecodeVideoCodec : public CVideoCodec {
	// Methods
	public:
										// Instance methods
		virtual	OV<SError>				setup(const SVideoProcessingFormat& videoProcessingFormat) = 0;
		virtual	void					seek(UniversalTimeInterval timeInterval) = 0;
		virtual	TIResult<CVideoFrame>	decode() = 0;

	protected:
										// Lifecycle methods
										CDecodeVideoCodec() : CVideoCodec() {}
};
