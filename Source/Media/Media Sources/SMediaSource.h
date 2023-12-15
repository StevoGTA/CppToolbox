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

	// Identity
	struct Identity {
				// Lifecycle methods
				Identity(OSType id, const CString& nameLocalizationGroup, const CString& nameLocalizationKey) :
					mID(id), mNameLocalizationGroup(nameLocalizationGroup), mNameLocalizationKey(nameLocalizationKey)
					{}
				Identity(OSType id, const CString& name) : mID(id), mName(name) {}
				Identity(const Identity& other) :
					mID(other.mID), mName(other.mName), mNameLocalizationGroup(other.mNameLocalizationGroup),
							mNameLocalizationKey(other.mNameLocalizationKey)
					{}

		OSType	getID() const
					{ return mID; }
		CString	getName() const
					{ return mName.hasValue() ? *mName : CString(*mNameLocalizationGroup, *mNameLocalizationKey); }

		// Properties
		private:
			OSType		mID;
			OV<CString>	mName;
			OV<CString>	mNameLocalizationGroup;
			OV<CString>	mNameLocalizationKey;
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
											ImportSetup(const ImportSetup& other) :
												mRandomAccessDataSource(other.mRandomAccessDataSource),
														mAppleResourceManager(other.mAppleResourceManager),
														mOptions(other.mOptions)
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
									ImportResult(OSType mediaSourceID, const CMediaTrackInfos& mediaTrackInfos,
											const TArray<CString>& messages = TNArray<CString>()) :
										mResult(kSuccess), mMediaSourceID(mediaSourceID),
												mMediaTrackInfos(mediaTrackInfos), mMessages(messages)
										{}
									ImportResult(const SError& error) :
										mResult(kSourceMatchButUnableToLoad), mError(error)
										{}
									ImportResult() : mResult(kSourceMismatch) {}

									// Instance methods
		const	Result				getResult() const
										{ return mResult; }
				OSType				getMediaSourceID() const
										{ return *mMediaSourceID; }
		const	CMediaTrackInfos&	getMediaTrackInfos() const
										{ return *mMediaTrackInfos; }
		const	TArray<CString>&	getMessages() const
										{ return *mMessages; }
		const	SError&				getError() const
										{ return *mError; }

		// Properties
		private:
			Result					mResult;
			OV<OSType>				mMediaSourceID;
			OV<CMediaTrackInfos>	mMediaTrackInfos;
			OV<TArray<CString> >	mMessages;
			OV<SError>				mError;
	};

	// Procs
	typedef	I<ImportResult>		(*ImportProc)(const ImportSetup& importSetup);

								// Lifecycle methods
								SMediaSource(const TArray<Identity>& identities, const TArray<CString>& extensions,
										ImportProc importProc) :
									mIdentities(identities), mExtensions(extensions), mImportProc(importProc)
									{}
								SMediaSource(const Identity& identity, const TArray<CString>& extensions,
										ImportProc importProc) :
									mIdentities(identity), mExtensions(extensions), mImportProc(importProc)
									{}
								SMediaSource(const SMediaSource& other) :
									mIdentities(other.mIdentities), mExtensions(other.mExtensions),
											mImportProc(other.mImportProc)
									{}

								// Instance methods
	const	TArray<Identity>&	getIdentities() const
									{ return mIdentities; }
	const	TSet<CString>&		getExtensions() const
									{ return mExtensions; }
			I<ImportResult>		import(const ImportSetup& importSetup) const
									{ return mImportProc(importSetup); }

	// Properties
	private:
		TNArray<Identity>	mIdentities;
		TNSet<CString>		mExtensions;
		ImportProc			mImportProc;
};
