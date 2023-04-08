//----------------------------------------------------------------------------------------------------------------------
//	SAudioFormats.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: EAudioChannelMap
// Speaker designations
//		L - left
//		R - right
//		C - center
//		Ls - left surround (back left)
//		Rs - right surround (back right)
//		Cs - center surround
//		Rls - rear left surround
//		Rrs - rear right surround
//		Lw - left wide
//		Rw - right wide
//		Lsd - left surround direct (side left)
//		Rsd - right surround direct (side right)
//		Lc - left center
//		Rc - right center
//		Ts - top surround
//		Vhl - vertical height left (top front left)
//		Vhc - vertical height center (top front center)
//		Vhr - vertical height right (top front right)
//		Lt - left matrix total. for matrix encoded stereo.
//		Rt - right matrix total. for matrix encoded stereo.

// EAudioChannelMap is 0xoocc where oo is the option variant and cc is the number of channels.

enum EAudioChannelMap : UInt16 {
	// 1 Channel
	kAudioChannelMap_1_0			= 0x0001,	// C

	// 2 Channels
	kAudioChannelMap_2_0_Option1	= 0x0002,	// L R
	kAudioChannelMap_1_1_Option1	= 0x0102,	// C LFE
	kAudioChannelMap_2_0_Option2	= 0x0202,	// Ls Rs
	kAudioChannelMap_2_0_Option3	= 0x0302,	// Lt Rt (Matrix encoded stereo stream)
//	kAudioChannelMap_2_0_Option4	= 0x0402,	// Mide/Side
//	kAudioChannelMap_2_0_Option5	= 0x0502,	// Coincident Mic Pair (often 2 figure 8s)
//	kAudioChannelMap_2_0_Option6	= 0x0602,	// Binaural Stereo (L R)

	// 3 Channels
	kAudioChannelMap_3_0_Unknown	= 0x0003,	// Unknown
	kAudioChannelMap_2_1_Option1	= 0x0103,	// L R LFE
	kAudioChannelMap_3_0_Option1	= 0x0203,	// L R C
	kAudioChannelMap_3_0_Option2	= 0x0303,	// C L R
	kAudioChannelMap_3_0_Option3	= 0x0403,	// L R Cs
	kAudioChannelMap_3_0_Option4	= 0x0503,	// L C R

	// 4 Channels
	kAudioChannelMap_4_0_Unknown	= 0x0004,	// Unknown
	kAudioChannelMap_3_1_Option1	= 0x0104,	// L R LFE Cs
	kAudioChannelMap_3_1_Option2	= 0x0204,	// L R C LFE
	kAudioChannelMap_3_1_Option3	= 0x0304,	// L C R LFE
	kAudioChannelMap_3_1_Option4	= 0x0404,	// L R Cs LFE
	kAudioChannelMap_4_0_Option1	= 0x0504,	// L R Ls Rs (Quadraphonic)
	kAudioChannelMap_4_0_Option2	= 0x0604,	// L R C Cs
	kAudioChannelMap_4_0_Option3	= 0x0704,	// C L R Cs
	kAudioChannelMap_4_0_Option4	= 0x0804,	// L C R Cs
//	kAudioChannelMap_4_0_Option5	= 0x0904,	// Ambisonic B (W X Y Z)

	// 5 Channels
	kAudioChannelMap_5_0_Unknown	= 0x0005,	// Unknown
	kAudioChannelMap_4_1_Option1	= 0x0105,	// L R LFE Ls Rs
	kAudioChannelMap_4_1_Option2	= 0x0205,	// L R C LFE Cs
	kAudioChannelMap_4_1_Option3	= 0x0305,	// L R Ls Rs LFE
	kAudioChannelMap_4_1_Option4	= 0x0405,	// L C R Cs LFE
	kAudioChannelMap_5_0_Option1	= 0x0505,	// L R C Ls Rs
	kAudioChannelMap_5_0_Option2	= 0x0605,	// L R Ls Rs C (Pentagonal)
	kAudioChannelMap_5_0_Option3	= 0x0705,	// L C R Ls Rs
	kAudioChannelMap_5_0_Option4	= 0x0805,	// C L R Ls Rs

