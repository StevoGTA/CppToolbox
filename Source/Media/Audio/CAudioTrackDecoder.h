//----------------------------------------------------------------------------------------------------------------------
//	CAudioTrackDecoder.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioTrack.h"
#include "CAudioProcessor.h"
#include "SAudioFormats.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioTrackDecoder

class CAudioTrackDecoderInternals;
class CAudioTrackDecoder : public CAudioSource {
	// Methods
	public:
										// Lifecycle methods
										CAudioTrackDecoder(const CAudioTrack& audioTrack,
												const I<CDataSource>& dataSource);
										CAudioTrackDecoder(const CAudioTrackDecoder& other);
										~CAudioTrackDecoder();

										// CAudioProcessor methods
		TArray<SAudioProcessingSetup>	getOutputSetups() const;
		void							setOutputFormat(const SAudioProcessingFormat& audioProcessingFormat);

		SAudioReadStatus				perform(const SMediaPosition& mediaPosition, CAudioFrames& audioFrames);

	// Properties
	private:
		CAudioTrackDecoderInternals*	mInternals;
};
