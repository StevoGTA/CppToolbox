//----------------------------------------------------------------------------------------------------------------------
//	CAppleResourceManager.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataSource.h"
#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAppleResourceManager

class CAppleResourceManager {
	// Classes
	private:
		class Resource;
		class Internals;

	// Methods
	public:
												// Lifecycle methods
												CAppleResourceManager();
												~CAppleResourceManager();

												// Instance methods
				OR<CData>						get(OSType resourceType, UInt16 resourceID) const;
				OV<CString>						getPascalString(OSType resourceType, UInt16 resourceID) const;

				void							set(OSType resourceType, UInt16 resourceID, const CString& name,
														const CData& data);
				void							set(OSType resourceType, UInt16 resourceID, const CString& name,
														const CString& pascalString);

				CData							getAsData();

												// Class methods
		static	TIResult<CAppleResourceManager>	from(const I<CRandomAccessDataSource>& randomAccessDataSource);

	private:
												// Lifecycle methods
												CAppleResourceManager(
														const TNDictionary<TNArray<Resource> >& resourceMap);

	// Properties
	private:
		Internals*	mInternals;
};