	// 6 Channels
	kAudioChannelMap_6_0_Unknown	= 0x0006,	// Unknown
	kAudioChannelMap_5_1_Option1	= 0x0106,	// L R C LFE Ls Rs
	kAudioChannelMap_5_1_Option2	= 0x0206,	// L R Ls Rs C LFE
	kAudioChannelMap_5_1_Option3	= 0x0306,	// L C R Ls Rs LFE
	kAudioChannelMap_5_1_Option4	= 0x0406,	// C L R Ls Rs LFE
	kAudioChannelMap_6_0_Option1	= 0x0506,	// L R Ls Rs C Cs (Hexagonal)
	kAudioChannelMap_6_0_Option2	= 0x0606,	// C L R Ls Rs Cs

	// 7 Channels
	kAudioChannelMap_7_0_Unknown	= 0x0007,	// Unknown
	kAudioChannelMap_6_1_Option1	= 0x0107,	// L R C LFE Ls Rs Cs
	kAudioChannelMap_6_1_Option2	= 0x0207,	// C L R Ls Rs Cs LFE
	kAudioChannelMap_6_1_Option3	= 0x0307,	// L C R Ls Cs Rs LFE
	kAudioChannelMap_7_0_Option1	= 0x0407,	// L R Ls Rs C Rls Rrs
	kAudioChannelMap_7_0_Option2	= 0x0507,	// C L R Ls Rs Rls Rrs
	kAudioChannelMap_7_0_Option3	= 0x0607,	// L R Ls Rs C Lc Rc

	// 8 Channels
	kAudioChannelMap_8_0_Unknown	= 0x0008,	// Unknown
	kAudioChannelMap_7_1_Option1	= 0x0108,	// L R C LFE Ls Rs Lc Rc
	kAudioChannelMap_7_1_Option2	= 0x0208,	// C Lc Rc L R Ls Rs LFE (doc: IS-13818-7 MPEG2-AAC Table 3.1)
	kAudioChannelMap_7_1_Option3	= 0x0308,	// L R C LFE Ls Rs Rls Rrs
	kAudioChannelMap_7_1_Option4	= 0x0408,	// L R Ls Rs C LFE Lc Rc (Emagic Default 7.1)
	kAudioChannelMap_7_1_Option5	= 0x0508,	// L R C LFE Ls Rs Lt Rt (SMPTE DTV)
	kAudioChannelMap_7_1_Option6	= 0x0608,	// L Lc C Rc R Ls Rs LFE
	kAudioChannelMap_8_0_Option1	= 0x0708,	// L R Ls Rs C Cs Lw Rw (Octagonal)
	kAudioChannelMap_8_0_Option2	= 0x0808,	// C L R Ls Rs Rls Rrs Cs
//	kAudioChannelMap_8_0_Option2	= 0x0908,	// L R Ls Rs topLeft topRight topRearLeft topRearRight (Cube)
	kAudioChannelMap_7_1_Option7	= 0x0A08,	// C L R Ls Rs LFE Lt Rt (From Fotokem)

	// 9 Channels
	kAudioChannelMap_9_0_Unknown	= 0x0009,	// Unknown

	// 10 Channels
	kAudioChannelMap_10_0_Unknown	= 0x000A,	// Unknown
};

#define AUDIOCHANNELMAP_FORUNKNOWN(CHANNELS)		(EAudioChannelMap) CHANNELS
#define AUDIOCHANNELMAP_ISUNKNOWN(CHANNELMAP)		((CHANNELMAP & 0xFF00) == 0x0000)
#define AUDIOCHANNELMAP_CHANNELCOUNT(CHANNELMAP)	(CHANNELMAP & 0x00FF)

extern	const	CString	eAudioChannelMapGetDescription(EAudioChannelMap audioChannelMap);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SAudioStorageFormat

struct SAudioStorageFormat {
						// Lifecycle methods
						SAudioStorageFormat(OSType codecID, UInt8 bits, Float32 sampleRate,
								EAudioChannelMap audioChannelMap) :
							mCodecID(codecID), mBits(OV<UInt8>(bits)), mSampleRate(sampleRate),
									mAudioChannelMap(audioChannelMap)
							{}
						SAudioStorageFormat(OSType codecID, OV<UInt8> bits, Float32 sampleRate,
								EAudioChannelMap audioChannelMap) :
							mCodecID(codecID), mBits(bits), mSampleRate(sampleRate),
									mAudioChannelMap(audioChannelMap)
							{}
						SAudioStorageFormat(OSType codecID, Float32 sampleRate, EAudioChannelMap audioChannelMap) :
							mCodecID(codecID), mSampleRate(sampleRate), mAudioChannelMap(audioChannelMap)
							{}
						SAudioStorageFormat(const SAudioStorageFormat& other) :
							mCodecID(other.mCodecID), mBits(other.mBits), mSampleRate(other.mSampleRate),
									mAudioChannelMap(other.mAudioChannelMap)
							{}
						SAudioStorageFormat(const SAudioStorageFormat& other, EAudioChannelMap audioChannelMap) :
							mCodecID(other.mCodecID), mBits(other.mBits), mSampleRate(other.mSampleRate),
									mAudioChannelMap(audioChannelMap)
							{}

