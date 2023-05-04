//----------------------------------------------------------------------------------------------------------------------
//	MetalBufferCache.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#import <Metal/Metal.h>

NS_ASSUME_NONNULL_BEGIN

//----------------------------------------------------------------------------------------------------------------------
// MARK: MetalBufferCache

@interface MetalBufferCache : NSObject

// Lifecycle methods

- (instancetype) initWithDevice:(id<MTLDevice>) device;

// Instance methods

- (id<MTLBuffer>) bufferWithLength:(NSUInteger) length options:(MTLResourceOptions) options;
- (id<MTLBuffer>) bufferWithBytes:(const void*) pointer length:(NSUInteger) length options:(MTLResourceOptions) options;
- (void) reset;

@end

NS_ASSUME_NONNULL_END
