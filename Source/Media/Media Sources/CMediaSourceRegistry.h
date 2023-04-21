//----------------------------------------------------------------------------------------------------------------------
//	CMediaSourceRegistry.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SMediaSource.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaSourceRegistry

class CMediaSourceRegistry {
	// Methods
	public:
												// Instance methods
				void							registerMediaSource(const SMediaSource& mediaSource);
		const	SMediaSource&					getMediaSource(OSType id) const;
				CString							getName(OSType id) const;
				TSet<CString>					getExtensions(OSType id) const;
				TSet<CString>					getAllMediaSourceExtensions() const;

				I<SMediaSource::ImportResult>	import(const SMediaSource::ImportSetup& importSetup,
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
