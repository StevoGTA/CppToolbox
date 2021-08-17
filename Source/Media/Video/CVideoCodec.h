//----------------------------------------------------------------------------------------------------------------------
//	CVideoCodec.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CCodec.h"
#include "CVideoFrame.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CVideoCodec

class CVideoCodec : public CCodec {
	// Info
	public:
		struct Info {
			// Procs
			typedef	I<CVideoCodec>	(*InstantiateProc)(OSType id);

									// Lifecycle methods
									Info(OSType id, const CString& name, InstantiateProc instantiateProc) :
										mID(id), mDecodeName(name), mInstantiateProc(instantiateProc)
										{}
									Info(const Info& other) :
										mID(other.mID), mDecodeName(other.mDecodeName),
												mInstantiateProc(other.mInstantiateProc)
										{}

									// Instance methods
			OSType					getID() const
										{ return mID; }
			const	CString&		getDecodeName() const
										{ return mDecodeName; }
					I<CVideoCodec>	instantiate() const
										{ return mInstantiateProc(mID); }

			// Properties
			private:
				OSType			mID;
				CString			mDecodeName;
				InstantiateProc	mInstantiateProc;
		};

	// Methods
	public:
										// Lifecycle methods
										~CVideoCodec() {}

										// Instance methods
		virtual	void					setupForDecode(const I<CMediaReader>& mediaReader,
												const I<CCodec::DecodeInfo>& decodeInfo,
												CVideoFrame::Compatibility compatibility) = 0;
		virtual	TIResult<CVideoFrame>	decode() = 0;
		virtual	void					decodeReset() {}

	protected:
										// Lifecycle methods
										CVideoCodec() : CCodec() {}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDecodeOnlyVideoCodec

class CDecodeOnlyVideoCodec : public CVideoCodec {
	// Methods
	public:
		// Lifecycle methods
		CDecodeOnlyVideoCodec() : CVideoCodec() {}
};
