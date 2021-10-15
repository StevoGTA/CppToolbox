//----------------------------------------------------------------------------------------------------------------------
//	CMediaSourceRegistry.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMediaSourceRegistry.h"

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sCMediaSourceRegistryErrorDomain(OSSTR("CMediaSourceRegistry"));
static	SError	sUnknownExtension(sCMediaSourceRegistryErrorDomain, 1, CString(OSSTR("Unknown Extension")));

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
		const I<CSeekableDataSource>& seekableDataSource, const CString& extension, SMediaSource::Options options) const
//----------------------------------------------------------------------------------------------------------------------
{
	TSet<CString>	mediaSourceTypes = sMediaSourceRegistryInternals->mMediaSources.getKeys();
	for (TIteratorS<CString> iterator = mediaSourceTypes.getIterator(); iterator.hasValue(); iterator.advance()) {
		// Get info
		SMediaSource&	mediaSource = *sMediaSourceRegistryInternals->mMediaSources[iterator->getUInt32()];

		// Check extensions
		if (mediaSource.getExtensions().contains(extension)) {
			// Found by extension
			TIResult<CMediaTrackInfos>	mediaTrackInfos = mediaSource.queryTracks(seekableDataSource, options);
			if (mediaTrackInfos.hasValue())
				// Success
				return TIResult<IdentifyInfo>(IdentifyInfo(mediaSource.getID(), mediaTrackInfos.getValue()));
		}
	}

	return TIResult<IdentifyInfo>(sUnknownExtension);
}
