//----------------------------------------------------------------------------------------------------------------------
//	SAudioFormats.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SAudioFormats.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: BitsInfo

SAudioProcessingSetup::BitsInfo	SAudioProcessingSetup::BitsInfo::mUnspecified(kUnspecified);
SAudioProcessingSetup::BitsInfo	SAudioProcessingSetup::BitsInfo::mUnchanged(kUnchanged);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SampleRateInfo

SAudioProcessingSetup::SampleRateInfo	SAudioProcessingSetup::SampleRateInfo::mUnspecified(kUnspecified);
SAudioProcessingSetup::SampleRateInfo	SAudioProcessingSetup::SampleRateInfo::mUnchanged(kUnchanged);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - ChannelMapInfo

SAudioProcessingSetup::ChannelMapInfo	SAudioProcessingSetup::ChannelMapInfo::mUnspecified(kUnspecified);
SAudioProcessingSetup::ChannelMapInfo	SAudioProcessingSetup::ChannelMapInfo::mUnchanged(kUnchanged);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SAudioProcessingSetup

SAudioProcessingSetup	SAudioProcessingSetup::mUnspecified(BitsInfo::mUnspecified, SampleRateInfo::mUnspecified,
								ChannelMapInfo::mUnspecified, kSampleTypeUnspecified, kEndianUnspecified,
								kInterleavedUnspecified);
