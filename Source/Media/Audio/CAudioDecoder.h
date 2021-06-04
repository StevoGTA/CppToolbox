//----------------------------------------------------------------------------------------------------------------------
//	CAudioDecoder.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioProcessor.h"
#include "CCodec.h"
#include "CDataSource.h"
#include "SAudioFormats.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioDecoder

class CAudioDecoderInternals;
class CAudioDecoder : public CAudioSource {
	// Methods
	public:
										// Lifecycle methods
										CAudioDecoder(const SAudioStorageFormat& audioStorageFormat,
												const I<CCodec::DecodeInfo>& codecDecodeInfo,
												const I<CDataSource>& dataSource);
										CAudioDecoder(const CAudioDecoder& other);
										~CAudioDecoder();

										// CAudioProcessor methods
		TArray<SAudioProcessingSetup>	getOutputSetups() const;
		void							setOutputFormat(const SAudioProcessingFormat& audioProcessingFormat);

		SAudioReadStatus				perform(const SMediaPosition& mediaPosition, CAudioFrames& audioFrames);

	// Properties
	private:
		CAudioDecoderInternals*	mInternals;
};
