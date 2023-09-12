//----------------------------------------------------------------------------------------------------------------------
//	SAudio.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SMedia.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SAudio

struct SAudio {
	// ChannelMap
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
	struct ChannelMap {
		public:
										// Lifecycle methods
										ChannelMap(UInt8 channels) : mValue(channels) {}
										ChannelMap(const ChannelMap& other) : mValue(other.mValue) {}

										// Instance methods
							bool		hasUnknownOrder() const
											{ return (mValue & 0xFF00) == 0x0000; }
							UInt8		getChannels() const
											{ return mValue & 0x00FF; }
							CString		getDisplayString() const;

							UInt16		getRawValue() const
											{ return mValue; }

							bool		operator==(const ChannelMap& other) const
											{ return mValue == other.mValue; }
							bool		operator!=(const ChannelMap& other) const
											{ return mValue != other.mValue; }

										// Class methods
										// 1 Channel
			static	const	ChannelMap&	_1_0();				// C

										// 2 Channels
			static	const	ChannelMap&	_2_0_Option1();		// L R
			static	const	ChannelMap&	_1_1_Option1();		// C LFE
			static	const	ChannelMap&	_2_0_Option2();		// Ls Rs
			static	const	ChannelMap&	_2_0_Option3();		// Lt Rt (Matrix encoded stereo stream)
//			static	const	ChannelMap&	_2_0_Option4();		// Mide/Side
//			static	const	ChannelMap&	_2_0_Option5();		// Coincident Mic Pair (often 2 figure 8s)
//			static	const	ChannelMap&	_2_0_Option6();		// Binaural Stereo (L R)

										// 3 Channels
			static	const	ChannelMap&	_3_0_Unknown();		// Unknown
			static	const	ChannelMap&	_2_1_Option1();		// L R LFE
			static	const	ChannelMap&	_3_0_Option1();		// L R C
			static	const	ChannelMap&	_3_0_Option2();		// C L R
			static	const	ChannelMap&	_3_0_Option3();		// L R Cs
			static	const	ChannelMap&	_3_0_Option4();		// L C R

										// 4 Channels
			static	const	ChannelMap&	_4_0_Unknown();		// Unknown
			static	const	ChannelMap&	_3_1_Option1();		// L R LFE Cs
			static	const	ChannelMap&	_3_1_Option2();		// L R C LFE
			static	const	ChannelMap&	_3_1_Option3();		// L C R LFE
			static	const	ChannelMap&	_3_1_Option4();		// L R Cs LFE
			static	const	ChannelMap&	_4_0_Option1();		// L R Ls Rs (Quadraphonic)
			static	const	ChannelMap&	_4_0_Option2();		// L R C Cs
			static	const	ChannelMap&	_4_0_Option3();		// C L R Cs
			static	const	ChannelMap&	_4_0_Option4();		// L C R Cs
//			static	const	ChannelMap&	_4_0_Option5();		// Ambisonic B (W X Y Z)

										// 5 Channels
			static	const	ChannelMap&	_5_0_Unknown();		// Unknown
			static	const	ChannelMap&	_4_1_Option1();		// L R LFE Ls Rs
			static	const	ChannelMap&	_4_1_Option2();		// L R C LFE Cs
			static	const	ChannelMap&	_4_1_Option3();		// L R Ls Rs LFE
			static	const	ChannelMap&	_4_1_Option4();		// L C R Cs LFE
			static	const	ChannelMap&	_5_0_Option1();		// L R C Ls Rs
			static	const	ChannelMap&	_5_0_Option2();		// L R Ls Rs C (Pentagonal)
			static	const	ChannelMap&	_5_0_Option3();		// L C R Ls Rs
			static	const	ChannelMap&	_5_0_Option4();		// C L R Ls Rs

