
#import "macwindow.h"

// 000   000  00000000  0000000    000   000  000  00000000  000   000  
// 000 0 000  000       000   000  000   000  000  000       000 0 000  
// 000000000  0000000   0000000     000 000   000  0000000   000000000  
// 000   000  000       000   000     000     000  000       000   000  
// 00     00  00000000  0000000        0      000  00000000  00     00  

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
    
    // [self takeSnapshot];
    
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
        NSString *filePath = @"~/Desktop/neu.jpg"; // todo: make path configurable somehow
        
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

// 00     00   0000000    0000000  000   000  000  000   000  0000000     0000000   000   000  
// 000   000  000   000  000       000 0 000  000  0000  000  000   000  000   000  000 0 000  
// 000000000  000000000  000       000000000  000  000 0 000  000   000  000   000  000000000  
// 000 0 000  000   000  000       000   000  000  000  0000  000   000  000   000  000   000  
// 000   000  000   000   0000000  00     00  000  000   000  0000000     0000000   00     00  

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
            
            self.movableByWindowBackground = YES; // not sure if this is still needed or doing anything anymore
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

- (void)setWidth:(int)width height:(int)height 
        minWidth:(int)minWidth minHeight:(int)minHeight 
        maxWidth:(int)maxWidth maxHeight:(int)maxHeight 
        resizable:(BOOL)resizable
{
    [self setStyleMask:(resizable ? 
        [self styleMask] |  NSWindowStyleMaskResizable : 
        [self styleMask] & ~NSWindowStyleMaskResizable)];

    if (minWidth != -1 || minHeight != -1) {
        [self setContentMinSize:CGSizeMake(minWidth, minHeight)];
    }
    if (maxWidth != -1 || maxHeight != -1) {
        [self setContentMaxSize: CGSizeMake(maxWidth, maxHeight)];
    }
    if(width != -1 || height != -1) {
        [self setFrame:CGRectMake(0, 0, width, height) display:YES animate:NO];
        [self center];
    }
}

- (BOOL)isMovableByWindowBackground // not sure if this is still needed or doing anything anymore
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