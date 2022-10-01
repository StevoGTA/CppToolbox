//----------------------------------------------------------------------------------------------------------------------
//	CCoreMediaVideoCodec.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CVideoCodec.h"

#include <CoreMedia/CoreMedia.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreMediaDecodeVideoCodec

class CCoreMediaDecodeVideoCodecInternals;
class CCoreMediaDecodeVideoCodec : public CDecodeVideoCodec {
	// Methods
	public:
													// Lifecycle methods
													~CCoreMediaDecodeVideoCodec();

													// CDecodeVideoCodec methods
				OI<SError>							setup(const SVideoProcessingFormat& videoProcessingFormat);
				void								seek(UniversalTimeInterval timeInterval);
				TIResult<CVideoFrame>				decode();

	protected:
													// Lifecycle methods
													CCoreMediaDecodeVideoCodec(OSType codecID,
														const I<CMediaPacketSource>& mediaPacketSource,
														UInt32 timeScale, const TNumberArray<UInt32>& keyframeIndexes);

													// Subclass methods
		virtual			void						seek(UInt64 frameTime) {}
		virtual	TVResult<CMFormatDescriptionRef>	composeFormatDescription() = 0;
		virtual	TVResult<CMSampleTimingInfo>		composeSampleTimingInfo(
															const CMediaPacketSource::DataInfo& dataInfo,
															UInt32 timeScale) = 0;

	// Properties
	private:
		CCoreMediaDecodeVideoCodecInternals*	mInternals;
};
