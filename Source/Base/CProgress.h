//----------------------------------------------------------------------------------------------------------------------
//	CProgress.h			©2021 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CProgress

class CProgressInternals;
class CProgress {
	// UpdateInfo
	public:
		struct UpdateInfo {
			// Types
			typedef	void	(*Proc)(const CProgress& progress, void* userData);

					// Methods
					UpdateInfo(Proc proc, void* userData) :
						mProc(proc), mUserData(userData)
						{}
					UpdateInfo(const UpdateInfo& other) :
						mProc(other.mProc), mUserData(other.mUserData)
						{}

			void	notify(const CProgress& progress)
						{ mProc(progress, mUserData); }

			// Properties
			private:
				Proc	mProc;
				void*	mUserData;
		};

	// Methods
	public:
										// Lifecycle methods
										CProgress(const UpdateInfo& updateInfo);
										CProgress(const CProgress& other);
		virtual							~CProgress();

										// Instance methods
				const	CString&		getMessage() const;
						void			setMessage(const CString& message);

				const	OV<Float32>&	getValue() const;
						void			setValue(OV<Float32> value);

	// Properties
	private:
		CProgressInternals*	mInternals;
};
