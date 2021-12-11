//----------------------------------------------------------------------------------------------------------------------
//	IStorageItem+Helpers.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SFoldersFiles.h"

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.h>

using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Storage;

//----------------------------------------------------------------------------------------------------------------------
// MARK: Global methods

SFoldersFiles	IStorageItemGetFoldersFiles(const IVectorView<IStorageItem>& storageItems);
