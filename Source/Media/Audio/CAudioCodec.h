//----------------------------------------------------------------------------------------------------------------------
//	CAudioCodec.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CByteParceller.h"
#include "SAudioFormats.h"
#include "SAudioReadStatus.h"
#include "SMediaPosition.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK:  CAudioCodec

class CAudioCodec {
	// Types
	public:
		class CDecodeInfo {
			// Methods
			public:
						// Lifecycle methods
						CDecodeInfo() {}
				virtual	~CDecodeInfo() {}
		};

		class SDataDecodeInfo : public CDecodeInfo {
			// Methods
			public:
						// Lifecycle methods
						SDataDecodeInfo(SInt64 startOffset, SInt64 size) :
							CDecodeInfo(), mStartOffset(startOffset), mSize(size)
							{}

						// Instance methods
				SInt64	getStartOffset() const
							{ return mStartOffset; }
				SInt64	getSize() const
							{ return mSize; }

			// Properties
			private:
				SInt64	mStartOffset;
				SInt64	mSize;
		};

		class CEncodeSettings {
			// Methods
			public:
				// Lifecycle methods
				CEncodeSettings() : mDummy(false) {}
				CEncodeSettings(const CEncodeSettings& other) : mDummy(other.mDummy) {}

			// Properties
			private:
				bool	mDummy;
		};

		struct SInfo {
			// Types
			typedef	TArray<SAudioProcessingSetup>	(*GetAudioProcessingSetupsProc)(OSType id,
															const SAudioStorageFormat& audioStorageFormat);
			typedef	I<CAudioCodec>					(*InstantiateProc)(OSType id);

													// Lifecycle methods
													SInfo(OSType id, const CString& name,
															GetAudioProcessingSetupsProc getAudioProcessingSetupsProc,
															InstantiateProc instantiateProc) :
														mID(id), mDecodeName(name), mEncodeName(name),
																mGetAudioProcessingSetupsProc(
																		getAudioProcessingSetupsProc),
																mInstantiateProc(instantiateProc)
														{}
													SInfo(OSType id, const CString& name,
															const CEncodeSettings& encodeSettings,
															GetAudioProcessingSetupsProc getAudioProcessingSetupsProc,
															InstantiateProc instantiateProc) :
														mID(id), mDecodeName(name), mEncodeName(name),
																mEncodeSettings(OI<CEncodeSettings>(encodeSettings)),
																mGetAudioProcessingSetupsProc(
																		getAudioProcessingSetupsProc),
																mInstantiateProc(instantiateProc)
														{}
													SInfo(OSType id, const CString& decodeName,
															const CString& encodeName,
															const CEncodeSettings& encodeSettings,
															GetAudioProcessingSetupsProc getAudioProcessingSetupsProc,
															InstantiateProc instantiateProc) :
														mID(id), mDecodeName(decodeName), mEncodeName(encodeName),
																mEncodeSettings(OI<CEncodeSettings>(encodeSettings)),
																mGetAudioProcessingSetupsProc(
																		getAudioProcessingSetupsProc),
																mInstantiateProc(instantiateProc)
														{}
													SInfo(const SInfo& other) :
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
				OI<CEncodeSettings>				mEncodeSettings;
				InstantiateProc					mInstantiateProc;
				GetAudioProcessingSetupsProc	mGetAudioProcessingSetupsProc;
		};

	// Methods
	public:
												// Lifecycle methods
												CAudioCodec() {}
		virtual									~CAudioCodec() {}

												// Instance methods
		virtual	TArray<SAudioProcessingSetup>	getDecodeAudioProcessingSetups(
														const SAudioStorageFormat& storedAudioSampleFormat) const = 0;
		virtual	void							setupForDecode(const SAudioProcessingFormat& audioProcessingFormat,
														CByteParceller& byteParceller,
														const I<CAudioCodec::CDecodeInfo>& decodeInfo) = 0;
		virtual	SAudioReadStatus				decode(const SMediaPosition& mediaPosition, CAudioData& audioData) = 0;

		virtual	TArray<SAudioProcessingSetup>	getEncodeAudioProcessingSetups() const = 0;
		virtual	void							setupForEncode(const SAudioProcessingFormat& audioProcessingFormat) = 0;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDecodeOnlyAudioCodec

class CDecodeOnlyAudioCodec : public CAudioCodec {
	// Methods
	public:
										// Lifecycle methods
										CDecodeOnlyAudioCodec() {}

										// CAudioCodec methods
		TArray<SAudioProcessingSetup>	getEncodeAudioProcessingSetups() const
											{ AssertFailUnimplemented(); return TNArray<SAudioProcessingSetup>(); }
		void							setupForEncode(const SAudioProcessingFormat& audioProcessingFormat)
											{ AssertFailUnimplemented(); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CEncodeOnlyAudioCodec

class CEncodeOnlyAudioCodec : public CAudioCodec {
	// Methods
	public:
										// Lifecycle methods
										CEncodeOnlyAudioCodec() {}

										// CAudioCodec methods
		TArray<SAudioProcessingSetup>	getDecodeAudioProcessingSetups(
												const SAudioStorageFormat& storedAudioSampleFormat) const
											{ AssertFailUnimplemented(); return TNArray<SAudioProcessingSetup>(); }
		void							setupForDecode(const SAudioProcessingFormat& audioProcessingFormat,
												CByteParceller& byteParceller,
												const I<CAudioCodec::CDecodeInfo>& decodeInfo)
											{ AssertFailUnimplemented(); }
		SAudioReadStatus				decode(const SMediaPosition& mediaPosition, CAudioData& audioData)
											{
												AssertFailUnimplemented();

												return SAudioReadStatus(SError::mUnimplemented);
											}
};
