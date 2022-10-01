//----------------------------------------------------------------------------------------------------------------------
//	CMediaSourceRegistry.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMediaSourceRegistry.h"

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sCMediaSourceRegistryErrorDomain(OSSTR("CMediaSourceRegistry"));
static	SError	sImportFailed(sCMediaSourceRegistryErrorDomain, 1, CString(OSSTR("Failed to import")));

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
TIResult<CMediaSourceRegistry::ImportResult> CMediaSourceRegistry::import(
		const I<CRandomAccessDataSource>& randomAccessDataSource, const CString& extension,
		const OI<CAppleResourceManager>& appleResourceManager, UInt32 options) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CString			extensionUse = extension.lowercased();
	TSet<CString>	mediaSourceTypes = sMediaSourceRegistryInternals->mMediaSources.getKeys();
	TSet<CString>	remainingMediaSourceTypes = sMediaSourceRegistryInternals->mMediaSources.getKeys();

	// Iterate media source types to check file extension
	for (TIteratorS<CString> iterator = mediaSourceTypes.getIterator(); iterator.hasValue(); iterator.advance()) {
		// Get info
		SMediaSource&	mediaSource = *sMediaSourceRegistryInternals->mMediaSources[iterator->getUInt32()];

		// Check extensions
		if (mediaSource.getExtensions().contains(extensionUse)) {
			// Found by extension
			I<SMediaSource::ImportResult>	importResult =
													mediaSource.import(randomAccessDataSource, appleResourceManager,
															options);
			switch (importResult->getResult()) {
				case SMediaSource::ImportResult::kSuccess:
					// Success
					return TIResult<ImportResult>(
							ImportResult(mediaSource.getID(), importResult));

				case SMediaSource::ImportResult::kSourceMatchButUnableToLoad:
					// Matched source, but source unable to load
					return TIResult<ImportResult>(importResult->getError());

				case SMediaSource::ImportResult::kSourceMismatch:
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
		I<SMediaSource::ImportResult>	importResult =
												mediaSource.import(randomAccessDataSource, appleResourceManager,
														options);
		switch (importResult->getResult()) {
			case SMediaSource::ImportResult::kSuccess:
				// Success
				return TIResult<ImportResult>(ImportResult(mediaSource.getID(), importResult));

			case SMediaSource::ImportResult::kSourceMatchButUnableToLoad:
				// Matched source, but source unable to load
				return TIResult<ImportResult>(importResult->getError());

			case SMediaSource::ImportResult::kSourceMismatch:
				// Not a matched source
				break;
		}
	}

	return TIResult<ImportResult>(sImportFailed);
}
