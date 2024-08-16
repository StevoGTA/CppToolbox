//----------------------------------------------------------------------------------------------------------------------
//	MetalBufferCache.m			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#import "MetalBufferCache.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: MetalBufferCache

@interface MetalBufferCache ()

@property (nonatomic, assign)	id<MTLDevice>													device;

@property (nonatomic, strong)	NSMutableDictionary<NSNumber*, NSMutableArray<id<MTLBuffer>>*>* availableBuffers;
@property (nonatomic, strong)	NSMutableDictionary<NSNumber*, NSMutableArray<id<MTLBuffer>>*>* inUseBuffers;

@end

@implementation MetalBufferCache

// MARK: Properties

@synthesize availableBuffers;
@synthesize inUseBuffers;

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
- (instancetype) initWithDevice:(id<MTLDevice>) device
{
	// Do super
	self = [super init];
	if (self != nil) {
		// Store
		self.device = device;

		// Setup
		self.availableBuffers = [NSMutableDictionary dictionary];
		self.inUseBuffers = [NSMutableDictionary dictionary];
	}

	return self;
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
- (id<MTLBuffer>) bufferWithLength:(NSUInteger) length options:(MTLResourceOptions) options
{
	// Retrieve existing buffer or create new
	NSMutableArray<id<MTLBuffer>>*	array = self.availableBuffers[@(length)];
	id<MTLBuffer>					buffer = nil;
	if ((array != nil) && (array.count > 0)) {
		// Have existing buffer
		buffer = array[0];
		[array removeObjectAtIndex:0];
	} else
		// Don't have existing buffer
		buffer = [self.device newBufferWithLength:length options:options];

	// Add to in-use
	array = self.inUseBuffers[@(length)];
	if (array != nil)
		// Have array
		[array addObject:buffer];
	else
		// Don't have array
		self.inUseBuffers[@(length)] = [NSMutableArray arrayWithObject:buffer];

	return buffer;
}

//----------------------------------------------------------------------------------------------------------------------
- (id<MTLBuffer>) bufferWithBytes:(const void*) pointer length:(NSUInteger) length options:(MTLResourceOptions) options
{
	// Get buffer
	id<MTLBuffer>	buffer = [self bufferWithLength:length options:options];

	// Copy data
	memcpy(buffer.contents, pointer, length);

	return buffer;
}

//----------------------------------------------------------------------------------------------------------------------
- (void) reset
{
	// Iterate all in-use keys
	for (NSNumber* key in self.inUseBuffers.allKeys) {
		// Retrieve from in-use
		NSMutableArray<id<MTLBuffer>>*	theInUseBuffers = self.inUseBuffers[key];
		self.inUseBuffers[key] = nil;

		// Retrieve from available
		NSMutableArray<id<MTLBuffer>>*	theAvailableBuffers = self.availableBuffers[key];
		if (theAvailableBuffers != nil)
			// Append
			[theAvailableBuffers addObjectsFromArray:theInUseBuffers];
		else
			// Store
			self.availableBuffers[key] = theInUseBuffers;
	}
}

@end
