
#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>

@interface MacWebView : WKWebView
{
}

@end

@interface MacWindow : NSWindow
{
}

- (id)initWithHiddenTitlebar:(BOOL)hideTitlebar hiddenButtons:(BOOL)hideButtons resizable:(BOOL)resizable;

- (void)setWidth:(int)width height:(int)height 
        minWidth:(int)minWidth minHeight:(int)minHeight 
        maxWidth:(int)maxWidth maxHeight:(int)maxHeight 
        resizable:(BOOL)resizable;

@end