						// Instance methods
	OSType				getCodecID() const
							{ return mCodecID; }
	OV<UInt8>			getBits() const
							{ return mBits; }
	Float32				getSampleRate() const
							{ return mSampleRate; }
	EAudioChannelMap	getAudioChannelMap() const
							{ return mAudioChannelMap; }
	UInt8				getChannels() const
							{ return AUDIOCHANNELMAP_CHANNELCOUNT(mAudioChannelMap); }
	CString				getDescription() const
							{
								// Compose description
								CString	description;

								if (mBits.hasValue())
									description += CString(*mBits) + CString(OSSTR(", "));
								else
									description += CString(OSSTR("n/a, "));

								description += CString(mSampleRate, 0, 0) + CString(OSSTR("Hz, "));
								description +=
										CString(AUDIOCHANNELMAP_CHANNELCOUNT(mAudioChannelMap)) + CString(OSSTR(" (")) +
												eAudioChannelMapGetDescription(mAudioChannelMap) + CString(OSSTR(")"));

								return description;
							}

	// Properties
	private:
		OSType				mCodecID;			// All stored audio must be in some format
		OV<UInt8>			mBits;				// Some stored audio does not have an inherent bits
		Float32				mSampleRate;		// All stored audio has a sample rate
		EAudioChannelMap	mAudioChannelMap;	// All stored audio has a channel map
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SAudioProcessingFormat

struct SAudioProcessingFormat {
	// SampleType
	enum SampleType {
		kSampleTypeFloat,
		kSampleTypeSignedInteger,
	};

	// Endian
	enum Endian {
		kEndianBig,
		kEndianLittle,
#if TARGET_RT_BIG_ENDIAN
		kEndianNative = kEndianBig,
#else
		kEndianNative = kEndianLittle,
#endif
	};

	// Interleaved
	enum Interleaved {
		kInterleaved,
		kNonInterleaved,
	};

							// Lifecycle methods
							SAudioProcessingFormat(UInt8 bits, Float32 sampleRate, EAudioChannelMap audioChannelMap,
									SampleType sampleType = kSampleTypeSignedInteger, Endian endian = kEndianNative,
									Interleaved interleaved = kInterleaved) :
								mBits(bits), mSampleRate(sampleRate), mAudioChannelMap(audioChannelMap),
										mSampleType(sampleType), mEndian(endian),
										mInterleaved(
												(audioChannelMap == kAudioChannelMap_1_0) ? kInterleaved : interleaved)
								{}
							SAudioProcessingFormat(const SAudioProcessingFormat& other) :
								mBits(other.mBits), mSampleRate(other.mSampleRate),
										mAudioChannelMap(other.mAudioChannelMap), mSampleType(other.mSampleType),
										mEndian(other.mEndian), mInterleaved(other.mInterleaved)
								{}

