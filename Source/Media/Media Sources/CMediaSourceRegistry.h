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
		kOptionsNone			= 0,
		kOptionsCreateDecoders	= 1 << 1,

		kOptionsLast = kOptionsCreateDecoders,
	};

	// ImportResult
	class ImportResult {
		// Result
		public:
			enum Result {
				kSuccess,
				kSourceMatchButUnableToLoad,
				kSourceMismatch,
			};

		// Methods:
		public:

									// Lifecycle methods
									ImportResult(const CMediaTrackInfos& mediaTrackInfos) :
										mResult(kSuccess), mMediaTrackInfos(mediaTrackInfos)
										{}
									ImportResult(const SError& error) :
										mResult(kSourceMatchButUnableToLoad), mError(error)
										{}
									ImportResult() : mResult(kSourceMismatch) {}

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
			OV<CMediaTrackInfos>	mMediaTrackInfos;
			OV<SError>				mError;
	};

	// Procs
	typedef	I<ImportResult>		(*ImportProc)(const I<CRandomAccessDataSource>& randomAccessDataSource,
										const OI<CAppleResourceManager>& appleResourceManager, UInt32 options);

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
			I<ImportResult>		import(const I<CRandomAccessDataSource>& randomAccessDataSource,
										const OI<CAppleResourceManager>& appleResourceManager, UInt32 options) const
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
				TVResult<ImportResult>	import(const I<CRandomAccessDataSource>& randomAccessDataSource,
												const CString& extension,
												const OI<CAppleResourceManager>& appleResourceManager,
												UInt32 options = SMediaSource::kOptionsNone) const;

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
