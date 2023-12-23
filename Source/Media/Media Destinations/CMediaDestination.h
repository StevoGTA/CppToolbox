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
		virtual										~CMediaDestination();

													// Instance methods
				const	CString&					getName() const;

						UInt32						getAudioTrackCount() const;
		virtual			void						add(const I<CAudioDestination>& audioDestination,
															UInt32 trackIndex);

						UInt32						getVideoTrackCount() const;
		virtual			void						add(const I<CVideoDestination>& videoDestination,
															UInt32 trackIndex);

		virtual			void						setupComplete() const = 0;

		virtual			void						setMediaSegment(
															const OV<SMedia::Segment>& mediaSegment =
																	OV<SMedia::Segment>());
		virtual			void						seek(UniversalTimeInterval timeInterval);

	protected:
													// Lifecycle methods
													CMediaDestination(const CString& name);

													// Instance methods
						OR<I<CAudioDestination> >	getAudioDestination(UInt32 trackIndex) const;
						void						removeAllAudioDestinations();

						OR<I<CVideoDestination> >	getVideoDestination(UInt32 trackIndex) const;
						void						removeAllVideoDestinations();

	// Properties
	private:
		Internals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TMediaDestination

template <typename AD, typename VD> class TMediaDestination : public CMediaDestination {
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
							AD&	ad = *((AD*) &(**CMediaDestination::getAudioDestination(i)));

							// Note setup is complete
							ad.setupComplete();

							// Add setup messages
							setupMessages += CString(OSSTR("    Audio Track ")) + CString(i + 1) + CString::mColon;
							setupMessages += ad.getSetupDescription(CString(OSSTR("        ")));
						}

						// Iterate all video tracks
						for (UInt32 i = 0; i < getVideoTrackCount(); i++) {
							// Setup
							VD&	vd = *((VD*) &(**CMediaDestination::getVideoDestination(i)));

							// Note setup is complete
							vd.setupComplete();

							// Add setup messages
							setupMessages += CString(OSSTR("    Video Track ")) + CString(i + 1) + CString::mColon;
							setupMessages += vd.getSetupDescription(CString(OSSTR("        ")));
						}

						// Log
						CLogServices::logMessages(setupMessages);
					}

		OR<AD>	getAudioDestination(UInt32 trackIndex = 0) const
					{
						// Get Audio Destination
						OR<I<CAudioDestination> >	audioDestination =
															CMediaDestination::getAudioDestination(trackIndex);

						return audioDestination.hasReference() ? OR<AD>(*((AD*) &(**audioDestination))) :  OR<AD>();
					}
		OR<VD>	getVideoDestination(UInt32 trackIndex = 0) const
					{
						// Get Video Destination
						OR<I<CVideoDestination> >	videoDestination =
															CMediaDestination::getVideoDestination(trackIndex);

						return videoDestination.hasReference() ? OR<VD>(*((VD*) &(**videoDestination))) :  OR<VD>();
					}
};
