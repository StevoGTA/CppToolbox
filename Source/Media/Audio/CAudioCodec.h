//----------------------------------------------------------------------------------------------------------------------
//	CAudioCodec.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioFrames.h"
#include "CCodec.h"
#include "CDataSource.h"
#include "SAudioFormats.h"
#include "SAudioSourceStatus.h"
#include "SMediaPosition.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioCodec

class CAudioCodec : public CCodec {
	// Types
	public:
		typedef	UInt64					(*ComposeFrameCountProc)(const SAudioStorageFormat& audioStorageFormat,
												UInt64 byteCount);
		typedef	I<CCodec::DecodeInfo>	(*ComposeDecodeInfoProc)(const SAudioStorageFormat& audioStorageFormat,
												UInt64 startByteOffset, UInt64 byteCount);

	// Info
	public:
		struct Info {
			// Procs
			typedef	TArray<SAudioProcessingSetup>	(*GetAudioProcessingSetupsProc)(OSType id,
															const SAudioStorageFormat& audioStorageFormat);
			typedef	I<CAudioCodec>					(*InstantiateProc)(OSType id);

													// Lifecycle methods
													Info(OSType id, const CString& name,
															GetAudioProcessingSetupsProc getAudioProcessingSetupsProc,
															InstantiateProc instantiateProc) :
														mID(id), mDecodeName(name), mEncodeName(name),
																mGetAudioProcessingSetupsProc(
																		getAudioProcessingSetupsProc),
																mInstantiateProc(instantiateProc)
														{}
													Info(OSType id, const CString& name,
															const EncodeSettings& encodeSettings,
															GetAudioProcessingSetupsProc getAudioProcessingSetupsProc,
															InstantiateProc instantiateProc) :
														mID(id), mDecodeName(name), mEncodeName(name),
																mEncodeSettings(OI<EncodeSettings>(encodeSettings)),
																mGetAudioProcessingSetupsProc(
																		getAudioProcessingSetupsProc),
																mInstantiateProc(instantiateProc)
														{}
													Info(OSType id, const CString& decodeName,
															const CString& encodeName,
															const EncodeSettings& encodeSettings,
															GetAudioProcessingSetupsProc getAudioProcessingSetupsProc,
															InstantiateProc instantiateProc) :
														mID(id), mDecodeName(decodeName), mEncodeName(encodeName),
																mEncodeSettings(OI<EncodeSettings>(encodeSettings)),
																mGetAudioProcessingSetupsProc(
																		getAudioProcessingSetupsProc),
																mInstantiateProc(instantiateProc)
														{}
													Info(const Info& other) :
														mID(other.mID), mDecodeName(other.mDecodeName),
																mEncodeName(other.mEncodeName),
																mEncodeSettings(other.mEncodeSettings),
																mGetAudioProcessingSetupsProc(
																		other.mGetAudioProcessingSetupsProc),
																mInstantiateProc(other.mInstantiateProc)
														{}

													// Instance methods
			OSType									getID() const
														{ return mID; }
			const	CString&						getDecodeName() const
														{ return mDecodeName; }
			const	CString&						getEncodeName() const
														{ return mEncodeName; }
					I<CAudioCodec>					instantiate() const
														{ return mInstantiateProc(mID); }
					TArray<SAudioProcessingSetup>	getAudioProcessingSetups(
															const SAudioStorageFormat& audioStorageFormat) const
														{ return mGetAudioProcessingSetupsProc(mID,
																audioStorageFormat); }

			// Properties
			private:
				OSType							mID;
				CString							mDecodeName;
				CString							mEncodeName;
				OI<EncodeSettings>				mEncodeSettings;
				InstantiateProc					mInstantiateProc;
				GetAudioProcessingSetupsProc	mGetAudioProcessingSetupsProc;
		};

	// Methods
	public:
												// Lifecycle methods
												~CAudioCodec() {}

												// Instance methods
		virtual	void							setupForDecode(const SAudioProcessingFormat& audioProcessingFormat,
														const I<CMediaReader>& mediaReader,
														const I<CCodec::DecodeInfo>& decodeInfo) = 0;
		virtual	OI<SError>						decode(CAudioFrames& audioFrames) = 0;
		virtual	void							decodeReset() {}

		virtual	TArray<SAudioProcessingSetup>	getEncodeAudioProcessingSetups() const = 0;
		virtual	void							setupForEncode(const SAudioProcessingFormat& audioProcessingFormat) = 0;

	protected:
												// Lifecycle methods
												CAudioCodec() : CCodec() {}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDecodeOnlyAudioCodec

class CDecodeOnlyAudioCodec : public CAudioCodec {
	// Methods
	public:
										// CAudioCodec methods
		TArray<SAudioProcessingSetup>	getEncodeAudioProcessingSetups() const
											{ AssertFailUnimplemented(); return TNArray<SAudioProcessingSetup>(); }
		void							setupForEncode(const SAudioProcessingFormat& audioProcessingFormat)
											{ AssertFailUnimplemented(); }

	protected:
										// Lifecycle methods
										CDecodeOnlyAudioCodec() : CAudioCodec() {}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CEncodeOnlyAudioCodec

class CEncodeOnlyAudioCodec : public CAudioCodec {
	// Methods
	public:
					// CAudioCodec methods
		void		setupForDecode(const SAudioProcessingFormat& audioProcessingFormat,
							const I<CMediaReader>& mediaReader, const I<CCodec::DecodeInfo>& decodeInfo)
						{ AssertFailUnimplemented(); }
		OI<SError>	decode(CAudioFrames& audioFrames)
						{ AssertFailUnimplemented(); return OI<SError>(SError::mUnimplemented); }

	protected:
							// Lifecycle methods
							CEncodeOnlyAudioCodec() : CAudioCodec() {}
};
