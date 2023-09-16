//----------------------------------------------------------------------------------------------------------------------
//	CMediaDestination.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioProcessor.h"
#include "CLogServices.h"
#include "CVideoProcessor.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaDestination

class CMediaDestination {
	// Classes
	private:
		class Internals;

	// Methods
	public:
												// Lifecycle methods
		virtual									~CMediaDestination();

												// Instance methods
				const	CString&				getName() const;

						UInt32					getAudioTrackCount() const;
		virtual			void					add(const I<CAudioProcessor>& audioProcessor, UInt32 trackIndex);

						UInt32					getVideoTrackCount() const;
		virtual			void					add(const I<CVideoProcessor>& videoProcessor, UInt32 trackIndex);

		virtual			void					setupComplete() const = 0;

		virtual			void					setSourceWindow(UniversalTimeInterval startTimeInterval = 0.0,
														const OV<UniversalTimeInterval>& durationTimeInterval =
																OV<UniversalTimeInterval>());
		virtual			void					seek(UniversalTimeInterval timeInterval);

	protected:
												// Lifecycle methods
												CMediaDestination(const CString& name);

												// Instance methods
						OR<I<CAudioProcessor> >	getAudioProcessor(UInt32 trackIndex) const;
						void					removeAllAudioProcessors();

						OR<I<CVideoProcessor> >	getVideoProcessor(UInt32 trackIndex) const;
						void					removeAllVideoProcessors();

	// Properties
	private:
		Internals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TMediaDestination

template <typename T, typename U> class TMediaDestination : public CMediaDestination {
	// Methods
	public:
				// Lifecycle methods
				TMediaDestination(const CString& name) : CMediaDestination(name) {}

				// CMediaDestination methods
		void	setupComplete() const
					{
						// Setup
						TNArray<CString>	setupMessages;
						setupMessages += getName() + CString(OSSTR(" - Setup:"));

						// Iterate all audio tracks
						for (UInt32 i = 0; i < getAudioTrackCount(); i++) {
							// Setup
							T&	t = *((T*) &(**CMediaDestination::getAudioProcessor(i)));

							// Note setup is complete
							t.setupComplete();

							// Add setup messages
							setupMessages += CString(OSSTR("    Audio Track ")) + CString(i + 1) + CString::mColon;
							setupMessages += t.getSetupDescription(CString(OSSTR("        ")));
						}

						// Iterate all video tracks
						for (UInt32 i = 0; i < getVideoTrackCount(); i++) {
							// Setup
							U&	u = *((U*) &(**CMediaDestination::getVideoProcessor(i)));

							// Note setup is complete
							u.setupComplete();

							// Add setup messages
							setupMessages += CString(OSSTR("    Video Track ")) + CString(i + 1) + CString::mColon;
							setupMessages += u.getSetupDescription(CString(OSSTR("        ")));
						}

						// Log
						CLogServices::logMessages(setupMessages);
					}

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
