//----------------------------------------------------------------------------------------------------------------------
//	CMediaDestination.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioProcessor.h"
#include "CVideoProcessor.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaDestination

class CMediaDestinationInternals;
class CMediaDestination {
	// Methods
	public:
										// Lifecycle methods
		virtual							~CMediaDestination();

										// Instance methods
				UInt32					getAudioTrackCount() const;
				UInt32					getVideoTrackCount() const;

		virtual	void					setSourceWindow(UniversalTimeInterval startTimeInterval = 0.0,
												const OV<UniversalTimeInterval>& durationTimeInterval =
														OV<UniversalTimeInterval>());
		virtual	void					seek(UniversalTimeInterval timeInterval);

										// Subclass methods
		virtual	void					setupComplete() = 0;

	protected:
										// Lifecycle methods
										CMediaDestination();

										// Instance methods
				void					add(const I<CAudioProcessor>& audioProcessor, UInt32 trackIndex);
				OR<I<CAudioProcessor> >	getAudioProcessor(UInt32 trackIndex) const;
				void					removeAllAudioProcessors();

				void					add(const I<CVideoProcessor>& videoProcessor, UInt32 trackIndex);
				OR<I<CVideoProcessor> >	getVideoProcessor(UInt32 trackIndex) const;
				void					removeAllVideoProcessors();

	// Properties
	private:
		CMediaDestinationInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TMediaDestination

template <typename T, typename U> class TMediaDestination : public CMediaDestination {
	// Methods
	public:
				// Lifecycle methods
				TMediaDestination() : CMediaDestination() {}

				// Instance methods
		OR<T>	getAudioProcessor(UInt32 trackIndex = 0) const
					{
						// Get Audio Processor
						OR<I<CAudioProcessor> >	audioProcessor = CMediaDestination::getAudioProcessor(trackIndex);

						return audioProcessor.hasReference() ? OR<T>(*((T*) &(**audioProcessor))) :  OR<T>();
					}
		OR<U>	getVideoProcessor(UInt32 trackIndex = 0) const
					{
						// Get Video Processor
						OR<I<CVideoProcessor> >	videoProcessor = CMediaDestination::getVideoProcessor(trackIndex);

						return videoProcessor.hasReference() ? OR<U>(*((U*) &(**videoProcessor))) :  OR<U>();
					}
};
