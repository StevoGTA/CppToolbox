//----------------------------------------------------------------------------------------------------------------------
//	NSString+C++.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

//----------------------------------------------------------------------------------------------------------------------
// MARK: NSString extension

@interface NSString (Cpp)

// MARK: Class methods

+ (NSString*) stringForCString:(const CString&) string;

// MARK: Instance methods

- (NSString*) stringByReplacingOccurrencesOfCString:(const CString&) target withCString:(const CString&) replacement;

@end

NS_ASSUME_NONNULL_END
