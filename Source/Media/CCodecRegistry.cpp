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

		TNKeyConvertibleDictionary<OSType, CAudioCodec::Info>	mAudioCodecInfo;
		TNKeyConvertibleDictionary<OSType, CVideoCodec::Info>	mVideoCodecInfo;
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
void CCodecRegistry::registerCodec(const CAudioCodec::Info& info)
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
void CCodecRegistry::registerCodec(const CVideoCodec::Info& info)
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
const CAudioCodec::Info& CCodecRegistry::getAudioCodecInfo(OSType codecID)
//----------------------------------------------------------------------------------------------------------------------
{
	return *sCodecRegistryInternals->mAudioCodecInfo.get(codecID);
}

//----------------------------------------------------------------------------------------------------------------------
const CVideoCodec::Info& CCodecRegistry::getVideoCodecInfo(OSType codecID)
//----------------------------------------------------------------------------------------------------------------------
{
	return *sCodecRegistryInternals->mVideoCodecInfo.get(codecID);
}
