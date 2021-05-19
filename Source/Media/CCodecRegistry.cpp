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

		TKeyConvertibleDictionary<OSType, CAudioCodec::Info>	mAudioCodecInfo;
		TKeyConvertibleDictionary<OSType, CVideoCodec::Info>	mVideoCodecInfo;
};

CCodecRegistryInternals*	sInternals = nil;

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
	if (sInternals == nil)
		// Initialize
		sInternals = new CCodecRegistryInternals();

	// Add info
	sInternals->mAudioCodecInfo.set(info.getID(), info);
}

//----------------------------------------------------------------------------------------------------------------------
void CCodecRegistry::registerCodec(const CVideoCodec::Info& info)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if initialized
	if (sInternals == nil)
		// Initialize
		sInternals = new CCodecRegistryInternals();

	// Add info
	sInternals->mVideoCodecInfo.set(info.getID(), info);
}

//----------------------------------------------------------------------------------------------------------------------
const CAudioCodec::Info& CCodecRegistry::getAudioCodecInfo(OSType codecID)
//----------------------------------------------------------------------------------------------------------------------
{
	return *sInternals->mAudioCodecInfo.get(codecID);
}

//----------------------------------------------------------------------------------------------------------------------
const CVideoCodec::Info& CCodecRegistry::getVideoCodecInfo(OSType codecID)
//----------------------------------------------------------------------------------------------------------------------
{
	return *sInternals->mVideoCodecInfo.get(codecID);
}
