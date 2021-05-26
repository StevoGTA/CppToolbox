//----------------------------------------------------------------------------------------------------------------------
//	CVideoCodec.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CCodec.h"
#include "CDataSource.h"
#include "CVideoFrame.h"
#include "SMediaPosition.h"
#include "TInstance.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CVideoCodec

class CVideoCodec : public CCodec {
	// DecodeFrameInfo
	public:
		struct DecodeFrameInfo {
			// Compatibility
			enum Compatibility {
#if TARGET_OS_IOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
				kCompatibilityAppleMetal,
				kCompatibilityAppleOpenGLES,
#elif TARGET_OS_MACOS
				kCompatibilityAppleMetal,
				kCompatibilityAppleOpenGL,
#else
				kCompatibilityNotApplicable,
#endif
			};

			// Procs
			typedef	void	(*FrameReadyProc)(const CVideoFrame& videoFrame, void* userData);
			typedef	void	(*ErrorProc)(const SError& error, void* userData);

							// Lifecycle methods
							DecodeFrameInfo(Compatibility compatibility, FrameReadyProc frameReadyProc,
									ErrorProc errorProc, void* userData) :
								mCompatibility(compatibility), mFrameReadyProc(frameReadyProc), mErrorProc(errorProc),
										mUserData(userData)
								{}
							DecodeFrameInfo(const DecodeFrameInfo& other) :
								mCompatibility(other.mCompatibility), mFrameReadyProc(other.mFrameReadyProc),
										mErrorProc(other.mErrorProc), mUserData(other.mUserData)
								{}

							// Instance methods
			Compatibility	getCompatibility() const
								{ return mCompatibility; }
			void			frameReady(const CVideoFrame& videoFrame) const
								{ mFrameReadyProc(videoFrame, mUserData); }
			void			error(const SError& error) const
								{ mErrorProc(error, mUserData); }

			// Properties
			private:
				Compatibility	mCompatibility;
				FrameReadyProc	mFrameReadyProc;
				ErrorProc		mErrorProc;
				void*			mUserData;
		};

	// Info
	public:
		struct Info {
			// Procs
			typedef	I<CVideoCodec>	(*InstantiateProc)(OSType id);

									// Lifecycle methods
									Info(OSType id, const CString& name, InstantiateProc instantiateProc) :
										mID(id), mDecodeName(name), mInstantiateProc(instantiateProc)
										{}
									Info(const Info& other) :
										mID(other.mID), mDecodeName(other.mDecodeName),
												mInstantiateProc(other.mInstantiateProc)
										{}

									// Instance methods
			OSType					getID() const
										{ return mID; }
			const	CString&		getDecodeName() const
										{ return mDecodeName; }
					I<CVideoCodec>	instantiate() const
										{ return mInstantiateProc(mID); }

			// Properties
			private:
				OSType			mID;
				CString			mDecodeName;
				InstantiateProc	mInstantiateProc;
		};

	// Methods
	public:
							// Lifecycle methods
							CVideoCodec() : CCodec() {}
		virtual				~CVideoCodec() {}

							// Instance methods
		virtual	void		setupForDecode(const I<CDataSource>& dataSource, const I<CCodec::DecodeInfo>& decodeInfo,
									const DecodeFrameInfo& decodeFrameInfo) = 0;
		virtual	bool		triggerDecode() = 0;
		virtual	OI<SError>	set(const SMediaPosition& mediaPosition) = 0;
		virtual	OI<SError>	reset() = 0;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDecodeOnlyVideoCodec

class CDecodeOnlyVideoCodec : public CVideoCodec {
	// Methods
	public:
		// Lifecycle methods
		CDecodeOnlyVideoCodec() : CVideoCodec() {}
};
