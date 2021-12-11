//----------------------------------------------------------------------------------------------------------------------
//	IStorageItem+Helpers.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "IStorageItem+Helpers.h"

// MARK: Global methods

//----------------------------------------------------------------------------------------------------------------------
SFoldersFiles IStorageItemGetFoldersFiles(const IVectorView<IStorageItem>& storageItems)
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose folders and files
	TNArray<CFolder>	folders;
	TNArray<CFile>		files;
	for (auto const& storageItem : storageItems) {
		// Check if directory
		if ((uint32_t) storageItem.Attributes() & (uint32_t) FileAttributes::Directory)
			// Folder
			folders += CFolder(CFilesystemPath(CString(storageItem.Path().data())));
		else
			// File
			files += CFile(CFilesystemPath(CString(storageItem.Path().data())));
	}

	return SFoldersFiles(folders, files);
}
