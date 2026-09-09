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
							SVersionInfo(const CString& name, UInt8 majorVersion, UInt8 minorVersion,
									UInt8 patchVersion, const CString& build) :
								mMajorVersion(majorVersion), mMinorVersion(minorVersion), mPatchVersion(patchVersion),
										mString(
												(!name.isEmpty() ? name + CString::mSpace : CString::mEmpty) +
												((mPatchVersion == 0) ?
														CString(mMajorVersion) + CString(OSSTR(".")) +
																CString(mMinorVersion) :
														CString(mMajorVersion) + CString(OSSTR(".")) +
																CString(mMinorVersion) + CString(OSSTR(".")) +
																CString(mPatchVersion)) +
												(!build.isEmpty() ?
														CString::mSpace + CString(OSSTR("(")) + build +
																CString(OSSTR(")")) :
														CString::mEmpty))
								{}
							SVersionInfo(UInt8 majorVersion, UInt8 minorVersion, UInt8 patchVersion) :
								mMajorVersion(majorVersion), mMinorVersion(minorVersion), mPatchVersion(patchVersion),
										mString(
												(mPatchVersion == 0) ?
														CString(mMajorVersion) + CString(OSSTR(".")) +
																CString(mMinorVersion) :
														CString(mMajorVersion) + CString(OSSTR(".")) +
																CString(mMinorVersion) + CString(OSSTR(".")) +
																CString(mPatchVersion))
								{}
							SVersionInfo(UInt32 combinedVersion) :
								mMajorVersion((combinedVersion >> 16) & 0xFF),
										mMinorVersion((combinedVersion >> 8) & 0xFF),
										mPatchVersion((combinedVersion >> 0) & 0xFF),
										mString(
												(mPatchVersion == 0) ?
														CString(mMajorVersion) + CString(OSSTR(".")) +
																CString(mMinorVersion) :
														CString(mMajorVersion) + CString(OSSTR(".")) +
																CString(mMinorVersion) + CString(OSSTR(".")) +
																CString(mPatchVersion))
								{}
							SVersionInfo(const CString& string) :
								mString(string), mMajorVersion(0), mMinorVersion(0), mPatchVersion(0)
								{}

							// Instance methods
				UInt8		getMajorVersion() const
								{ return mMajorVersion; }
				UInt8		getMinorVersion() const
								{ return mMinorVersion; }
				UInt8		getPatchVersion() const
								{ return mPatchVersion; }

		const	CString&	getString() const
								{ return mString; }

	// Properties
	private:
		UInt8	mMajorVersion;
		UInt8	mMinorVersion;
		UInt8	mPatchVersion;

		CString	mString;
};
