//----------------------------------------------------------------------------------------------------------------------
//	SVersionInfo.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SVersionInfo

struct SVersionInfo {
	// Methods
	public:
							// Lifecycle methods
							SVersionInfo(UInt8 majorVersion, UInt8 minorVersion, UInt8 patchVersion) :
								mMajorVersion(majorVersion), mMinorVersion(minorVersion), mPatchVersion(patchVersion)
								{
									// Check patch version
									if (mPatchVersion == 0)
										// No patch version
										mString = CString(mMajorVersion) + CString(".") + CString(mMinorVersion);
									else
										// Have patch version
										mString =
												CString(mMajorVersion) + CString(".") + CString(mMinorVersion) +
														CString(".") + CString(mPatchVersion);
								}
							SVersionInfo(const CString& string) :
								mString(string), mMajorVersion(0), mMinorVersion(0), mPatchVersion(0)
								{}

							// Instance methods
		const	CString&	getString() const { return mString; }
				UInt8		getMajorVersion() { return mMajorVersion; }
				UInt8		getMinorVersion() { return mMinorVersion; }
				UInt8		getPatchVersion() { return mPatchVersion; }

	// Properties
	protected:
		CString	mString;

	private:
		UInt8	mMajorVersion;
		UInt8	mMinorVersion;
		UInt8	mPatchVersion;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SSystemVersionInfo

struct SSystemVersionInfo : SVersionInfo {
	// Methods
	public:
		// Lifecycle methods
		SSystemVersionInfo(const CString& name, UInt8 majorVersion, UInt8 minorVersion, UInt8 patchVersion,
				const CString& build) :
			SVersionInfo(majorVersion, minorVersion, patchVersion), mName(name), mBuild(build)
			{
				mString =
						(!mName.isEmpty() ? mName + CString::mSpaceCharacter : CString::mEmpty) +
						mString +
						(!mBuild.isEmpty() ? CString::mSpaceCharacter + CString("(") + mBuild + CString(")") :
								CString::mEmpty);
			}
		SSystemVersionInfo(const CString& string) : SVersionInfo(string) {}

	// Properties
	private:
		CString	mName;
		CString	mBuild;
};
