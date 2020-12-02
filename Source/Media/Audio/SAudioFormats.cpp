//----------------------------------------------------------------------------------------------------------------------
//	SAudioFormats.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SAudioFormats.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SBitsInfo

SAudioProcessingSetup::SBitsInfo	SAudioProcessingSetup::SBitsInfo::mUnspecified(kUnspecified);
SAudioProcessingSetup::SBitsInfo	SAudioProcessingSetup::SBitsInfo::mUnchanged(kUnchanged);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SSampleRateInfo

SAudioProcessingSetup::SSampleRateInfo	SAudioProcessingSetup::SSampleRateInfo::mUnspecified(kUnspecified);
SAudioProcessingSetup::SSampleRateInfo	SAudioProcessingSetup::SSampleRateInfo::mUnchanged(kUnchanged);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SChannelMapInfo

SAudioProcessingSetup::SChannelMapInfo	SAudioProcessingSetup::SChannelMapInfo::mUnspecified(kUnspecified);
SAudioProcessingSetup::SChannelMapInfo	SAudioProcessingSetup::SChannelMapInfo::mUnchanged(kUnchanged);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SAudioProcessingSetup

SAudioProcessingSetup	SAudioProcessingSetup::mUnspecified(SBitsInfo::mUnspecified, SSampleRateInfo::mUnspecified,
								SChannelMapInfo::mUnspecified, kSampleTypeUnspecified, kEndianUnspecified,
								kInterleavedUnspecified);
