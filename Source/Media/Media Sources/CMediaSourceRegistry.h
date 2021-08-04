//----------------------------------------------------------------------------------------------------------------------
//	CMediaSourceRegistry.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataSource.h"
#include "SMediaTracks.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SMediaSource

struct SMediaSource {
	// Info
	public:
		 struct Info {
			// Procs
			typedef	TIResult<SMediaTracks>	(*QueryTracksProc)(const I<CSeekableDataSource>& seekableDataSource);

											// Lifecycle methods
											Info(OSType id, const CString& name, const TArray<CString>& extensions,
													QueryTracksProc queryTracksProc) :
												mID(id), mName(name), mExtensions(extensions),
														mQueryTracksProc(queryTracksProc)
												{}
											Info(const Info& other) :
												mID(other.mID), mName(other.mName), mExtensions(other.mExtensions),
														mQueryTracksProc(other.mQueryTracksProc)
												{}

											// Instance methods
					OSType					getID() const
												{ return mID; }
			const	CString&				getName() const
												{ return mName; }
			const	TArray<CString>&		getExtensions()
												{ return mExtensions; }
					TIResult<SMediaTracks>	queryTracks(const I<CSeekableDataSource>& seekableDataSource)
												{ return mQueryTracksProc(seekableDataSource); }

			// Properties
			private:
				OSType				mID;
				CString				mName;
				TNArray<CString>	mExtensions;
				QueryTracksProc		mQueryTracksProc;
		};
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaSourceRegistry

class CMediaSourceRegistry {
	// IdentifyInfo
	public:
		struct IdentifyInfo {
									// Lifecycle methods
									IdentifyInfo(OSType id, const SMediaTracks& mediaTracks) :
										mID(id), mMediaTracks(mediaTracks)
										{}
									IdentifyInfo(const IdentifyInfo& other) :
										mID(other.mID), mMediaTracks(other.mMediaTracks)
										{}

									// Instance methods
					OSType			getID() const
										{ return mID; }
			const	SMediaTracks&	getMediaTracks() const
										{ return mMediaTracks; }

			// Properties
			private:
				OSType			mID;
				SMediaTracks	mMediaTracks;
		};

	// Methods
	public:
										// Instance methods
				void					registerMediaSource(const SMediaSource::Info& info);
		const	SMediaSource::Info&		getMediaSourceInfo(OSType id) const;
				TIResult<IdentifyInfo>	identify(const I<CSeekableDataSource>& seekableDataSource,
												const CString& extension) const;

	private:
										// Lifecycle methods
										CMediaSourceRegistry();

	// Properties
	public:
		static	CMediaSourceRegistry	mShared;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Macros

#define REGISTER_MEDIA_SOURCE(mediaSourceCodedName, info)													\
	class mediaSourceCodedName##Registerer {																\
		public:																								\
			mediaSourceCodedName##Registerer() { CMediaSourceRegistry::mShared.registerMediaSource(info); }	\
	};																										\
	static	mediaSourceCodedName##Registerer _##mediaSourceCodedName##Registerer
