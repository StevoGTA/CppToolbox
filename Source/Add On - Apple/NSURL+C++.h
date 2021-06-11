//----------------------------------------------------------------------------------------------------------------------
//	NSURL+C++.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SFoldersFiles.h"

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

//----------------------------------------------------------------------------------------------------------------------
// MARK: NSURL extension

@interface NSURL (Cpp)

// MARK: Properties

@property (nonatomic, readonly)	CFilesystemPath filesystemPath;

// MARK: Class methods

+ (SFoldersFiles) foldersFilesFor:(NSArray<NSURL*>*) urls;
+ (TArray<CFolder>) foldersFor:(NSArray<NSURL*>*) urls;
+ (TArray<CFile>) filesFor:(NSArray<NSURL*>*) urls;

@end

NS_ASSUME_NONNULL_END
