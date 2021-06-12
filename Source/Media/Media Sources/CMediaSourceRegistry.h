//----------------------------------------------------------------------------------------------------------------------
//	CMediaSourceRegistry.h			©20021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

//#include "CArray.h"
#include "CAudioTrack.h"
#include "CDataSource.h"
#include "CVideoTrack.h"
//#include "TResult.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SMediaTracks

struct SMediaTracks {

									// Lifecycle methods
									SMediaTracks(const TArray<CAudioTrack>& audioTracks,
											const TArray<CVideoTrack>& videoTracks) :
										mAudioTracks(audioTracks), mVideoTracks(videoTracks)
										{}
									SMediaTracks(const TArray<CAudioTrack>& audioTracks) : mAudioTracks(audioTracks) {}
									SMediaTracks(const TArray<CVideoTrack>& videoTracks) : mVideoTracks(videoTracks) {}
									SMediaTracks(const SMediaTracks& other) :
										mAudioTracks(other.mAudioTracks), mVideoTracks(other.mVideoTracks)
										{}

									// Instance methods
	const	TArray<CAudioTrack>&	getAudioTracks() const
										{ return mAudioTracks; }
	const	TArray<CVideoTrack>&	getVideoTracks() const
										{ return mVideoTracks; }

	// Properties
	private:
		TNArray<CAudioTrack>	mAudioTracks;
		TNArray<CVideoTrack>	mVideoTracks;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SMediaSource

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
	// Methods
	public:
										// Instance methods
				void					registerMediaSource(const SMediaSource::Info& info);
		const	SMediaSource::Info&		getMediaSourceInfo(OSType id) const;
				TIResult<SMediaTracks>	queryTracks(const CString& extension,
												const I<CSeekableDataSource>& seekableDataSource) const;

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