										// 6 Channels
			static	const	ChannelMap&	_6_0_Unknown();		// Unknown
			static	const	ChannelMap&	_5_1_Option1();		// L R C LFE Ls Rs
			static	const	ChannelMap&	_5_1_Option2();		// L R Ls Rs C LFE
			static	const	ChannelMap&	_5_1_Option3();		// L C R Ls Rs LFE
			static	const	ChannelMap&	_5_1_Option4();		// C L R Ls Rs LFE
			static	const	ChannelMap&	_6_0_Option1();			// L R Ls Rs C Cs (Hexagonal)
			static	const	ChannelMap&	_6_0_Option2();		// C L R Ls Rs Cs

										// 7 Channels
			static	const	ChannelMap&	_7_0_Unknown();		// Unknown
			static	const	ChannelMap&	_6_1_Option1();		// L R C LFE Ls Rs Cs
			static	const	ChannelMap&	_6_1_Option2();		// C L R Ls Rs Cs LFE
			static	const	ChannelMap&	_6_1_Option3();		// L C R Ls Cs Rs LFE
			static	const	ChannelMap&	_7_0_Option1();		// L R Ls Rs C Rls Rrs
			static	const	ChannelMap&	_7_0_Option2();		// C L R Ls Rs Rls Rrs
			static	const	ChannelMap&	_7_0_Option3();		// L R Ls Rs C Lc Rc

										// 8 Channels
			static	const	ChannelMap&	_8_0_Unknown();		// Unknown
			static	const	ChannelMap&	_7_1_Option1();		// L R C LFE Ls Rs Lc Rc
			static	const	ChannelMap&	_7_1_Option2();		// C Lc Rc L R Ls Rs LFE (doc: IS-13818-7 MPEG2-AAC Table 3.1)
			static	const	ChannelMap&	_7_1_Option3();		// L R C LFE Ls Rs Rls Rrs
			static	const	ChannelMap&	_7_1_Option4();		// L R Ls Rs C LFE Lc Rc (Emagic Default 7.1)
			static	const	ChannelMap&	_7_1_Option5();		// L R C LFE Ls Rs Lt Rt (SMPTE DTV)
			static	const	ChannelMap&	_7_1_Option6();		// L Lc C Rc R Ls Rs LFE
			static	const	ChannelMap&	_8_0_Option1();		// L R Ls Rs C Cs Lw Rw (Octagonal)
			static	const	ChannelMap&	_8_0_Option2();		// C L R Ls Rs Rls Rrs Cs
//			static	const	ChannelMap&	_8_0_Option2();		// L R Ls Rs topLeft topRight topRearLeft topRearRight (Cube)
			static	const	ChannelMap&	_7_1_Option7();		// C L R Ls Rs LFE Lt Rt (From Fotokem)

										// 9 Channels
			static	const	ChannelMap&	_9_0_Unknown();		// Unknown

										// 10 Channels
			static	const	ChannelMap&	_10_0_Unknown();		// Unknown

			static			ChannelMap	fromRawValue(UInt16 rawValue)
											{ return ChannelMap(rawValue); }

		private:
										// Lifecycle methods
										ChannelMap(UInt16 value) : mValue(value) {}

		// Properties
		private:
			UInt16	mValue;
	};

	// Format
	struct Format {
					// Lifecycle methods
					Format(OSType codecID, UInt8 bits, Float32 sampleRate, const ChannelMap& channelMap) :
						mCodecID(codecID), mBits(bits), mSampleRate(sampleRate), mChannelMap(channelMap)
						{}
					Format(OSType codecID, Float32 sampleRate, const ChannelMap& channelMap) :
						mCodecID(codecID), mSampleRate(sampleRate), mChannelMap(channelMap)
						{}
					Format(const Format& other, const ChannelMap& channelMap) :
						mCodecID(other.mCodecID), mBits(other.mBits), mSampleRate(other.mSampleRate),
								mChannelMap(channelMap)
						{}
					Format(const Format& other) :
						mCodecID(other.mCodecID), mBits(other.mBits), mSampleRate(other.mSampleRate),
								mChannelMap(other.mChannelMap)
						{}

