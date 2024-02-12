
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

@end
