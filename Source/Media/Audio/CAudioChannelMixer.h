//----------------------------------------------------------------------------------------------------------------------
//	CAudioChannelMixer.h			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioProcessor.h"
#include "Tuple.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioChannelMixer

class CAudioChannelMixer : public CBasicAudioProcessor {
	public:
		struct Matrix : public TV3<UInt8, UInt8, TBuffer<Float32> > {
			// Methods
			public:
								// Instance methods
						UInt8	getInputChannelCount() const
									{ return getA(); }
						UInt8	getOutputChannelCount() const
									{ return getB(); }
						Float32	getGain(UInt8 outputChannelIndex, UInt8 inputChannelIndex) const
									{ return getC()[(UInt64) outputChannelIndex * getA() + inputChannelIndex]; }

								// Class methods
				static	Matrix	getMatrixForMono(UInt8 inputChannelCount);

			private:
								// Lifecycle methods
								Matrix(UInt8 inputChannelCount, UInt8 outputChannelCount,
										const TBuffer<Float32>& gains) :
									TV3(inputChannelCount, outputChannelCount, gains)
									{}
		};

	// Classes
	private:
		class Internals;

	// Methods
	public:
													// Lifecycle methods
		explicit									CAudioChannelMixer(const Matrix& matrix);
													~CAudioChannelMixer();

													// CAudioProcessor methods
					OV<SError>						connectInput(const I<CAudioProcessor>& audioProcessor,
															const SAudio::ProcessingFormat& audioProcessingFormat);
					TArray<CString>					getSetupDescription(const CString& indent);

					TNumberArray<Float32>			setChannelGains(const TNumberArray<Float32>& channelGains);

					TVResult<SourceInfo>			performInto(CAudioFrames& audioFrames);

					TArray<SAudio::ProcessingSetup>	getInputSetups() const;

					TArray<SAudio::ProcessingSetup>	getOutputSetups() const;

	// Properties
	private:
		Internals*	mInternals;
};
