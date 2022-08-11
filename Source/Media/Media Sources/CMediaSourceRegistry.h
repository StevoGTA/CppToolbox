//----------------------------------------------------------------------------------------------------------------------
//	CMediaSourceRegistry.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAppleResourceManager.h"
#include "CDataSource.h"
#include "SMediaTracks.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SMediaSource

struct SMediaSource {
	// Types
	enum Options {
		kNone			= 0,
		kCreateDecoders	= 1 << 0,
	};

	// ImportResult
	struct ImportResult {
		// Result
		enum Result {
			kSuccess,
			kSourceMatchButUnableToLoad,
			kSourceMismatch,
		};

									// Lifecycle methods
									ImportResult(const CMediaTrackInfos& mediaTrackInfos) :
										mResult(kSuccess), mMediaTrackInfos(mediaTrackInfos)
										{}
									ImportResult(const SError& error) :
										mResult(kSourceMatchButUnableToLoad), mError(error)
										{}
									ImportResult() : mResult(kSourceMismatch) {}
									ImportResult(const ImportResult& other) :
										mResult(other.mResult), mMediaTrackInfos(other.mMediaTrackInfos),
												mError(other.mError)
										{}

									// Instance methods
		const	Result				getResult() const
										{ return mResult; }
		const	CMediaTrackInfos&	getMediaTrackInfos() const
										{ return *mMediaTrackInfos; }
		const	SError&				getError() const
										{ return *mError; }

		// Properties
		private:
			Result					mResult;
			OI<CMediaTrackInfos>	mMediaTrackInfos;
			OI<SError>				mError;
	};

	// Procs
	typedef	ImportResult		(*ImportProc)(const I<CRandomAccessDataSource>& randomAccessDataSource,
										const OI<CAppleResourceManager>& appleResourceManager, Options options);

								// Lifecycle methods
								SMediaSource(OSType id, const CString& name, const TArray<CString>& extensions,
										ImportProc importProc) :
									mID(id), mName(name), mExtensions(extensions), mImportProc(importProc)
									{}
								SMediaSource(const SMediaSource& other) :
									mID(other.mID), mName(other.mName), mExtensions(other.mExtensions),
											mImportProc(other.mImportProc)
									{}

								// Instance methods
			OSType				getID() const
									{ return mID; }
	const	CString&			getName() const
									{ return mName; }
	const	TArray<CString>&	getExtensions() const
									{ return mExtensions; }
			ImportResult		import(const I<CRandomAccessDataSource>& randomAccessDataSource,
										const OI<CAppleResourceManager>& appleResourceManager, Options options) const
									{ return mImportProc(randomAccessDataSource, appleResourceManager, options); }

	// Properties
	private:
		OSType				mID;
		CString				mName;
		TNArray<CString>	mExtensions;
		ImportProc			mImportProc;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaSourceRegistry

class CMediaSourceRegistry {
	// IdentifyInfo
	public:
		struct ImportResult {
										// Lifecycle methods
										ImportResult(OSType id, const CMediaTrackInfos& mediaTrackInfos) :
											mID(id), mMediaTrackInfos(mediaTrackInfos)
											{}
										ImportResult(const ImportResult& other) :
											mID(other.mID), mMediaTrackInfos(other.mMediaTrackInfos)
											{}

										// Instance methods
					OSType				getID() const
											{ return mID; }
			const	CMediaTrackInfos&	getMediaTrackInfos() const
											{ return mMediaTrackInfos; }

			// Properties
			private:
				OSType				mID;
				CMediaTrackInfos	mMediaTrackInfos;
		};

	// Methods
	public:
										// Instance methods
				void					registerMediaSource(const SMediaSource& mediaSource);
		const	SMediaSource&			getMediaSource(OSType id) const;
				TIResult<ImportResult>	import(const I<CRandomAccessDataSource>& randomAccessDataSource,
												const CString& extension,
												const OI<CAppleResourceManager>& appleResourceManager,
												SMediaSource::Options options = SMediaSource::kNone) const;

	private:
										// Lifecycle methods
										CMediaSourceRegistry();

	// Properties
	public:
		static	CMediaSourceRegistry	mShared;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Macros

#define REGISTER_MEDIA_SOURCE(mediaSourceCodedName, info)																\
	class mediaSourceCodedName##MediaSourceRegisterer {																	\
		public:																											\
			mediaSourceCodedName##MediaSourceRegisterer() { CMediaSourceRegistry::mShared.registerMediaSource(info); }	\
	};																													\
	static	mediaSourceCodedName##MediaSourceRegisterer _##mediaSourceCodedName##MediaSourceRegisterer