					// Instance methods
		OSType		getCodecID() const
						{ return mCodecID; }
		OV<UInt8>	getBits() const
						{ return mBits; }
		Float32		getSampleRate() const
						{ return mSampleRate; }
		ChannelMap	getChannelMap() const
						{ return mChannelMap; }

		CString		getDescription() const
						{
							// Compose description
							CString	description;

							description +=
									mBits.hasValue() ?
											CString(*mBits) + CString(OSSTR(", ")) :
											CString(OSSTR("n/a, "));
							description += CString(mSampleRate, 0, 0) + CString(OSSTR("Hz, "));
							description +=
									CString(mChannelMap.getChannels()) + CString(OSSTR(" (")) +
											mChannelMap.getDisplayString() + CString(OSSTR(")"));

							return description;
						}

		// Properties
		private:
			OSType		mCodecID;
			OV<UInt8>	mBits;
			Float32		mSampleRate;
			ChannelMap	mChannelMap;
	};

	// ProcessingFormat
	struct ProcessingFormat {
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
									ProcessingFormat(UInt8 bits, Float32 sampleRate, const ChannelMap& channelMap,
											SampleType sampleType = kSampleTypeSignedInteger,
											Endian endian = kEndianNative, Interleaved interleaved = kInterleaved) :
										mBits(bits), mSampleRate(sampleRate), mChannelMap(channelMap),
												mSampleType(sampleType), mEndian(endian),
												mInterleaved(
														(channelMap == ChannelMap::_1_0()) ? kInterleaved : interleaved)
										{}
									ProcessingFormat(const ProcessingFormat& other) :
										mBits(other.mBits), mSampleRate(other.mSampleRate),
												mChannelMap(other.mChannelMap), mSampleType(other.mSampleType),
												mEndian(other.mEndian), mInterleaved(other.mInterleaved)
										{}

									// Instance methods
				UInt8				getBits() const
										{ return mBits; }

				Float32				getSampleRate() const
										{ return mSampleRate; }

		const	ChannelMap&			getChannelMap() const
										{ return mChannelMap; }

				bool				getIsFloat() const
										{ return mSampleType == kSampleTypeFloat; }
				bool				getIsSignedInteger() const
										{ return mSampleType == kSampleTypeSignedInteger; }
				SampleType			getSampleType() const
										{ return mSampleType; }

				bool				getIsBigEndian() const
										{ return mEndian == kEndianBig; }
				Endian				getEndian() const
										{ return mEndian; }

				bool				getIsInterleaved() const
										{ return mInterleaved == kInterleaved; }
				Interleaved			getInterleaved() const
										{ return mInterleaved; }

				UInt32				getBytesPerSample() const
										{ return mBits / 8; }
				UInt32				getBytesPerFrame() const
										{ return mBits / 8 * mChannelMap.getChannels(); }

				CString				getDescription() const
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
													CString(mChannelMap.getChannels()) +
															CString(OSSTR(" (")) +
															mChannelMap.getDisplayString() +
															CString(OSSTR("), "));
											description +=
													(mEndian == kEndianBig) ?
															CString(OSSTR("Big Endian, ")) :
															CString(OSSTR("Little Endian, "));
											description +=
													(mInterleaved == kInterleaved) ?
															CString(OSSTR("Interleaved")) :
															CString(OSSTR("Non-Interleaved"));

											return description;
										}

				ProcessingFormat&	operator=(const ProcessingFormat& other)
										{
											// Store
											mBits = other.mBits;
											mSampleRate = other.mSampleRate;
											mChannelMap = other.mChannelMap;
											mSampleType = other.mSampleType;
											mEndian = other.mEndian;
											mInterleaved = other.mInterleaved;

											return *this;
										}

