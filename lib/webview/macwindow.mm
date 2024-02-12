
#import "macwindow.h"

@implementation MacWebView

-(void)mouseDown:(NSEvent *)event 
{
    NSPoint   viewLoc = [self convertPoint:event.locationInWindow fromView:nil];
    NSString *docElem = [NSString stringWithFormat:@"document.elementFromPoint(%f, %f)", viewLoc.x, viewLoc.y];
    NSString *jsCode  = [NSString stringWithFormat:@"%@.classList.contains(\"app-drag-region\")", docElem];
    
    [self evaluateJavaScript:jsCode completionHandler:
        ^(id result, NSError * error) {
            if (error) NSLog(@"%@", error);
            else 
            {
                if ([[NSNumber numberWithInt:1] compare:result] == NSOrderedSame)
                {
                    [self.window performWindowDragWithEvent:event];
                }
            }
    }];
    
    [self takeSnapshot];
    
    [super mouseDown:event];
}

-(void)takeSnapshot
{
    WKSnapshotConfiguration * snapshotConfiguration = [[WKSnapshotConfiguration alloc] init];
    [self takeSnapshotWithConfiguration:snapshotConfiguration completionHandler:
        ^(NSImage * image, NSError * error) {
            [self snapshotTaken:image error:error];
    }];
}

-(void)snapshotTaken:(NSImage *)image error:(NSError *)error
{
    if (error) NSLog(@"%@", error);
    else
    {
        NSString *filePath = @"~/Desktop/neu.jpg";
        
        int number = 0;
        while ([[NSFileManager defaultManager] fileExistsAtPath:[filePath stringByExpandingTildeInPath]])
        {
            number++;
            filePath = [NSString stringWithFormat:@"~/Desktop/neu_%d.jpg", number];
        }
        
        NSData *imageData = [image TIFFRepresentation];
        NSBitmapImageRep *imageRep = [NSBitmapImageRep imageRepWithData:imageData];
        NSDictionary *imageProps = [NSDictionary dictionaryWithObject:[NSNumber numberWithFloat:1.0] forKey:NSImageCompressionFactor];
        imageData = [imageRep representationUsingType:NSJPEGFileType properties:imageProps];
        [imageData writeToFile:[filePath stringByExpandingTildeInPath] atomically:NO];        
    }
}

@end


@implementation MacWindow

- (id)initWithHiddenTitlebar:(BOOL)hideTitlebar hiddenButtons:(BOOL)hideButtons resizable:(BOOL)resizable
{
    NSUInteger styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskMiniaturizable;
    
    if (resizable)
    {
        styleMask |= NSWindowStyleMaskResizable;
    }
    
    if (hideTitlebar)
    {
        styleMask |= NSWindowStyleMaskFullSizeContentView;
    }
    
    self = [self 
        initWithContentRect:CGRectMake(0, 0, 0, 0)
        styleMask:          styleMask
        backing:            NSBackingStoreBuffered
        defer:              NO];
                
    if (self)
    {
        [self setOpaque:NO];
        [self setBackgroundColor:[NSColor clearColor]];
        
        if (hideTitlebar)
        {
            self.titleVisibility = NSWindowTitleHidden;
            self.titlebarAppearsTransparent = YES;
            
            self.movableByWindowBackground = YES;
        }
        
        if (hideButtons)
        {
            [self standardWindowButton:NSWindowCloseButton      ].hidden = YES;
            [self standardWindowButton:NSWindowMiniaturizeButton].hidden = YES;
            [self standardWindowButton:NSWindowZoomButton       ].hidden = YES;
        }        
    }
    
	return self;
}

- (BOOL)isMovableByWindowBackground 
{
    return YES;
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