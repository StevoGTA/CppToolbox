//----------------------------------------------------------------------------------------------------------------------
//	CAudioTrackReader.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioTrack.h"
#include "CAudioProcessor.h"
#include "SAudioFormats.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK:  CAudioTrackReader

class CAudioTrackReaderInternals;
class CAudioTrackReader : public CAudioSource {
	// Methods
	public:
										// Lifecycle methods
										CAudioTrackReader(const CAudioTrack& audioTrack, CByteParceller& byteParceller);
										CAudioTrackReader(const CAudioTrackReader& other);
										~CAudioTrackReader();

										// CAudioProcessor methods
		TArray<SAudioProcessingSetup>	getOutputSetups() const;
		void							setOutputFormat(const SAudioProcessingFormat& audioProcessingFormat);

		SAudioReadStatus				perform(const SMediaPosition& mediaPosition, CAudioData& audioData);

	// Properties
	private:
		CAudioTrackReaderInternals*	mInternals;
};
