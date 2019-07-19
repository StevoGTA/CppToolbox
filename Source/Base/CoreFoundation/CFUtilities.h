//----------------------------------------------------------------------------------------------------------------------
//	CFUtilities.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"
#include "CFilesystemPath.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Array utilities

extern	TArray<CData>		eArrayOfDatasFrom(CFArrayRef arrayRef);
extern	TArray<CDictionary>	eArrayOfDictionariesFrom(CFArrayRef arrayRef);
extern	CFArrayRef			eArrayCopyCFArrayRef(const TArray<CDictionary>& array);

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
