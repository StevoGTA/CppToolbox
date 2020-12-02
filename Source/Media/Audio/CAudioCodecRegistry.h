//----------------------------------------------------------------------------------------------------------------------
//	CAudioCodecRegistry.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioCodec.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK:  CAudioCodecRegistry

class CAudioCodecRegistryInternals;
class CAudioCodecRegistry {
	// Methods
	public:
									// Instance methods
		void						registerCodec(const CAudioCodec::SInfo& info);
		const	CAudioCodec::SInfo&	getInfo(OSType codecID);

	private:
									// Lifecycle methods
									CAudioCodecRegistry();
									~CAudioCodecRegistry();

	// Properties
	public:
		static	CAudioCodecRegistry				mShared;

	private:
				CAudioCodecRegistryInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Macros

#define REGISTER_AUDIO_CODEC(codecCodedName, info)												\
	class codecCodedName##Initializor {															\
		public:																					\
			codecCodedName##Initializor() { CAudioCodecRegistry::mShared.registerCodec(info); }	\
	};																							\
	static	codecCodedName##Initializor _##codecCodedName##Initializor