							// Instance methods
	UInt8					getBits() const
								{ return mBits; }
	Float32					getSampleRate() const
								{ return mSampleRate; }
	EAudioChannelMap		getAudioChannelMap() const
								{ return mAudioChannelMap; }
	UInt8					getChannels() const
								{ return AUDIOCHANNELMAP_CHANNELCOUNT(mAudioChannelMap); }
	bool					getIsFloat() const
								{ return mSampleType == kSampleTypeFloat; }
	bool					getIsSignedInteger() const
								{ return mSampleType == kSampleTypeSignedInteger; }
	SampleType				getSampleType() const
								{ return mSampleType; }
	bool					getIsBigEndian() const
								{ return mEndian == kEndianBig; }
	Endian					getEndian() const
								{ return mEndian; }
	bool					getIsInterleaved() const
								{ return mInterleaved == kInterleaved; }
	Interleaved				getInterleaved() const
								{ return mInterleaved; }
	UInt32					getBytesPerSample() const
								{ return mBits / 8; }
	UInt32					getBytesPerFrame() const
								{ return mBits / 8 * AUDIOCHANNELMAP_CHANNELCOUNT(mAudioChannelMap); }
	CString					getDescription() const
								{
									// Compose description
									CString	description;
									description +=
											CString(mBits) +
													((mSampleType == kSampleTypeFloat) ?
															CString(OSSTR(" (Float), ")) :
															CString(OSSTR(" (Signed Integer), ")));
									description += CString(mSampleRate, 0, 0) + CString(OSSTR("Hz, "));
									description +=
											CString(AUDIOCHANNELMAP_CHANNELCOUNT(mAudioChannelMap)) +
													CString(OSSTR(" (")) +
													eAudioChannelMapGetDescription(mAudioChannelMap) +
													CString(OSSTR("), "));
									description +=
											(mEndian == kEndianBig) ?
													CString(OSSTR("Big Endian, ")) : CString(OSSTR("Little Endian, "));
									description +=
											(mInterleaved == kInterleaved) ?
													CString(OSSTR("Interleaved")) :
													CString(OSSTR("Non-Interleaved"));

									return description;
								}

	SAudioProcessingFormat&	operator=(const SAudioProcessingFormat& other)
								{
									// Store
									mBits = other.mBits;
									mSampleRate = other.mSampleRate;
									mAudioChannelMap = other.mAudioChannelMap;
									mSampleType = other.mSampleType;
									mEndian = other.mEndian;
									mInterleaved = other.mInterleaved;

									return *this;
								}

	// Properties
	private:
		UInt8				mBits;
		Float32				mSampleRate;
		EAudioChannelMap	mAudioChannelMap;
		SampleType			mSampleType;
		Endian				mEndian;
		Interleaved			mInterleaved;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SAudioProcessingSetup

struct SAudioProcessingSetup {
	// BitsInfo
	struct BitsInfo {
		// Enums
		enum Option {
			kSpecified,
			kUnspecified,
			kUnchanged,
		};

		// Methods
		public:
				// Lifecycle methods
				BitsInfo(UInt8 bits) : mOption(kSpecified), mValue(bits) {}

				// Instance methods
		Option	getOption() const
					{ return mOption; }
		bool	isSpecified() const
					{ return mOption == kSpecified; }
		UInt8	getValue() const
					{ AssertFailIf(!mValue.hasValue()); return *mValue; }

		private:
				// Lifecycle methods
				BitsInfo(Option option) : mOption(option) {}

		// Properties
		public:
			static	const	BitsInfo	mUnspecified;
			static	const	BitsInfo	mUnchanged;

		private:
							Option		mOption;
							OV<UInt8>	mValue;
	};

	// SampleRateInfo
	struct SampleRateInfo {
		// Enums
		enum Option {
			kSpecified,
			kUnspecified,
			kUnchanged,
		};

		// Methods
		public:
				// Lifecycle methods
				SampleRateInfo(Float32 sampleRate) : mOption(kSpecified), mValue(sampleRate) {}

				// Instance methods
		Option	getOption() const
					{ return mOption; }
		bool	isSpecified() const
					{ return mOption == kSpecified; }
		Float32	getValue() const
					{ AssertFailIf(!mValue.hasValue()); return *mValue; }

		private:
				// Lifecycle methods
				SampleRateInfo(Option option) : mOption(option) {}

		// Properties
		public:
			static	const	SampleRateInfo	mUnspecified;
			static	const	SampleRateInfo	mUnchanged;

		private:
							Option			mOption;
							OV<Float32>		mValue;
	};

	// ChannelMapInfo
	struct ChannelMapInfo {
		// Enums
		enum Option {
			kSpecified,
			kUnspecified,
			kUnchanged,
		};

		// Methods
		public:
							// Lifecycle methods
							ChannelMapInfo(EAudioChannelMap audioChannelMap) :
								mOption(kSpecified), mValue(audioChannelMap)
								{}

							// Instance methods
		Option				getOption() const
								{ return mOption; }
		bool				isSpecified() const
								{ return mOption == kSpecified; }
		EAudioChannelMap	getValue() const
								{ AssertFailIf(!mValue.hasValue()); return *mValue; }

