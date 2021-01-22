//----------------------------------------------------------------------------------------------------------------------
//	CAudioCodec.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CByteParceller.h"
#include "SAudioFormats.h"
#include "SAudioReadStatus.h"
#include "SMediaPosition.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioCodec

class CAudioCodec {
	// Structs
	public:
		struct Packet {
			// Lifecycle methods
			Packet(UInt32 sampleCount, UInt32 size) : mSampleCount(sampleCount), mSize(size) {}

			// Properties
			UInt32	mSampleCount;
			UInt32	mSize;
		};

		struct PacketLocation {
			// Lifecycle methods
			PacketLocation(Packet packet, SInt64 pos) : mPacket(packet), mPos(pos) {}

			// Properties
			Packet	mPacket;
			SInt64	mPos;
		};

		struct PacketBlock {
			// Lifecycle methods
			PacketBlock(UInt32 count, Packet packet) : mCount(count), mPacket(packet) {}

			// Properties
			UInt32	mCount;
			Packet	mPacket;
		};

	// Classes
	public:
		class DecodeInfo {
			// Methods
			public:
						// Lifecycle methods
						DecodeInfo() {}
				virtual	~DecodeInfo() {}
		};

		class DataDecodeInfo : public DecodeInfo {
			// Methods
			public:
						// Lifecycle methods
						DataDecodeInfo(SInt64 startOffset, SInt64 size) :
							DecodeInfo(), mStartOffset(startOffset), mSize(size)
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

		class PacketsDecodeInfo : public DecodeInfo {
			// Methods
			public:
												// Lifecycle methods
												PacketsDecodeInfo(const TArray<PacketLocation>& packetLocations) :
													DecodeInfo(), mPacketLocations(packetLocations)
													{}

												// Instance methods
				const	TArray<PacketLocation>&	getPacketLocations() const
							{ return mPacketLocations; }

			// Properties
			private:
				TArray<PacketLocation>	mPacketLocations;
		};

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

	// Structs
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
												CAudioCodec() {}
		virtual									~CAudioCodec() {}

												// Instance methods
		virtual	void							setupForDecode(const SAudioProcessingFormat& audioProcessingFormat,
														CByteParceller& byteParceller,
														const I<CAudioCodec::DecodeInfo>& decodeInfo) = 0;
		virtual	SAudioReadStatus				decode(const SMediaPosition& mediaPosition, CAudioData& audioData) = 0;

		virtual	TArray<SAudioProcessingSetup>	getEncodeAudioProcessingSetups() const = 0;
		virtual	void							setupForEncode(const SAudioProcessingFormat& audioProcessingFormat) = 0;

	protected:
												// Class methods
		static	UInt32							getPacketIndex(const SMediaPosition& mediaPosition,
														const SAudioProcessingFormat& audioProcessingFormat,
														const TArray<CAudioCodec::PacketLocation>& packetLocations);
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
		void				setupForDecode(const SAudioProcessingFormat& audioProcessingFormat,
									CByteParceller& byteParceller, const I<CAudioCodec::DecodeInfo>& decodeInfo)
								{ AssertFailUnimplemented(); }
		SAudioReadStatus	decode(const SMediaPosition& mediaPosition, CAudioData& audioData)
								{
									AssertFailUnimplemented();

									return SAudioReadStatus(SError::mUnimplemented);
								}
};
