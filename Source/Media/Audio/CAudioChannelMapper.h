//----------------------------------------------------------------------------------------------------------------------
//	CAudioChannelMapper.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioProcessor.h"
#include "CBits.h"
#include "Tuple.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioChannelMapper

class CAudioChannelMapper : public CBasicAudioProcessor {
	public:
		struct Map : public TV2<UInt8, TNArray<OV<UInt8> > > {
			// Methods
			public:
									// Instance methods
						UInt8		getInputChannelCount() const
										{ return getA(); }
						UInt8		getOutputChannelCount() const
										{ return (UInt8) getB().getCount(); }
				const	OV<UInt8>&	getInputChannelIndex(UInt8 outputChannelIndex) const
										{ return getB()[outputChannelIndex]; }

									// Class methods
				static	Map			getMap(const SAudio::ChannelMap& fromAudioChannelMap,
											const SAudio::ChannelMap& toAudioChannelMap);
				static	Map			getMap(const CBits& fromChannels, const CBits& toChannels);

			private:
									// Lifecycle methods
									Map(UInt8 inputChannelCount, const TArray<OV<UInt8> >& inputChannelIndexes) :
										TV2(inputChannelCount, inputChannelIndexes)
										{}
		};

	// Classes
	private:
		class Internals;

	// Methods
	public:
													// Lifecycle methods
		explicit									CAudioChannelMapper(const Map& map);
													~CAudioChannelMapper();

													// CAudioProcessor methods
					OV<SError>						connectInput(const I<CAudioProcessor>& audioProcessor,
															const SAudio::ProcessingFormat& audioProcessingFormat);
					TArray<CString>					getSetupDescription(const CString& indent);

					TNumberArray<Float32>			setChannelGains(const TNumberArray<Float32>& channelGains);

					TVResult<SourceInfo>			performInto(CAudioFrames& audioFrames);

					TArray<SAudio::ProcessingSetup>	getInputSetups() const;

					TArray<SAudio::ProcessingSetup>	getOutputSetups() const;

													// Class methods
		static		bool							canPerform(const SAudio::ChannelMap& fromAudioChannelMap,
															const SAudio::ChannelMap& toAudioChannelMap);

	// Properties
	private:
		Internals*	mInternals;
};
