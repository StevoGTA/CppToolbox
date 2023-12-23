//----------------------------------------------------------------------------------------------------------------------
//	CAudioProcessor.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioFrames.h"
#include "SAudio.h"
#include "TResult.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioProcessor

class CAudioProcessor {
	// SourceInfo
	public:
		struct SourceInfo {
										// Lifecycle methods
										SourceInfo(UniversalTimeInterval timeInterval) : mTimeInterval(timeInterval) {}
										SourceInfo(const SourceInfo& other) : mTimeInterval(other.mTimeInterval) {}

										// Instance methods
				UniversalTimeInterval	getTimeInterval() const
											{ return mTimeInterval; }

			// Properties
			private:
				UniversalTimeInterval	mTimeInterval;
		};

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

		virtual	void							setMediaSegment(const OV<SMedia::Segment>& mediaSegment);
		virtual	void							seek(UniversalTimeInterval timeInterval);

		virtual	TVResult<SourceInfo>			performInto(CAudioFrames& audioFrames);

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
	// Classes
	private:
		class Internals;

	// Methods
	public:
										// Lifecycle methods
										CAudioSource();
										~CAudioSource();

										// CAudioProcessor methods
		TArray<CString>					getSetupDescription(const CString& indent)
											{ return TNArray<CString>(); }

		void							setMediaSegment(const OV<SMedia::Segment>& mediaSegment);
		void							seek(UniversalTimeInterval timeInterval);

		void							reset();

		TArray<SAudio::ProcessingSetup>	getInputSetups() const
											{ AssertFailUnimplemented(); return TNArray<SAudio::ProcessingSetup>(); }
		void							setInputFormat(const SAudio::ProcessingFormat& audioProcessingFormat) const
											{ AssertFailUnimplemented(); }

	protected:
										// Instance methods
		UniversalTimeInterval			getCurrentTimeInterval() const;
		void							setCurrentTimeInterval(UniversalTimeInterval timeInterval);

		TVResult<UInt32>				calculateMaxFrames(Float32 sampleRate) const;

	// Properties
	private:
		Internals*	mInternals;
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
		virtual	void							setupComplete()
													{}
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
