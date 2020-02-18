//----------------------------------------------------------------------------------------------------------------------
//	CFUtilities.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"
#include "CFilesystemPath.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Array utilities

extern	TNArray<CData>			eArrayOfDatasFrom(CFArrayRef arrayRef);
extern	TNArray<CDictionary>	eArrayOfDictionariesFrom(CFArrayRef arrayRef);
extern	TNArray<CString>		eArrayOfStringsFrom(CFArrayRef arrayRef);
extern	CFArrayRef				eArrayCopyCFArrayRef(const TArray<CDictionary>& array);
extern	CFArrayRef				eArrayCopyCFArrayRef(const TArray<CString>& array);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Data utilities

extern	CData		eDataFrom(CFDataRef dataRef);
extern	CFDataRef	eDataCopyCFDataRef(const CData& data);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Dictionary utilities

extern	CDictionary		eDictionaryFrom(CFDictionaryRef dictionaryRef);
extern	CFDictionaryRef	eDictionaryCopyCFDictionaryRef(const CDictionary& dictionary);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - String utilities

extern	CFStringRef	eStringCopyCFStringRef(const CString& string);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - FilesystemPath utilities

extern	CFStringRef	eFilesystemPathCopyStringRef(const CFilesystemPath& filesystemPath);
extern	CFURLRef	eFilesystemPathCopyURLRef(const CFilesystemPath& filesystemPath, bool isFolder);
