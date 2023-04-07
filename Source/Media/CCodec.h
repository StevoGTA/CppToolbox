//----------------------------------------------------------------------------------------------------------------------
//	CCodec.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataSource.h"
#include "SMediaPacket.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCodec

class CCodec {
	// Info
	public:
		struct Info {
								// Lifecycle methods
								Info(OSType id, const CString& name) : mID(id), mDecodeName(name), mEncodeName(name) {}
								Info(OSType id, const CString& decodeName, const CString& encodeName) :
									mID(id), mDecodeName(decodeName), mEncodeName(encodeName)
									{}
								Info(const Info& other) :
									mID(other.mID), mDecodeName(other.mDecodeName), mEncodeName(other.mEncodeName)
									{}

								// Instance methods
					OSType		getID() const
									{ return mID; }
			const	CString&	getDecodeName() const
									{ return mDecodeName; }
			const	CString&	getEncodeName() const
									{ return mEncodeName; }

			// Properties
			private:
				OSType	mID;
				CString	mDecodeName;
				CString	mEncodeName;
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
