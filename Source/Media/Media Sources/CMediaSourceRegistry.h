//----------------------------------------------------------------------------------------------------------------------
//	CMediaSourceRegistry.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SMediaSource.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaSourceRegistry

class CMediaSourceRegistry {
	// IdentifyInfo
	public:
		struct ImportResult {
													// Lifecycle methods
													ImportResult(OSType id,
															const I<SMediaSource::ImportResult>&
																	mediaSourceImportResult) :
														mID(id), mMediaSourceImportResult(mediaSourceImportResult)
														{}
													ImportResult(const ImportResult& other) :
														mID(other.mID),
																mMediaSourceImportResult(other.mMediaSourceImportResult)
														{}

													// Instance methods
					OSType							getID() const
														{ return mID; }
			const	I<SMediaSource::ImportResult>&	getMediaSourceImportResult() const
														{ return mMediaSourceImportResult; }

			// Properties
			private:
				OSType							mID;
				I<SMediaSource::ImportResult>	mMediaSourceImportResult;
		};

	// Methods
	public:
										// Instance methods
				void					registerMediaSource(const SMediaSource& mediaSource);
		const	SMediaSource&			getMediaSource(OSType id) const;
				TSet<CString>			getAllMediaSourceExtensions() const;

				TVResult<ImportResult>	import(const SMediaSource::ImportSetup& importSetup,
												const OV<CString>& extension) const;

	private:
										// Lifecycle methods
										CMediaSourceRegistry() {}

	// Properties
	public:
		static	CMediaSourceRegistry	mShared;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Macros

#define REGISTER_MEDIA_SOURCE(mediaSourceCodedName, mediaSource)										\
	class mediaSourceCodedName##MediaSourceRegisterer {													\
		public:																							\
			mediaSourceCodedName##MediaSourceRegisterer()												\
					{ CMediaSourceRegistry::mShared.registerMediaSource(mediaSource); }					\
	};																									\
	static	mediaSourceCodedName##MediaSourceRegisterer _##mediaSourceCodedName##MediaSourceRegisterer
