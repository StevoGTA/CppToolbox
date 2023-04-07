//----------------------------------------------------------------------------------------------------------------------
//	SAudioFormats.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SAudioFormats.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SAudioProcessingSetup::BitsInfo

const	SAudioProcessingSetup::BitsInfo	SAudioProcessingSetup::BitsInfo::mUnspecified(kUnspecified);
const	SAudioProcessingSetup::BitsInfo	SAudioProcessingSetup::BitsInfo::mUnchanged(kUnchanged);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SAudioProcessingSetup::SampleRateInfo

const	SAudioProcessingSetup::SampleRateInfo	SAudioProcessingSetup::SampleRateInfo::mUnspecified(kUnspecified);
const	SAudioProcessingSetup::SampleRateInfo	SAudioProcessingSetup::SampleRateInfo::mUnchanged(kUnchanged);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SAudioProcessingSetup::ChannelMapInfo

const	SAudioProcessingSetup::ChannelMapInfo	SAudioProcessingSetup::ChannelMapInfo::mUnspecified(kUnspecified);
const	SAudioProcessingSetup::ChannelMapInfo	SAudioProcessingSetup::ChannelMapInfo::mUnchanged(kUnchanged);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SAudioProcessingSetup

const	SAudioProcessingSetup	SAudioProcessingSetup::mUnspecified(BitsInfo::mUnspecified,
										SampleRateInfo::mUnspecified, ChannelMapInfo::mUnspecified,
										kSampleTypeUnspecified, kEndianUnspecified, kInterleavedUnspecified);
