//----------------------------------------------------------------------------------------------------------------------
//	SMediaSource.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAppleResourceManager.h"
#include "CAudioCodec.h"
#include "CDataSource.h"
#include "CVideoCodec.h"
#include "SMedia.h"
#include "SVideo.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SMediaSource

struct SMediaSource {
	// Options
	enum Options {
		kOptionsNone				= 0,
		kOptionsCreateDecoders		= 1 << 1,
		kOptionsImportVideoTracks	= 1 << 2,

		kOptionsLast				= kOptionsImportVideoTracks,
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
													UInt32 options) :
												mRandomAccessDataSource(randomAccessDataSource),
														mAppleResourceManager(appleResourceManager), mOptions(options)
												{}
											ImportSetup(const I<CRandomAccessDataSource>& randomAccessDataSource,
													UInt32 options) :
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
				bool						isCreatingDecoders() const
												{ return mOptions & kOptionsCreateDecoders; }
				bool						isImportingVideoTracks() const
												{ return mOptions & kOptionsImportVideoTracks; }

		// Properties
		private:
			I<CRandomAccessDataSource>	mRandomAccessDataSource;
			OI<CAppleResourceManager>	mAppleResourceManager;
			UInt32						mOptions;
	};

	// Track
	template <typename MTF, typename DC> struct Track {
											// Lifecycle methods
											Track(const MTF& mediaTrackFormat,
													const SMedia::SegmentInfo& mediaSegmentInfo,
													const I<DC>& decodeCodec) :
												mMediaTrackFormat(mediaTrackFormat),
														mMediaSegmentInfo(mediaSegmentInfo), mDecodeCodec(decodeCodec)
												{}
											Track(const MTF& mediaTrackFormat,
													const SMedia::SegmentInfo& mediaSegmentInfo,
													const OV<I<DC> >& decodeCodec = OV<I<DC> >()) :
												mMediaTrackFormat(mediaTrackFormat),
														mMediaSegmentInfo(mediaSegmentInfo), mDecodeCodec(decodeCodec)
												{}
											Track(const Track<MTF, DC>& other) :
												mMediaTrackFormat(other.mMediaTrackFormat),
														mMediaSegmentInfo(other.mMediaSegmentInfo),
														mDecodeCodec(other.mDecodeCodec)
												{}

											// Instance methods
			const	MTF&					getMediaTrackFormat() const
												{ return mMediaTrackFormat; }
			const	SMedia::SegmentInfo&	getMediaSegmentInfo() const
												{ return mMediaSegmentInfo; }
			const	OV<I<DC> >&				getDecodeCodec() const
												{ return mDecodeCodec; }

		// Properties
		private:
			MTF					mMediaTrackFormat;
			SMedia::SegmentInfo	mMediaSegmentInfo;
			OV<I<DC> >			mDecodeCodec;
	};

	// Tracks
	class Tracks {
		// Types
		public:
			typedef	Track<SAudio::Format, CDecodeAudioCodec>	AudioTrack;
			typedef	Track<SVideo::Format, CDecodeVideoCodec>	VideoTrack;

		// Methods
		public:

										// Lifecycle methods
										Tracks() {}
										Tracks(const Tracks& other) :
											mAudioTracks(other.mAudioTracks), mVideoTracks(other.mVideoTracks)
											{}

										// Instance methods
		const	TArray<AudioTrack>&		getAudioTracks() const
											{ return mAudioTracks; }
				void					add(const AudioTrack& audioTrack)
											{ mAudioTracks += audioTrack; }

		const	TArray<VideoTrack>&		getVideoTracks() const
											{ return mVideoTracks; }
				void					add(const VideoTrack& videoTrack)
											{ mVideoTracks += videoTrack; }

				UniversalTimeInterval	getDuration() const
											{
												// Compose total duration
												UniversalTimeInterval	duration = 0.0;
												for (TIteratorD<AudioTrack> iterator = mAudioTracks.getIterator();
														iterator.hasValue(); iterator.advance())
													// Update duration
													duration =
															std::max<UniversalTimeInterval>(duration,
																	iterator->getMediaSegmentInfo().getDuration());
												for (TIteratorD<VideoTrack> iterator = mVideoTracks.getIterator();
														iterator.hasValue(); iterator.advance())
													// Update duration
													duration =
															std::max<UniversalTimeInterval>(duration,
																	iterator->getMediaSegmentInfo().getDuration());

												return duration;
											}

		// Properties
		private:
			TNArray<AudioTrack>	mAudioTracks;
			TNArray<VideoTrack>	mVideoTracks;
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
									ImportResult(OSType mediaSourceID, const Tracks& tracks,
											const TArray<CString>& messages = TNArray<CString>()) :
										mResult(kSuccess), mMediaSourceID(mediaSourceID), mTracks(tracks),
												mMessages(messages)
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
		const	Tracks&				getTracks() const
										{ return *mTracks; }
		const	TArray<CString>&	getMessages() const
										{ return *mMessages; }
		const	SError&				getError() const
										{ return *mError; }

		// Properties
		private:
			Result					mResult;
			OV<OSType>				mMediaSourceID;
			OV<Tracks>				mTracks;
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