		// Properties
		private:
			UInt8		mBits;
			Float32		mSampleRate;
			ChannelMap	mChannelMap;
			SampleType	mSampleType;
			Endian		mEndian;
			Interleaved	mInterleaved;
	};

	// ProcessingSetup
	struct ProcessingSetup {
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
									ChannelMapInfo(const ChannelMap& channelMap) :
										mOption(kSpecified), mValue(channelMap)
										{}

									// Instance methods
						Option		getOption() const
										{ return mOption; }
						bool		isSpecified() const
										{ return mOption == kSpecified; }
				const	ChannelMap&	getValue() const
										{ AssertFailIf(!mValue.hasValue()); return *mValue; }

						private:
									// Lifecycle methods
									ChannelMapInfo(Option option) : mOption(option) {}

			// Properties
			public:
				static	const	ChannelMapInfo	mUnspecified;
				static	const	ChannelMapInfo	mUnchanged;

			private:
								Option			mOption;
								OV<ChannelMap>	mValue;
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
									ProcessingSetup(const BitsInfo& bitsInfo, const SampleRateInfo& sampleRateInfo,
											const ChannelMapInfo& channelMapInfo, SampleTypeOption sampleTypeOption,
											EndianOption endianOption, InterleavedOption interleavedOption) :
										mBitsInfo(bitsInfo), mSampleRateInfo(sampleRateInfo),
												mChannelMapInfo(channelMapInfo), mSampleTypeOption(sampleTypeOption),
												mEndianOption(endianOption), mInterleavedOption(interleavedOption)
										{}
									ProcessingSetup(UInt8 bits, Float32 sampleRate,
											const ChannelMap& audioChannelMap,
											SampleTypeOption sampleTypeOption = kSampleTypeSignedInteger,
											EndianOption endianOption = kEndianNative,
											InterleavedOption interleavedOption = kInterleaved) :
										mBitsInfo(bits), mSampleRateInfo(sampleRate),
												mChannelMapInfo(audioChannelMap),
												mSampleTypeOption(sampleTypeOption), mEndianOption(endianOption),
												mInterleavedOption(
														(audioChannelMap == ChannelMap::_1_0()) ?
																kInterleaved : interleavedOption)
										{}

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
			static	const	ProcessingSetup			mUnspecified;

		private:
							BitsInfo				mBitsInfo;
							SampleRateInfo			mSampleRateInfo;
							ChannelMapInfo			mChannelMapInfo;
							SampleTypeOption		mSampleTypeOption;
							EndianOption			mEndianOption;
							InterleavedOption		mInterleavedOption;
	};


	// Methods
	public:
									// Class methods
		static	SMedia::SegmentInfo	composeMediaSegmentInfo(const Format& format, UniversalTimeInterval duration,
											UInt32 framesPerPacket, UInt32 bytesPerPacket)
										{ return SMedia::SegmentInfo(duration,
												(UInt32) (format.getSampleRate() / (Float32) framesPerPacket *
														(Float32) bytesPerPacket * 8.0f)); }
		static	SMedia::SegmentInfo	composeMediaSegmentInfo(const Format& format, UInt64 frameCount, UInt64 byteCount)
										{ return SMedia::SegmentInfo(
											(UniversalTimeInterval) frameCount /
													(UniversalTimeInterval) format.getSampleRate(),
											byteCount); }
		static	SMedia::SegmentInfo	composeMediaSegmentInfo(const Format& format, UInt32 framesPerPacket,
											UInt32 bytesPerPacket, UInt64 byteCount)
										{ return composeMediaSegmentInfo(format,
												byteCount / bytesPerPacket * framesPerPacket, byteCount); }

		static	Float32				getDBFromValue(Float32 value)
										{ return (Float32) (20.0 * log10(value)); }
		static	Float32				getValueFromDB(Float32 db)
										{ return powf(10.0, (Float32) (db / 20.0)); }

		static	CString				getDBDisplayString(Float32 value, Float32 muteValue = 0.0);
		static	Float32				getValueFromDBDisplayString(const CString& string)
										{ return getValueFromDB(string.getFloat32()); }
		static	CString				getPercentDisplayString(Float32 value)
										{ return CString(value * 100.0, 0, 1) + CString::mPercent; }
		static	Float32				getValueFromPercentDisplayString(const CString& string)
										{ return string.getFloat32() / 100.0; }
};
