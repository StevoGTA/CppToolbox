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

		TKeyConvertibleDictionary<OSType, SMediaSource::Info>	mMediaSourceInfo;
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
void CMediaSourceRegistry::registerMediaSource(const SMediaSource::Info& info)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if initialized
	if (sMediaSourceRegistryInternals == nil)
		// Initialize
		sMediaSourceRegistryInternals = new CMediaSourceRegistryInternals();

	// Add info
	sMediaSourceRegistryInternals->mMediaSourceInfo.set(info.getID(), info);
}

//----------------------------------------------------------------------------------------------------------------------
const SMediaSource::Info& CMediaSourceRegistry::getMediaSourceInfo(OSType id) const
//----------------------------------------------------------------------------------------------------------------------
{
	return *sMediaSourceRegistryInternals->mMediaSourceInfo.get(id);
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CMediaSourceRegistry::IdentifyInfo> CMediaSourceRegistry::identify(
		const I<CSeekableDataSource>& seekableDataSource, const CString& extension) const
//----------------------------------------------------------------------------------------------------------------------
{
	TSet<CString>	mediaSourceTypes = sMediaSourceRegistryInternals->mMediaSourceInfo.getKeys();
	for (TIteratorS<CString> iterator = mediaSourceTypes.getIterator(); iterator.hasValue(); iterator.advance()) {
		// Get info
		SMediaSource::Info&	mediaSourceInfo = *sMediaSourceRegistryInternals->mMediaSourceInfo[iterator->getUInt32()];

		// Check extensions
		if (mediaSourceInfo.getExtensions().contains(extension)) {
			// Found by extension
			TIResult<SMediaTracks>	mediaTracks = mediaSourceInfo.queryTracks(seekableDataSource);
			if (mediaTracks.hasValue())
				// Success
				return TIResult<IdentifyInfo>(IdentifyInfo(mediaSourceInfo.getID(), mediaTracks.getValue()));
		}
	}

	return TIResult<IdentifyInfo>(sUnknownExtension);
}
