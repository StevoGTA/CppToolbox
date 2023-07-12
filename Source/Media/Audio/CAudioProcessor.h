//----------------------------------------------------------------------------------------------------------------------
//	CAudioProcessor.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioFrames.h"
#include "SAudio.h"
#include "SAudioSourceStatus.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioProcessor

class CAudioProcessor {
	// Classes
	private:
		class Internals;

	// Methods
	public:
												// Lifecycle methods
												CAudioProcessor();
		virtual									~CAudioProcessor();

												// Instance methods
		virtual	OV<SError>						connectInput(const I<CAudioProcessor>& audioProcessor,
														const SAudio::ProcessingFormat& audioProcessingFormat);
		virtual	TArray<CString>					getSetupDescription(const CString& indent);

		virtual	CAudioFrames::Requirements		queryRequirements() const;

		virtual	void							setSourceWindow(UniversalTimeInterval startTimeInterval,
														const OV<UniversalTimeInterval>& durationTimeInterval);
		virtual	void							seek(UniversalTimeInterval timeInterval);

		virtual	SAudioSourceStatus				performInto(CAudioFrames& audioFrames);

		virtual	void							reset();

												// Subclass methods
		virtual	TArray<SAudio::ProcessingSetup>	getInputSetups() const = 0;
		virtual	void							setInputFormat(const SAudio::ProcessingFormat& audioProcessingFormat)
														const
													{}

		virtual	TArray<SAudio::ProcessingSetup>	getOutputSetups() const = 0;
		virtual	OV<SError>						setOutputFormat(const SAudio::ProcessingFormat& audioProcessingFormat)
														= 0;

	// Properties
	private:
		Internals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioSource

class CAudioSource : public CAudioProcessor {
	// Methods
	public:
										// Lifecycle methods
										CAudioSource() : CAudioProcessor() {}

										// CAudioProcessor methods
		TArray<CString>					getSetupDescription(const CString& indent)
											{ return TNArray<CString>(); }

		TArray<SAudio::ProcessingSetup>	getInputSetups() const
											{ AssertFailUnimplemented(); return TNArray<SAudio::ProcessingSetup>(); }
		void							setInputFormat(const SAudio::ProcessingFormat& audioProcessingFormat) const
											{ AssertFailUnimplemented(); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioDestination

class CAudioDestination : public CAudioProcessor {
	// Methods
	public:
												// Lifecycle methods
												CAudioDestination() : CAudioProcessor() {}

												// CAudioProcessor methods
				TArray<SAudio::ProcessingSetup>	getOutputSetups() const
													{ AssertFailUnimplemented();
															return TNArray<SAudio::ProcessingSetup>(); }
				OV<SError>						setOutputFormat(const SAudio::ProcessingFormat& audioProcessingFormat)
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
		OV<SError>	setOutputFormat(const SAudio::ProcessingFormat& audioProcessingFormat)
						{
							// Store
							mOutputAudioProcessingFormat.setValue(audioProcessingFormat);

							return OV<SError>();
						}

	// Properties
	protected:
		OV<SAudio::ProcessingFormat>	mOutputAudioProcessingFormat;
};
