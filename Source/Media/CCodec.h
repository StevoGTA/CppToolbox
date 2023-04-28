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
					Info(OSType id, const CString& nameLocalizationGroup, const CString& nameLocalizationKey) :
						mID(id), mNameLocalizationGroup(nameLocalizationGroup),
								mDecodeNameLocalizationKey(nameLocalizationKey),
								mEncodeNameLocalizationKey(nameLocalizationKey)
						{}
					Info(OSType id, const CString& nameLocalizationGroup, const CString& decodeNameLocalizationKey,
							const CString& encodeNameLocalizationKey) :
						mID(id), mNameLocalizationGroup(nameLocalizationGroup),
								mDecodeNameLocalizationKey(decodeNameLocalizationKey),
								mEncodeNameLocalizationKey(encodeNameLocalizationKey)
						{}
					Info(const Info& other) :
						mID(other.mID), mDecodeName(other.mDecodeName), mEncodeName(other.mEncodeName),
								mNameLocalizationGroup(other.mNameLocalizationGroup),
								mDecodeNameLocalizationKey(other.mDecodeNameLocalizationKey),
								mEncodeNameLocalizationKey(other.mEncodeNameLocalizationKey)
						{}

					// Instance methods
			OSType	getID() const
						{ return mID; }
			CString	getDecodeName() const
						{ return mDecodeName.hasValue() ?
								*mDecodeName : CString(*mNameLocalizationGroup, *mDecodeNameLocalizationKey); }
			CString	getEncodeName() const
						{ return CString(*mNameLocalizationGroup, *mEncodeNameLocalizationKey); }

			// Properties
			private:
				OSType		mID;
				OV<CString>	mDecodeName;
				OV<CString>	mEncodeName;
				OV<CString>	mNameLocalizationGroup;
				OV<CString>	mDecodeNameLocalizationKey;
				OV<CString>	mEncodeNameLocalizationKey;
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
