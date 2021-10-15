//----------------------------------------------------------------------------------------------------------------------
//	CAudioProcessor.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioFrames.h"
#include "SAudioFormats.h"
#include "SAudioSourceStatus.h"
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
		virtual	OI<SError>						connectInput(const I<CAudioProcessor>& audioProcessor,
														const SAudioProcessingFormat& audioProcessingFormat);

		virtual	Requirements					queryRequirements() const;

		virtual	void							setSourceWindow(UniversalTimeInterval startTimeInterval,
														const OV<UniversalTimeInterval>& durationTimeInterval);
		virtual	void							seek(UniversalTimeInterval timeInterval);

		virtual	SAudioSourceStatus				performInto(CAudioFrames& audioFrames);

		virtual	void							reset();

												// Subclass methods
		virtual	TArray<SAudioProcessingSetup>	getInputSetups() const = 0;

		virtual	TArray<SAudioProcessingSetup>	getOutputSetups() const = 0;
		virtual	void							setOutputFormat(const SAudioProcessingFormat& audioProcessingFormat)
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
				void							setOutputFormat(const SAudioProcessingFormat& audioProcessingFormat)
													{ AssertFailUnimplemented(); }

												// Instance methods
		virtual	void							setupComplete() = 0;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioChannelMapper

class CAudioChannelMapperInternals;
class CAudioChannelMapper : public CAudioProcessor {
	public:
												// Lifecycle methods
												CAudioChannelMapper();
												~CAudioChannelMapper();

												// Instance methods
				OI<SError>						connectInput(const I<CAudioProcessor>& audioProcessor,
														const SAudioProcessingFormat& audioProcessingFormat);

				SAudioSourceStatus				performInto(CAudioFrames& audioFrames);

												// Subclass methods
				TArray<SAudioProcessingSetup>	getInputSetups() const;

				TArray<SAudioProcessingSetup>	getOutputSetups() const;
				void							setOutputFormat(const SAudioProcessingFormat& audioProcessingFormat);

												// Class methods
		static	bool							canPerform(EAudioChannelMap fromAudioChannelMap,
														EAudioChannelMap toAudioChannelMap);

	// Properties
	private:
		CAudioChannelMapperInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioConverter

class CAudioConverter : public CAudioProcessor {
	public:
										// CAudioProcessor methods
		TArray<SAudioProcessingSetup>	getInputSetups() const
											{ return TNArray<SAudioProcessingSetup>(SAudioProcessingSetup(
													*mOutputAudioProcessingFormat)); }

		TArray<SAudioProcessingSetup>	getOutputSetups() const
											{ return TNArray<SAudioProcessingSetup>(
													SAudioProcessingSetup::mUnspecified); }
		void							setOutputFormat(const SAudioProcessingFormat& audioProcessingFormat)
											{ mOutputAudioProcessingFormat =
													OI<SAudioProcessingFormat>(audioProcessingFormat); }

	protected:
										// Lifecycle methods
										CAudioConverter() {}

	// Properties
	protected:
		OI<SAudioProcessingFormat>	mOutputAudioProcessingFormat;
};
