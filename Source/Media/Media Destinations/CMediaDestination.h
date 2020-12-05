//----------------------------------------------------------------------------------------------------------------------
//	CMediaDestination.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioProcessor.h"

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

										// Subclass methods
		virtual	void					setupComplete() = 0;

	protected:
										// Lifecycle methods
										CMediaDestination();

										// Instance methods
				void					add(const I<CAudioProcessor>& audioProcessor, UInt32 trackIndex);
				OR<I<CAudioProcessor> >	getAudioProcessor(UInt32 trackIndex) const;

	// Properties
	private:
		CMediaDestinationInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TMediaDestination

template <typename T> class TMediaDestination : public CMediaDestination {
	// Methods
	public:
				// Lifecycle methods
				TMediaDestination() : CMediaDestination() {}

				// Instance methods
		void	add(const I<T>& t, UInt32 trackIndex = 0)
					{ CMediaDestination::add((const I<CAudioProcessor>&) t, trackIndex); }
		OR<T>	getAudioProcessor(UInt32 trackIndex = 0) const
					{
						// Get Audio Processor
						OR<I<CAudioProcessor> >	audioProcessor = CMediaDestination::getAudioProcessor(trackIndex);

						return audioProcessor.hasReference() ? OR<T>(*((T*) &(**audioProcessor))) :  OR<T>();
					}
};
