//----------------------------------------------------------------------------------------------------------------------
//	CCoreMediaVideoCodec.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CMediaPacketSource.h"
#include "CVideoCodec.h"

#include <CoreMedia/CoreMedia.h>
#undef TARGET_OS_WINDOWS

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreMediaDecodeVideoCodec

class CCoreMediaDecodeVideoCodec : public CDecodeVideoCodec {
	// Classes
	private:
		class Internals;

	// Methods
	public:
													// Lifecycle methods
													~CCoreMediaDecodeVideoCodec();

													// CDecodeVideoCodec methods
				OV<SError>							setup(const CVideoProcessor::Format& videoProcessorFormat);
				void								seek(UniversalTimeInterval timeInterval);
				TIResult<CVideoFrame>				decode();

	protected:
													// Lifecycle methods
													CCoreMediaDecodeVideoCodec(OSType codecID,
														const I<CMediaPacketSource>& mediaPacketSource,
														UInt32 timeScale, const TNumberArray<UInt32>& keyframeIndexes);

													// Subclass methods
		virtual	void								seek(UInt64 frameTime) {}
		virtual	TVResult<CMFormatDescriptionRef>	composeFormatDescription() = 0;
		virtual	TVResult<CMSampleTimingInfo>		composeSampleTimingInfo(
															const CMediaPacketSource::DataInfo& dataInfo,
															UInt32 timeScale) = 0;

	private:
				void								setCompatibility(CFMutableDictionaryRef dictionaryRef);

	// Properties
	private:
		Internals*	mInternals;
};
