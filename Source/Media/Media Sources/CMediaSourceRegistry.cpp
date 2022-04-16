//----------------------------------------------------------------------------------------------------------------------
//	CMediaSourceRegistry.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMediaSourceRegistry.h"

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sCMediaSourceRegistryErrorDomain(OSSTR("CMediaSourceRegistry"));
static	SError	sIdentifyFailed(sCMediaSourceRegistryErrorDomain, 1, CString(OSSTR("Failed to identify")));

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaSourceRegistryInternals

class CMediaSourceRegistryInternals {
	public:
		CMediaSourceRegistryInternals() {}

		TKeyConvertibleDictionary<OSType, SMediaSource>	mMediaSources;
};

CMediaSourceRegistryInternals*	sMediaSourceRegistryInternals = nil;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaSourceRegistry

// MARK: Properties

CMediaSourceRegistry	CMediaSourceRegistry::mShared;

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CMediaSourceRegistry::CMediaSourceRegistry()
//----------------------------------------------------------------------------------------------------------------------
{
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CMediaSourceRegistry::registerMediaSource(const SMediaSource& mediaSource)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if initialized
	if (sMediaSourceRegistryInternals == nil)
		// Initialize
		sMediaSourceRegistryInternals = new CMediaSourceRegistryInternals();

	// Add info
	sMediaSourceRegistryInternals->mMediaSources.set(mediaSource.getID(), mediaSource);
}

//----------------------------------------------------------------------------------------------------------------------
const SMediaSource& CMediaSourceRegistry::getMediaSource(OSType id) const
//----------------------------------------------------------------------------------------------------------------------
{
	return *sMediaSourceRegistryInternals->mMediaSources.get(id);
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CMediaSourceRegistry::IdentifyInfo> CMediaSourceRegistry::identify(
		const I<CRandomAccessDataSource>& randomAccessDataSource, const CString& extension,
		const OI<CAppleResourceManager>& appleResourceManager, SMediaSource::Options options) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TSet<CString>	mediaSourceTypes = sMediaSourceRegistryInternals->mMediaSources.getKeys();
	TSet<CString>	remainingMediaSourceTypes = sMediaSourceRegistryInternals->mMediaSources.getKeys();

	// Iterate media source types to check file extension
	for (TIteratorS<CString> iterator = mediaSourceTypes.getIterator(); iterator.hasValue(); iterator.advance()) {
		// Get info
		SMediaSource&	mediaSource = *sMediaSourceRegistryInternals->mMediaSources[iterator->getUInt32()];

		// Check extensions
		if (mediaSource.getExtensions().contains(extension)) {
			// Found by extension
			SMediaSource::QueryTracksResult	queryTracksResult =
													mediaSource.queryTracks(randomAccessDataSource,
															appleResourceManager, options);
			switch (queryTracksResult.getResult()) {
				case SMediaSource::QueryTracksResult::kSuccess:
					// Success
					return TIResult<IdentifyInfo>(
							IdentifyInfo(mediaSource.getID(), queryTracksResult.getMediaTrackInfos()));

				case SMediaSource::QueryTracksResult::kSourceMatchButUnableToLoad:
					// Matched source, but source unable to load
					return TIResult<IdentifyInfo>(queryTracksResult.getError());

				case SMediaSource::QueryTracksResult::kSourceMismatch:
					// Not a matched source
					remainingMediaSourceTypes.remove(*iterator);
					break;
			}
		}
	}

	// Iterate media source types and just check
	for (TIteratorS<CString> iterator = remainingMediaSourceTypes.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Get info
		SMediaSource&	mediaSource = *sMediaSourceRegistryInternals->mMediaSources[iterator->getUInt32()];

		// Query tracks
		SMediaSource::QueryTracksResult	queryTracksResult =
												mediaSource.queryTracks(randomAccessDataSource, appleResourceManager,
														options);
		switch (queryTracksResult.getResult()) {
			case SMediaSource::QueryTracksResult::kSuccess:
				// Success
				return TIResult<IdentifyInfo>(
						IdentifyInfo(mediaSource.getID(), queryTracksResult.getMediaTrackInfos()));

			case SMediaSource::QueryTracksResult::kSourceMatchButUnableToLoad:
				// Matched source, but source unable to load
				return TIResult<IdentifyInfo>(queryTracksResult.getError());

			case SMediaSource::QueryTracksResult::kSourceMismatch:
				// Not a matched source
				break;
		}
	}

	return TIResult<IdentifyInfo>(sIdentifyFailed);
}
