//----------------------------------------------------------------------------------------------------------------------
//	SVersionInfo.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SVersionInfo

struct SVersionInfo {
						// Lifecycle methods
						SVersionInfo(UInt8 majorVersion, UInt8 minorVersion, UInt8 patchVersion) :
							mMajorVersion(majorVersion), mMinorVersion(minorVersion), mPatchVersion(patchVersion)
							{
								// Check patch version
								if (mPatchVersion == 0)
									// No patch version
									mString = CString(mMajorVersion) + CString(OSSTR(".")) + CString(mMinorVersion);
								else
									// Have patch version
									mString =
											CString(mMajorVersion) + CString(OSSTR(".")) + CString(mMinorVersion) +
													CString(OSSTR(".")) + CString(mPatchVersion);
							}
						SVersionInfo(UInt32 combinedVersion) :
							mMajorVersion((combinedVersion >> 16) & 0xFF), mMinorVersion((combinedVersion >> 8) & 0xFF),
									mPatchVersion((combinedVersion >> 0) & 0xFF)
							{
								// Check patch version
								if (mPatchVersion == 0)
									// No patch version
									mString = CString(mMajorVersion) + CString(OSSTR(".")) + CString(mMinorVersion);
								else
									// Have patch version
									mString =
											CString(mMajorVersion) + CString(OSSTR(".")) + CString(mMinorVersion) +
													CString(OSSTR(".")) + CString(mPatchVersion);
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
	// Lifecycle methods
	SSystemVersionInfo(const CString& name, UInt8 majorVersion, UInt8 minorVersion, UInt8 patchVersion,
			const CString& build) :
		SVersionInfo(majorVersion, minorVersion, patchVersion), mName(name), mBuild(build)
		{
			mString =
					(!mName.isEmpty() ? mName + CString::mSpace : CString::mEmpty) +
					mString +
					(!mBuild.isEmpty() ?
							CString::mSpace + CString(OSSTR("(")) + mBuild + CString(OSSTR(")")) : CString::mEmpty);
		}
	SSystemVersionInfo(const CString& string) : SVersionInfo(string) {}

	// Properties
	private:
		CString	mName;
		CString	mBuild;
};
