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
		void						registerCodec(const CAudioCodec::Info& info);
		void						registerCodec(const CVideoCodec::Info& info);

		const	CAudioCodec::Info&	getAudioCodecInfo(OSType codecID);
		const	CVideoCodec::Info&	getVideoCodecInfo(OSType codecID);

	private:
									// Lifecycle methods
									CCodecRegistry();

	// Properties
	public:
		static	CCodecRegistry	mShared;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Macros

#define REGISTER_CODEC(codecCodedName, info)													\
	class codecCodedName##CodecRegisterer {														\
		public:																					\
			codecCodedName##CodecRegisterer() { CCodecRegistry::mShared.registerCodec(info); }	\
	};																							\
	static	codecCodedName##CodecRegisterer _##codecCodedName##CodecRegisterer
