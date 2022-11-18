//----------------------------------------------------------------------------------------------------------------------
//	CAudioProcessor.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioFrames.h"
#include "SAudioFormats.h"
#include "SAudioSourceStatus.h"
#include "SError.h"
#include "TimeAndDate.h"
#include "TWrappers.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioProcessor

class CAudioProcessorInternals;
class CAudioProcessor {
	// Requirements
	public:
		struct Requirements {
			// Lifecycle methods
			Requirements(const CAudioFrames::Requirements& audioFramesRequirements) :
					mAudioFramesRequirements(audioFramesRequirements)
					{}
			Requirements(const Requirements& other) : mAudioFramesRequirements(other.mAudioFramesRequirements) {}

			// Properties
			CAudioFrames::Requirements	mAudioFramesRequirements;
		};

	// Methods
	public:
												// Lifecycle methods
												CAudioProcessor();
		virtual									~CAudioProcessor();

												// Instance methods
		virtual	OV<SError>						connectInput(const I<CAudioProcessor>& audioProcessor,
														const SAudioProcessingFormat& audioProcessingFormat);
		virtual	TNArray<CString>				getSetupDescription(const CString& indent);

		virtual	Requirements					queryRequirements() const;

		virtual	void							setSourceWindow(UniversalTimeInterval startTimeInterval,
														const OV<UniversalTimeInterval>& durationTimeInterval);
		virtual	void							seek(UniversalTimeInterval timeInterval);

		virtual	SAudioSourceStatus				performInto(CAudioFrames& audioFrames);

		virtual	void							reset();

												// Subclass methods
		virtual	TArray<SAudioProcessingSetup>	getInputSetups() const = 0;

		virtual	TArray<SAudioProcessingSetup>	getOutputSetups() const = 0;
		virtual	OV<SError>						setOutputFormat(const SAudioProcessingFormat& audioProcessingFormat)
														= 0;

	// Properties
	private:
		CAudioProcessorInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioSource

class CAudioSource : public CAudioProcessor {
	// Methods
	public:
										// Lifecycle methods
										CAudioSource() : CAudioProcessor() {}

										// CAudioProcessor methods
		TNArray<CString>				getSetupDescription(const CString& indent)
											{ return TNArray<CString>(); }

		TArray<SAudioProcessingSetup>	getInputSetups() const
											{ AssertFailUnimplemented(); return TNArray<SAudioProcessingSetup>(); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioDestination

class CAudioDestination : public CAudioProcessor {
	// Methods
	public:
												// Lifecycle methods
												CAudioDestination() : CAudioProcessor() {}

												// CAudioProcessor methods
				TArray<SAudioProcessingSetup>	getOutputSetups() const
													{ AssertFailUnimplemented();
															return TNArray<SAudioProcessingSetup>(); }
				OV<SError>						setOutputFormat(const SAudioProcessingFormat& audioProcessingFormat)
													{ AssertFailUnimplemented(); return OV<SError>(); }

												// Instance methods
		virtual	void							setupComplete() = 0;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBasicAudioProcessor

class CBasicAudioProcessor : public CAudioProcessor {
	// Methods
	public:
					// CAudioProcessor methods
		OV<SError>	setOutputFormat(const SAudioProcessingFormat& audioProcessingFormat)
						{
							// Store
							mOutputAudioProcessingFormat = OV<SAudioProcessingFormat>(audioProcessingFormat);

							return OV<SError>();
						}

	// Properties
	protected:
		OV<SAudioProcessingFormat>	mOutputAudioProcessingFormat;
};
