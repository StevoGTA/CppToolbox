//----------------------------------------------------------------------------------------------------------------------
//	CCodecRegistry.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioCodec.h"
#include "CVideoCodec.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCodecRegistry

class CCodecRegistry {
	// Methods
	public:
								// Instance methods
		void					registerAudioCodec(const CCodec::Info& info);
		void					registerVideoCodec(const CCodec::Info& info);

		const	CCodec::Info&	getAudioCodecInfo(OSType codecID);
		const	CCodec::Info&	getVideoCodecInfo(OSType codecID);

	private:
								// Lifecycle methods
								CCodecRegistry();

	// Properties
	public:
		static	CCodecRegistry	mShared;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Macros

#define REGISTER_AUDIO_CODEC(codecCodedName, info)													\
	class codecCodedName##CodecRegisterer {															\
		public:																						\
			codecCodedName##CodecRegisterer() { CCodecRegistry::mShared.registerAudioCodec(info); }	\
	};																								\
	static	codecCodedName##CodecRegisterer _##codecCodedName##CodecRegisterer

#define REGISTER_VIDEO_CODEC(codecCodedName, info)													\
	class codecCodedName##CodecRegisterer {															\
		public:																						\
			codecCodedName##CodecRegisterer() { CCodecRegistry::mShared.registerVideoCodec(info); }	\
	};																								\
	static	codecCodedName##CodecRegisterer _##codecCodedName##CodecRegisterer
