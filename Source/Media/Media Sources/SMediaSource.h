//----------------------------------------------------------------------------------------------------------------------
//	SMediaSource.h			©2021 Stevo Brock	All rights reserved.
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

		kOptionsLast			= kOptionsCreateDecoders,
	};

	// ImportSetup
	struct ImportSetup {
											// Lifecycle methods
											ImportSetup(const I<CRandomAccessDataSource>& randomAccessDataSource,
													const OI<CAppleResourceManager>& appleResourceManager,
													UInt32 options = SMediaSource::kOptionsNone) :
												mRandomAccessDataSource(randomAccessDataSource),
														mAppleResourceManager(appleResourceManager), mOptions(options)
												{}
											ImportSetup(const I<CRandomAccessDataSource>& randomAccessDataSource,
													UInt32 options = SMediaSource::kOptionsNone) :
												mRandomAccessDataSource(randomAccessDataSource), mOptions(options)
												{}

											// Instance methods
		const	I<CRandomAccessDataSource>&	getRandomAccessDataSource() const
												{ return mRandomAccessDataSource; }
		const	OI<CAppleResourceManager>	getAppleResourceManager() const
												{ return mAppleResourceManager; }
				UInt32						getOptions() const
												{ return mOptions; }

		// Properties
		private:
			I<CRandomAccessDataSource>	mRandomAccessDataSource;
			OI<CAppleResourceManager>	mAppleResourceManager;
			UInt32						mOptions;
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
									ImportResult(const CMediaTrackInfos& mediaTrackInfos,
											const TArray<CString>& messages) :
										mResult(kSuccess), mMediaTrackInfos(mediaTrackInfos), mMessages(messages)
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
		const	TArray<CString>&	getMessages() const
										{ return *mMessages; }
		const	SError&				getError() const
										{ return *mError; }

		// Properties
		private:
			Result					mResult;
			OV<CMediaTrackInfos>	mMediaTrackInfos;
			OV<TArray<CString> >	mMessages;
			OV<SError>				mError;
	};

	// Procs
	typedef	I<ImportResult>		(*ImportProc)(const ImportSetup& importSetup);

							// Lifecycle methods
							SMediaSource(OSType id, const CString& name, const TSet<CString>& extensions,
									ImportProc importProc) :
								mID(id), mName(name), mExtensions(extensions), mImportProc(importProc)
								{}
							SMediaSource(const SMediaSource& other) :
								mID(other.mID), mName(other.mName), mExtensions(other.mExtensions),
										mImportProc(other.mImportProc)
								{}

							// Instance methods
			OSType			getID() const
								{ return mID; }
	const	CString&		getName() const
								{ return mName; }
	const	TSet<CString>&	getExtensions() const
								{ return mExtensions; }
			I<ImportResult>	import(const ImportSetup& importSetup) const
								{ return mImportProc(importSetup); }

	// Properties
	private:
		OSType			mID;
		CString			mName;
		TNSet<CString>	mExtensions;
		ImportProc		mImportProc;
};