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

		TNArray<SMediaSource>										mMediaSources;
		TNKeyConvertibleDictionary<OSType, SMediaSource>			mMediaSourcesByID;
		TNKeyConvertibleDictionary<OSType, SMediaSource::Identity>	mMediaSourceIdentitiesByID;
};

CMediaSourceRegistryInternals*	sMediaSourceRegistryInternals = nil;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaSourceRegistry

// MARK: Properties

CMediaSourceRegistry	CMediaSourceRegistry::mShared;

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
	sMediaSourceRegistryInternals->mMediaSources += mediaSource;
	for (TArray<SMediaSource::Identity>::Iterator iterator = mediaSource.getIdentities().getIterator(); iterator;
			iterator++) {
		// Add
		sMediaSourceRegistryInternals->mMediaSourcesByID.set(iterator->getID(), mediaSource);
		sMediaSourceRegistryInternals->mMediaSourceIdentitiesByID.set(iterator->getID(), *iterator);
	}
}

//----------------------------------------------------------------------------------------------------------------------
const SMediaSource& CMediaSourceRegistry::getMediaSource(OSType id) const
//----------------------------------------------------------------------------------------------------------------------
{
	return *sMediaSourceRegistryInternals->mMediaSourcesByID.get(id);
}

//----------------------------------------------------------------------------------------------------------------------
CString CMediaSourceRegistry::getName(OSType id) const
//----------------------------------------------------------------------------------------------------------------------
{
	return sMediaSourceRegistryInternals->mMediaSourceIdentitiesByID.get(id)->getName();
}

//----------------------------------------------------------------------------------------------------------------------
TSet<CString> CMediaSourceRegistry::getExtensions(OSType id) const
//----------------------------------------------------------------------------------------------------------------------
{
	return sMediaSourceRegistryInternals->mMediaSourcesByID.get(id)->getExtensions();
}

//----------------------------------------------------------------------------------------------------------------------
TSet<CString> CMediaSourceRegistry::getAllMediaSourceExtensions() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Collect all extensions
	TNSet<CString>	extensions;
	for (TArray<SMediaSource>::Iterator iterator = sMediaSourceRegistryInternals->mMediaSources.getIterator(); iterator;
			iterator++)
		// Add extensions
		extensions += iterator->getExtensions();

	return extensions;
}

//----------------------------------------------------------------------------------------------------------------------
I<SMediaSource::ImportResult> CMediaSourceRegistry::import(const SMediaSource::ImportSetup& importSetup,
		const OV<CString>& extension) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if have extension
	if (extension.hasValue()) {
		// Setup
		CString	extensionUse = extension->lowercased();

		// Iterate media sources to check file extension
		for (TArray<SMediaSource>::Iterator iterator = sMediaSourceRegistryInternals->mMediaSources.getIterator();
				iterator; iterator++) {
			// Check extensions
			if (iterator->getExtensions().contains(extensionUse)) {
				// Found by extension
				I<SMediaSource::ImportResult>	importResult = iterator->import(importSetup);
				if (importResult->getResult() != SMediaSource::ImportResult::kSourceMismatch)
					// Good enough
					return importResult;
			}
		}
	}

	// Iterate media sources and just check
	for (TArray<SMediaSource>::Iterator iterator = sMediaSourceRegistryInternals->mMediaSources.getIterator(); iterator;
			iterator++) {
		// Check extension situation
		if (!extension.hasValue() || !iterator->getExtensions().contains(*extension)) {
			// Query tracks
			I<SMediaSource::ImportResult>	importResult = iterator->import(importSetup);
			if (importResult->getResult() != SMediaSource::ImportResult::kSourceMismatch)
				// Good enough
				return importResult;
		}
	}

	return I<SMediaSource::ImportResult>(new SMediaSource::ImportResult(sImportFailed));
}
