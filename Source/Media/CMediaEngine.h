//----------------------------------------------------------------------------------------------------------------------
//	CMediaEngine.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioProcessor.h"
#include "CMediaDestination.h"
#include "TInstance.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaEngine

class CMediaEngine {
	// Structs
	public:
		struct ConnectResult {

												// Lifecycle methods
												ConnectResult(const SAudioProcessingFormat& audioProcessingFormat) :
													mAudioProcessingFormat(
															OI<SAudioProcessingFormat>(audioProcessingFormat))
													{}
												ConnectResult(const SError& error) : mError(OI<SError>(error)) {}

												// Instance methods
					bool						isSuccess() const
													{ return !mError.hasInstance(); }
			const	OI<SAudioProcessingFormat>&	getAudioProcessingFormat() const
													{ return mAudioProcessingFormat; }
			const	OI<SError>&					getError() const
													{ return mError; }

			// Properties
			private:
				OI<SAudioProcessingFormat>	mAudioProcessingFormat;
				OI<SError>					mError;
		};

	// Methods
	public:
										// Lifecycle methods
										CMediaEngine() {}
		virtual							~CMediaEngine() {}

										// Instance methods
				SAudioProcessingFormat	composeAudioProcessingFormat(const CAudioSource& audioSource,
												const CAudioDestination& audioDestination,
												const OV<Float32>& processingSampleRate = OV<Float32>()) const;
				ConnectResult			connect(const I<CAudioProcessor>& audioProcessorSource,
												const I<CAudioProcessor>& audioProcessorDestination,
												const SAudioProcessingFormat& audioProcessingFormat) const;

	protected:
										// Subclass methods
		virtual	I<CAudioConverter>		createAudioConverter() const = 0;
};
