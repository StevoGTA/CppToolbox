//----------------------------------------------------------------------------------------------------------------------
//	CAppleResourceManager.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataSource.h"
#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAppleResourceManager

class CAppleResource;
class CAppleResourceManagerInternals;
class CAppleResourceManager {
	// Methods
	public:
												// Lifecycle methods
												~CAppleResourceManager();

												// Instance methods
				OR<CData>						get(OSType resourceType, UInt16 resourceID) const;
				OI<CString>						getPascalString(OSType resourceType, UInt16 resourceID) const;

//				void							set(OSType resourceType, UInt16 resourceID, const CString& name,
//														const CData& data);
//				void							set(OSType resourceType, UInt16 resourceID, const CString& name,
//														const CString& pascalString);
//
//				OI<SError>						write();

												// Class methods
		static	TIResult<CAppleResourceManager>	from(const I<CRandomAccessDataSource>& randomAccessDataSource);

	private:
												// Lifecycle methods
												CAppleResourceManager(
														const TNDictionary<TNArray<CAppleResource> >& resourceMap);

	// Properties
	private:
		CAppleResourceManagerInternals*	mInternals;
};