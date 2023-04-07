//----------------------------------------------------------------------------------------------------------------------
//	CCodecRegistry.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CCodecRegistry.h"

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCodecRegistryInternals

class CCodecRegistryInternals {
	public:
		CCodecRegistryInternals() {}

		TNKeyConvertibleDictionary<OSType, CCodec::Info>	mAudioCodecInfo;
		TNKeyConvertibleDictionary<OSType, CCodec::Info>	mVideoCodecInfo;
};

CCodecRegistryInternals*	sCodecRegistryInternals = nil;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CCodecRegistry

// MARK: Properties

CCodecRegistry	CCodecRegistry::mShared;

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CCodecRegistry::CCodecRegistry()
//----------------------------------------------------------------------------------------------------------------------
{
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CCodecRegistry::registerAudioCodec(const CCodec::Info& info)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if initialized
	if (sCodecRegistryInternals == nil)
		// Initialize
		sCodecRegistryInternals = new CCodecRegistryInternals();

	// Add info
	sCodecRegistryInternals->mAudioCodecInfo.set(info.getID(), info);
}

//----------------------------------------------------------------------------------------------------------------------
void CCodecRegistry::registerVideoCodec(const CCodec::Info& info)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if initialized
	if (sCodecRegistryInternals == nil)
		// Initialize
		sCodecRegistryInternals = new CCodecRegistryInternals();

	// Add info
	sCodecRegistryInternals->mVideoCodecInfo.set(info.getID(), info);
}

//----------------------------------------------------------------------------------------------------------------------
const CCodec::Info& CCodecRegistry::getAudioCodecInfo(OSType codecID)
//----------------------------------------------------------------------------------------------------------------------
{
	return *sCodecRegistryInternals->mAudioCodecInfo.get(codecID);
}

//----------------------------------------------------------------------------------------------------------------------
const CCodec::Info& CCodecRegistry::getVideoCodecInfo(OSType codecID)
//----------------------------------------------------------------------------------------------------------------------
{
	return *sCodecRegistryInternals->mVideoCodecInfo.get(codecID);
}
