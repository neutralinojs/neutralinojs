
#import "macwindow.h"

@implementation MacWindow

- (id)initWithContentRect:(NSRect)contentRect
	styleMask:(NSUInteger)windowStyle
	backing:(NSBackingStoreType)bufferingType
	defer:(BOOL)deferCreation
{
	self = [super
		initWithContentRect:contentRect
        styleMask:windowStyle
		backing:bufferingType
		defer:deferCreation];
        
    if (self)
    {
        [self setOpaque:NO];
        [self setBackgroundColor:[NSColor clearColor]];
    }
    
	return self;
}

- (BOOL)canBecomeKeyWindow
{
	return YES;
}

- (BOOL)canBecomeMainWindow
{
	return YES;
}

@end
