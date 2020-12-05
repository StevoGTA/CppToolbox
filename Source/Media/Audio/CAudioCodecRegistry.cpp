//----------------------------------------------------------------------------------------------------------------------
//	CAudioCodecRegistry.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioCodecRegistry.h"

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioCodecRegistryInternals

class CAudioCodecRegistryInternals {
	public:
		CAudioCodecRegistryInternals() {}

		TKeyConvertibleDictionary<OSType, CAudioCodec::Info>	mInfo;
};

CAudioCodecRegistryInternals*	sInternals = nil;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioCodecRegistry

// MARK: Properties

CAudioCodecRegistry	CAudioCodecRegistry::mShared;

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioCodecRegistry::CAudioCodecRegistry()
//----------------------------------------------------------------------------------------------------------------------
{
//	mInternals = new CAudioCodecRegistryInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CAudioCodecRegistry::~CAudioCodecRegistry()
//----------------------------------------------------------------------------------------------------------------------
{
//	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CAudioCodecRegistry::registerCodec(const CAudioCodec::Info& info)
//----------------------------------------------------------------------------------------------------------------------
{
//	mInternals->mInfo.set(info.getID(), info);
	if (sInternals == nil)
		sInternals = new CAudioCodecRegistryInternals();
	sInternals->mInfo.set(info.getID(), info);
}

//----------------------------------------------------------------------------------------------------------------------
const CAudioCodec::Info& CAudioCodecRegistry::getInfo(OSType codecID)
//----------------------------------------------------------------------------------------------------------------------
{
//	return *mInternals->mInfo.get(codecID);
	return *sInternals->mInfo.get(codecID);
}
