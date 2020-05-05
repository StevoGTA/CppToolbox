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
		static	TNArray<CData>			arrayOfDatasFrom(CFArrayRef arrayRef);
		static	TNArray<CDictionary>	arrayOfDictionariesFrom(CFArrayRef arrayRef);
		static	TNArray<CString>		arrayOfStringsFrom(CFArrayRef arrayRef);
		static	CFArrayRef				createArrayRefFrom(const TArray<CDictionary>& array);
		static	CFArrayRef				createArrayRefFrom(const TArray<CString>& array);

										// Data methods
		static	CData					dataFrom(CFDataRef dataRef);
		static	CFDataRef				createDataRefFrom(const CData& data);

										// Dictionary methods
		static	CDictionary				dictionaryFrom(CFDictionaryRef dictionaryRef);
		static	CFDictionaryRef			createDictionaryRefFrom(const CDictionary& dictionary);

										// String methods
		static	CFStringRef				createStringRefFrom(const CString& string);

										// FilesystemPath methods
		static	CFStringRef				createStringRefFrom(const CFilesystemPath& filesystemPath);
		static	CFURLRef				createURLRefFrom(const CFilesystemPath& filesystemPath, bool isFolder);
};
