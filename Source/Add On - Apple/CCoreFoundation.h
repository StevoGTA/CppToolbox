//----------------------------------------------------------------------------------------------------------------------
//	CCoreFoundation.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"
#include "CFilesystemPath.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreFoundation

class CCoreFoundation {
	// Methods
	public:
										// Array methods
		static	TArray<CData>			arrayOfDatasFrom(CFArrayRef arrayRef);
		static	TArray<CDictionary>		arrayOfDictionariesFrom(CFArrayRef arrayRef);
		static	TArray<CString>			arrayOfStringsFrom(CFArrayRef arrayRef);
		static	CFArrayRef				createArrayRefFrom(const TArray<CDictionary>& array);
		static	CFArrayRef				createArrayRefFrom(const TArray<CString>& array);

										// Data methods
		static	CData					dataFrom(CFDataRef dataRef);
		static	CFDataRef				createDataRefFrom(const CData& data);

										// Dictionary methods
		static	CDictionary				dictionaryFrom(CFDictionaryRef dictionaryRef);
		static	TDictionary<CString>	dictionaryOfStringsFrom(CFDictionaryRef dictionaryRef);
		static	CFDictionaryRef			createDictionaryRefFrom(const CDictionary& dictionary);
		static	CFDictionaryRef			createDictionaryRefFrom(const TDictionary<CString>& dictionary);

										// Set methods
		static	TSet<CString>			setOfStringsFrom(const CFSetRef setRef);

										// FilesystemPath methods
		static	CFURLRef				createURLRefFrom(const CFilesystemPath& filesystemPath, bool isFolder);
};