		private:
							// Lifecycle methods
							ChannelMapInfo(Option option) : mOption(option) {}

		// Properties
		public:
			static	const	ChannelMapInfo			mUnspecified;
			static	const	ChannelMapInfo			mUnchanged;

		private:
							Option					mOption;
							OV<EAudioChannelMap>	mValue;
	};

	// SampleTypeOption
	enum SampleTypeOption {
		kSampleTypeFloat,
		kSampleTypeSignedInteger,
		kSampleTypeUnspecified,
	};

	// EndianOption
	enum EndianOption {
		kEndianBig,
		kEndianLittle,
		kEndianUnspecified,
#if TARGET_RT_BIG_ENDIAN
		kEndianNative = kEndianBig,
#else
		kEndianNative = kEndianLittle,
#endif
	};

	// InterleavedOption
	enum InterleavedOption {
		kInterleaved,
		kNonInterleaved,
		kInterleavedUnspecified,
	};

								// Lifecycle methods
								SAudioProcessingSetup(const BitsInfo& bitsInfo, const SampleRateInfo& sampleRateInfo,
										const ChannelMapInfo& channelMapInfo, SampleTypeOption sampleTypeOption,
										EndianOption endianOption, InterleavedOption interleavedOption) :
									mBitsInfo(bitsInfo), mSampleRateInfo(sampleRateInfo),
											mChannelMapInfo(channelMapInfo), mSampleTypeOption(sampleTypeOption),
											mEndianOption(endianOption), mInterleavedOption(interleavedOption)
									{}
								SAudioProcessingSetup(UInt8 bits, Float32 sampleRate, EAudioChannelMap audioChannelMap,
										SampleTypeOption sampleTypeOption = kSampleTypeSignedInteger,
										EndianOption endianOption = kEndianNative,
										InterleavedOption interleavedOption = kInterleaved) :
									mBitsInfo(bits), mSampleRateInfo(sampleRate), mChannelMapInfo(audioChannelMap),
											mSampleTypeOption(sampleTypeOption), mEndianOption(endianOption),
											mInterleavedOption(
													(audioChannelMap == kAudioChannelMap_1_0) ?
															kInterleaved : interleavedOption)
									{}
								//SAudioProcessingSetup(const SAudioProcessingFormat& audioProcessingFormat) :
								//	mBitsInfo(audioProcessingFormat.getBits()),
								//			mSampleRateInfo(audioProcessingFormat.getSampleRate()),
								//			mChannelMapInfo(audioProcessingFormat.getAudioChannelMap()),
								//			mSampleTypeOption(
								//					audioProcessingFormat.getIsFloat() ?
								//							kSampleTypeFloat : kSampleTypeSignedInteger),
								//			mEndianOption(
								//					audioProcessingFormat.getIsBigEndian() ?
								//							kEndianBig : kEndianLittle),
								//			mInterleavedOption(
								//					audioProcessingFormat.getIsInterleaved() ?
								//							kInterleaved : kNonInterleaved)
								//	{}

								// Instance methods
	const	BitsInfo&			getBitsInfo() const
									{ return mBitsInfo; }
	const	SampleRateInfo&		getSampleRateInfo() const
									{ return mSampleRateInfo; }
	const	ChannelMapInfo&		getChannelMapInfo() const
									{ return mChannelMapInfo; }
			bool				isSampleTypeOptionSpecified() const
									{ return mSampleTypeOption != kSampleTypeUnspecified; }
			SampleTypeOption	getSampleTypeOption() const
									{ return mSampleTypeOption; }
			bool				isEndianOptionSpecified() const
									{ return mEndianOption != kEndianUnspecified; }
			EndianOption		getEndianOption() const
									{ return mEndianOption; }
			bool				isInterleavedOptionSpecified() const
									{ return mInterleavedOption != kInterleavedUnspecified; }
			InterleavedOption	getInterleavedOption() const
									{ return mInterleavedOption; }

	// Properties
	public:
		static	const	SAudioProcessingSetup	mUnspecified;

	private:
						BitsInfo				mBitsInfo;
						SampleRateInfo			mSampleRateInfo;
						ChannelMapInfo			mChannelMapInfo;
						SampleTypeOption		mSampleTypeOption;
						EndianOption			mEndianOption;
						InterleavedOption		mInterleavedOption;
};
