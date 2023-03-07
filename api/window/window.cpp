#include <string>
#include <iostream>

#if defined(__linux__) || defined(__FreeBSD__)
#include <gtk/gtk.h>
#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#elif defined(__APPLE__)
#include <objc/objc-runtime.h>
#include <CoreFoundation/Corefoundation.h>
#include <CoreGraphics/CGDisplayConfiguration.h>
#include <CoreGraphics/CGWindow.h>
#define NSBaseWindowLevel 0
#define NSFloatingWindowLevel 5
#define NSWindowStyleMaskFullScreen 16384

#elif defined(_WIN32)
#define _WINSOCKAPI_
#include <windows.h>
#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")
#pragma comment(lib, "WebView2Loader.dll.lib")
#endif

#include "lib/json/json.hpp"
#include "lib/webview/webview.h"
#include "resources.h"
#include "helpers.h"
#include "errors.h"
#include "server/neuserver.h"
#include "auth/permission.h"
#include "api/app/app.h"
#include "api/window/window.h"
#include "api/events/events.h"
#include "api/filesystem/filesystem.h"

using namespace std;
using json = nlohmann::json;
#if defined(_WIN32)
using namespace Gdiplus;
#endif

namespace window {

webview::webview *nativeWindow;
#if defined(__linux__) || defined(__FreeBSD__)
bool isGtkWindowFullScreen = false;

#elif defined(_WIN32)
bool isWinWindowFullScreen = false;
DWORD savedStyle;
DWORD savedStyleX;
RECT savedRect;
#endif

window::WindowOptions windowProps;
NEU_W_HANDLE windowHandle;

namespace handlers {

void windowStateChange(int state) {
    #if defined(__linux__) || defined(__FreeBSD__)
        isGtkWindowFullScreen = state == WEBVIEW_WINDOW_FULLSCREEN;
    #endif
    switch(state) {
        case WEBVIEW_WINDOW_CLOSE:
            if(windowProps.exitProcessOnClose ||
                !neuserver::isInitialized() || !permission::hasAPIAccess()) {
                app::exit();
            }
            else {
                events::dispatch("windowClose", nullptr);
            }
            break;
        case WEBVIEW_WINDOW_FOCUS:
            events::dispatch("windowFocus", nullptr);
            break;
        case WEBVIEW_WINDOW_BLUR:
            events::dispatch("windowBlur", nullptr);
    }
}

} // namespace handlers

NEU_W_HANDLE getWindowHandle() {
    return windowHandle;
}

void executeJavaScript(const string &js) {
    if(nativeWindow)
        nativeWindow->eval(js);
}

bool isMaximized() {
    json output;
    #if defined(__linux__) || defined(__FreeBSD__)
    return gtk_window_is_maximized(GTK_WINDOW(windowHandle)) == 1;
    #elif defined(_WIN32)
    return  IsZoomed(windowHandle) == 1;
    #elif defined(__APPLE__)
    return ((bool (*)(id, SEL, id))objc_msgSend)((id) windowHandle,
        "isZoomed"_sel, NULL);
    #endif
}

void maximize() {
    if(window::isMaximized())
        return;
    #if defined(__linux__) || defined(__FreeBSD__)
    gtk_window_maximize(GTK_WINDOW(windowHandle));
    #elif defined(_WIN32)
    ShowWindow(windowHandle, SW_MAXIMIZE);
    #elif defined(__APPLE__)
    ((void (*)(id, SEL, id))objc_msgSend)((id) windowHandle,
        "zoom:"_sel, NULL);
    #endif
}

void unmaximize() {
    if(!window::isMaximized())
        return;
    #if defined(__linux__) || defined(__FreeBSD__)
    gtk_window_unmaximize(GTK_WINDOW(windowHandle));
    #elif defined(_WIN32)
    ShowWindow(windowHandle, SW_RESTORE);
    #elif defined(__APPLE__)
    ((void (*)(id, SEL, id))objc_msgSend)((id) windowHandle,
        "zoom:"_sel, NULL);
    #endif
}

bool isVisible() {
    #if defined(__linux__) || defined(__FreeBSD__)
    return gtk_widget_is_visible(windowHandle) == 1;
    #elif defined(__APPLE__)
    return ((bool (*)(id, SEL, id))objc_msgSend)((id) windowHandle,
        "isVisible"_sel, NULL);
    return true;
    #elif defined(_WIN32)
    return IsWindowVisible(windowHandle) == 1;
    #endif
}

bool isFakeHidden() {
	//ensureOnScreen
	#if defined(_WIN32)
	RECT rect = { NULL };
	
	if(GetWindowRect( windowHandle, &rect)) {
		return rect.left > 9999;
	}
	#endif
	return false;
}

void undoFakeHidden() {
	#if defined(_WIN32)
	ShowWindow(windowHandle, SW_HIDE);
	SetWindowLong(windowHandle, GWL_EXSTYLE, nativeWindow->m_originalStyleEx);
	SetWindowPos(windowHandle, NULL, 10, 10, 0, 0, 0x0004 | 0x0001);
	ShowWindow(windowHandle, SW_SHOW);
	#endif
}

void show() {
    if(window::isVisible())
        return;
    #if defined(__linux__) || defined(__FreeBSD__)
    gtk_widget_show(windowHandle);
    #elif defined(__APPLE__)
    ((void (*)(id, SEL, bool))objc_msgSend)((id) windowHandle,
                "setIsVisible:"_sel, true);
    #elif defined(_WIN32)
    ShowWindow(windowHandle, SW_SHOW);
    if (window::isFakeHidden())
		window::undoFakeHidden();
    #endif
}

void hide() {
    if(!window::isVisible())
        return;
    #if defined(__linux__) || defined(__FreeBSD__)
    gtk_widget_hide(windowHandle);
    #elif defined(__APPLE__)
    ((void (*)(id, SEL, bool))objc_msgSend)((id) windowHandle,
                "setIsVisible:"_sel, false);
    #elif defined(_WIN32)
    ShowWindow(windowHandle, SW_HIDE);
    #endif
}

bool isFullScreen() {
    json output;
    #if defined(__linux__) || defined(__FreeBSD__)
    return isGtkWindowFullScreen;
    #elif defined(__APPLE__)
    unsigned long windowStyleMask = ((unsigned long (*)(id, SEL))objc_msgSend)(
        (id) windowHandle, "styleMask"_sel);
    return (windowStyleMask & NSWindowStyleMaskFullScreen) == NSWindowStyleMaskFullScreen;
    #elif defined(_WIN32)
    return isWinWindowFullScreen;
    #endif
}

void setFullScreen() {
    if(window::isFullScreen())
        return;
    #if defined(__linux__) || defined(__FreeBSD__)
    gtk_window_fullscreen(GTK_WINDOW(windowHandle));
    #elif defined(__APPLE__)
    ((void (*)(id, SEL, id))objc_msgSend)((id) windowHandle,
            "toggleFullScreen:"_sel, NULL);
    #elif defined(_WIN32)
    savedStyle = GetWindowLong(windowHandle, GWL_STYLE);
    savedStyleX = GetWindowLong(windowHandle, GWL_EXSTYLE);
    GetWindowRect(windowHandle, &savedRect);

    MONITORINFO monitor_info;
    DWORD newStyle = savedStyle & ~(WS_CAPTION | WS_THICKFRAME);
    DWORD newStyleX = savedStyleX & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE |
                        WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
    SetWindowLong(windowHandle, GWL_STYLE, newStyle);
    SetWindowLong(windowHandle, GWL_EXSTYLE, newStyleX);
    monitor_info.cbSize = sizeof(monitor_info);
    GetMonitorInfo(MonitorFromWindow(windowHandle, MONITOR_DEFAULTTONEAREST),
                &monitor_info);
    RECT r;
    r.left = monitor_info.rcMonitor.left;
    r.top = monitor_info.rcMonitor.top;
    r.right = monitor_info.rcMonitor.right;
    r.bottom = monitor_info.rcMonitor.bottom;
    SetWindowPos(windowHandle, NULL, r.left, r.top, r.right - r.left,
                r.bottom - r.top,
                SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    isWinWindowFullScreen = true;
    #endif
}

void exitFullScreen() {
    if(!window::isFullScreen())
        return;
    #if defined(__linux__) || defined(__FreeBSD__)
    gtk_window_unfullscreen(GTK_WINDOW(windowHandle));
    #elif defined(__APPLE__)
    ((void (*)(id, SEL, id))objc_msgSend)((id) windowHandle,
            "toggleFullScreen:"_sel, NULL);
    #elif defined(_WIN32)
    SetWindowLong(windowHandle, GWL_STYLE, savedStyle);
    SetWindowLong(windowHandle, GWL_EXSTYLE, savedStyleX);
    SetWindowPos(windowHandle, NULL, savedRect.left, savedRect.top, savedRect.right - savedRect.left,
                savedRect.bottom - savedRect.top,
                SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    isWinWindowFullScreen = false;
    #endif
}

void setIcon(const string &iconFile) {
    fs::FileReaderResult fileReaderResult = resources::getFile(iconFile);
    string iconDataStr = fileReaderResult.data;
    #if defined(__linux__) || defined(__FreeBSD__)
    GdkPixbuf *icon = nullptr;
    GdkPixbufLoader *loader;
    GdkPixbuf *pixbuf;
    loader = gdk_pixbuf_loader_new();

    const char *iconData = iconDataStr.c_str();
    unsigned char *uiconData = reinterpret_cast <unsigned char *> (const_cast <char *> (iconData));
    gdk_pixbuf_loader_write(loader, uiconData, iconDataStr.length(), NULL);
    icon = gdk_pixbuf_loader_get_pixbuf(loader);
    gtk_window_set_icon(GTK_WINDOW(windowHandle), (GdkPixbuf*)icon);

    #elif defined(__APPLE__)
    id icon = nullptr;
    const char *iconData = iconDataStr.c_str();
    icon =
        ((id (*)(id, SEL))objc_msgSend)("NSImage"_cls, "alloc"_sel);

    id nsIconData = ((id (*)(id, SEL, const char*, int))objc_msgSend)("NSData"_cls,
                "dataWithBytes:length:"_sel, iconData, iconDataStr.length());

    ((void (*)(id, SEL, id))objc_msgSend)(icon, "initWithData:"_sel, nsIconData);
    ((void (*)(id, SEL, id))objc_msgSend)(((id (*)(id, SEL))objc_msgSend)("NSApplication"_cls,
                                "sharedApplication"_sel),
                "setApplicationIconImage:"_sel,icon);

    #elif defined(_WIN32)
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    HICON icon = nullptr;
    const char *iconData = iconDataStr.c_str();
    unsigned char *uiconData = reinterpret_cast<unsigned char*>(const_cast<char*>(iconData));
    IStream *pStream = SHCreateMemStream((BYTE *) uiconData, iconDataStr.length());
    Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromStream(pStream);
    bitmap->GetHICON(&icon);
    pStream->Release();

    SendMessage(windowHandle, WM_SETICON, ICON_SMALL, (LPARAM)icon);
    SendMessage(windowHandle, WM_SETICON, ICON_BIG, (LPARAM)icon);
    GdiplusShutdown(gdiplusToken);
    #endif
}

void setAlwaysOnTop(bool onTop) {
    #if defined(__linux__) || defined(__FreeBSD__)
    gtk_window_set_keep_above(GTK_WINDOW(windowHandle), onTop);
    #elif defined(__APPLE__)
    ((void (*)(id, SEL, int))objc_msgSend)((id) windowHandle,
            "setLevel:"_sel, onTop ? NSFloatingWindowLevel : NSBaseWindowLevel);
    #elif defined(_WIN32)
    SetWindowPos(windowHandle, onTop ? HWND_TOPMOST : HWND_NOTOPMOST,
                0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    #endif
}

void setBorderless() {
    #if defined(__linux__) || defined(__FreeBSD__)
    gtk_window_set_decorated(GTK_WINDOW(windowHandle), false);
    #elif defined(__APPLE__)
    unsigned long windowStyleMask = ((unsigned long (*)(id, SEL))objc_msgSend)(
        (id) windowHandle, "styleMask"_sel);
    windowStyleMask &= ~NSWindowStyleMaskTitled;
    ((void (*)(id, SEL, int))objc_msgSend)((id) windowHandle,
            "setStyleMask:"_sel, windowStyleMask);
    #elif defined(_WIN32)
    DWORD currentStyle = GetWindowLong(windowHandle, GWL_STYLE);
    currentStyle &= ~(WS_CAPTION | WS_THICKFRAME);
    SetWindowLong(windowHandle, GWL_STYLE, currentStyle);
    SetWindowPos(windowHandle, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |
                    SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    #endif
}

void _close(int exitCode) {
    if(nativeWindow) {
        nativeWindow->terminate(exitCode);
        delete nativeWindow;
    }
}

namespace controllers {

void __createWindow() {
    nativeWindow = new webview::webview(windowProps.enableInspector, nullptr);
    nativeWindow->set_title(windowProps.title);
    nativeWindow->set_size(windowProps.sizeOptions.width, windowProps.sizeOptions.height, windowProps.sizeOptions.minWidth,
                    windowProps.sizeOptions.minHeight, windowProps.sizeOptions.maxWidth, windowProps.sizeOptions.maxHeight,
                    windowProps.sizeOptions.resizable);
    nativeWindow->setEventHandler(&window::handlers::windowStateChange);

    #if defined(__linux__) || defined(__FreeBSD__)
    windowHandle = (GtkWidget*) nativeWindow->window();

    #elif defined(__APPLE__)
    windowHandle = (id) nativeWindow->window();
    ((void (*)(id, SEL, bool))objc_msgSend)((id) windowHandle,
                "setHasShadow:"_sel, true);

    #elif defined(_WIN32)
    windowHandle = (HWND) nativeWindow->window();
    #endif

    if(windowProps.maximize)
        window::maximize();

    if(windowProps.hidden)
        window::hide();

    if (!windowProps.hidden && window::isFakeHidden())
		window::undoFakeHidden();

    if(windowProps.fullScreen)
        window::setFullScreen();

    if(windowProps.icon != "")
        window::setIcon(windowProps.icon);

    if(windowProps.alwaysOnTop)
        window::setAlwaysOnTop(true);

    if(windowProps.borderless)
        window::setBorderless();

    nativeWindow->navigate(windowProps.url);
    nativeWindow->run();
}

window::SizeOptions __jsonToSizeOptions(const json &input, bool useDefaultRect = false) {
    window::SizeOptions sizeOptions;

    if(useDefaultRect) {
        sizeOptions.width = 800;
        sizeOptions.height = 600;
    }

    if(helpers::hasField(input, "width"))
        sizeOptions.width = input["width"].get<int>();

    if(helpers::hasField(input, "height"))
        sizeOptions.height = input["height"].get<int>();

    if(helpers::hasField(input, "minWidth"))
        sizeOptions.minWidth = input["minWidth"].get<int>();

    if(helpers::hasField(input, "minHeight"))
        sizeOptions.minHeight = input["minHeight"].get<int>();

    if(helpers::hasField(input, "maxWidth"))
        sizeOptions.maxWidth = input["maxWidth"].get<int>();

    if(helpers::hasField(input, "maxHeight"))
        sizeOptions.maxHeight = input["maxHeight"].get<int>();

    if(helpers::hasField(input, "resizable"))
        sizeOptions.resizable = input["resizable"].get<bool>();
    return sizeOptions;
}

json __sizeOptionsToJson() {
    json output = {
        {"width", windowProps.sizeOptions.width},
        {"height", windowProps.sizeOptions.height},
        {"minWidth", windowProps.sizeOptions.minWidth},
        {"minHeight", windowProps.sizeOptions.minHeight},
        {"maxWidth", windowProps.sizeOptions.maxWidth},
        {"maxHeight", windowProps.sizeOptions.maxHeight},
        {"resizable", windowProps.sizeOptions.resizable}
    };
    return output;
}

#if defined(__APPLE__)
CGRect __getWindowRect() {
    // "frame"_sel is the easiest way, but it crashes
    // So, this is a workaround with low-level APIs.
    long winId = ((long(*)(id, SEL))objc_msgSend)(windowHandle, "windowNumber"_sel);
    auto winInfoArray = CGWindowListCopyWindowInfo(kCGWindowListOptionIncludingWindow, winId);
    auto winInfo = CFArrayGetValueAtIndex(winInfoArray, 0);
    auto winBounds = CFDictionaryGetValue((CFDictionaryRef) winInfo, kCGWindowBounds);

    CGRect winPos;
    CGRectMakeWithDictionaryRepresentation((CFDictionaryRef) winBounds, &winPos);
    return winPos;
}
#endif

json setTitle(const json &input) {
    json output;
    string title = "";
    if(helpers::hasField(input, "title")) {
        title = input["title"].get<string>();
    }
    nativeWindow->set_title(title);
    output["success"] = true;
    return output;
}

json getTitle(const json &input) {
    json output;
    output["returnValue"] = nativeWindow->get_title();
    output["success"] = true;
    return output;
}

json maximize(const json &input) {
    json output;
    window::maximize();
    output["success"] = true;
    return output;
}

json unmaximize(const json &input) {
    json output;
    window::unmaximize();
    output["success"] = true;
    return output;
}

json isMaximized(const json &input) {
    json output;
    output["returnValue"] = window::isMaximized();
    output["success"] = true;
    return output;
}

json minimize(const json &input) {
    json output;
    #if defined(__linux__) || defined(__FreeBSD__)
    gtk_window_iconify(GTK_WINDOW(windowHandle));
    #elif defined(_WIN32)
    ShowWindow(windowHandle, SW_MINIMIZE);
    #elif defined(__APPLE__)
    ((void (*)(id, SEL, id))objc_msgSend)((id) windowHandle,
        "miniaturize:"_sel, NULL);
    #endif
    output["success"] = true;
    return output;
}

json show(const json &input) {
    json output;
    #if defined(__linux__) || defined(__FreeBSD__)
    nativeWindow->dispatch([&]() {
        window::show();
    });
    #else
    window::show();
    #endif
    output["success"] = true;
    return output;
}

json hide(const json &input) {
    json output;
    window::hide();
    output["success"] = true;
    return output;
}

json isVisible(const json &input) {
    json output;
    output["returnValue"] = window::isVisible();
    output["success"] = true;
    return output;
}

json setFullScreen(const json &input) {
    json output;
    window::setFullScreen();
    output["success"] = true;
    return output;
}

json exitFullScreen(const json &input) {
    json output;
    window::exitFullScreen();
    output["success"] = true;
    return output;
}

json isFullScreen(const json &input) {
    json output;
    output["returnValue"] = window::isFullScreen();
    output["success"] = true;
    return output;
}

json focus(const json &input) {
    json output;
    #if defined(__linux__) || defined(__FreeBSD__)
    gtk_window_present(GTK_WINDOW(windowHandle));
    #elif defined(__APPLE__)
    ((void (*)(id, SEL, id))objc_msgSend)((id) windowHandle,
            "orderFront:"_sel, NULL);
    #elif defined(_WIN32)
    SetForegroundWindow(windowHandle);
    #endif
    output["success"] = true;
    return output;
}

json setIcon(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"icon"})) {
        output["error"] = errors::makeMissingArgErrorPayload();
        return output;
    }
    string icon = input["icon"].get<string>();
    window::setIcon(icon);
    output["success"] = true;
    return output;
}

json move(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"x", "y"})) {
        output["error"] = errors::makeMissingArgErrorPayload();
        return output;
    }
    int x = input["x"].get<int>();
    int y = input["y"].get<int>();
    #if defined(__linux__) || defined(__FreeBSD__)
    gtk_window_move(GTK_WINDOW(windowHandle), x, y);
    #elif defined(__APPLE__)
    auto displayId = CGMainDisplayID();
    int height = CGDisplayPixelsHigh(displayId);
    ((void (*)(id, SEL, CGPoint))objc_msgSend)(
        (id) windowHandle, "setFrameTopLeftPoint:"_sel,
        CGPointMake(x, height - y));
    #elif defined(_WIN32)
    RECT winPos;
    GetWindowRect(windowHandle, &winPos);
    MoveWindow(windowHandle, x, y, winPos.right - winPos.left,
                winPos.bottom - winPos.top, true);
    #endif
    output["success"] = true;
    return output;
}

json setSize(const json &input) {
    json output;
    windowProps.sizeOptions = __jsonToSizeOptions(input);
    nativeWindow->set_size(windowProps.sizeOptions.width, windowProps.sizeOptions.height,
                    windowProps.sizeOptions.minWidth, windowProps.sizeOptions.minHeight,
                    windowProps.sizeOptions.maxWidth, windowProps.sizeOptions.maxHeight,
                    windowProps.sizeOptions.resizable);
    output["success"] = true;
    return output;
}

json getSize(const json &input) {
    json output;
    #if defined(__linux__) || defined(__FreeBSD__)
    gtk_window_get_size(GTK_WINDOW(windowHandle),
                        &windowProps.sizeOptions.width, &windowProps.sizeOptions.height);
    #elif defined(__APPLE__)
    CGRect winPos = __getWindowRect();

    windowProps.sizeOptions.width = winPos.size.width;
    windowProps.sizeOptions.height = winPos.size.height;

    #elif defined(_WIN32)
    RECT winPos;
    GetWindowRect(windowHandle, &winPos);
    windowProps.sizeOptions.width = winPos.right - winPos.left;
    windowProps.sizeOptions.height = winPos.bottom - winPos.top;

    #endif
    output["returnValue"] = __sizeOptionsToJson();
    output["success"] = true;
    return output;
}

json setAlwaysOnTop(const json &input) {
    json output;
    bool onTop = true;
    if(helpers::hasField(input, "onTop")) {
        onTop = input["onTop"].get<bool>();
    }
    window::setAlwaysOnTop(onTop);
    output["success"] = true;
    return output;
}

json getPosition(const json &input) {
    json output;

    int x, y;
    #if defined(__linux__) || defined(__FreeBSD__)
    gdk_window_get_root_origin(gtk_widget_get_window(windowHandle), &x, &y);

    #elif defined(__APPLE__)
    CGRect winPos = __getWindowRect();
    x = winPos.origin.x;
    y = winPos.origin.y;

    #elif defined(_WIN32)
    RECT winPos;
    GetWindowRect(windowHandle, &winPos);
    x = winPos.left;
    y = winPos.top;
    #endif

    json posRes;
    posRes["x"] = x;
    posRes["y"] = y;
    output["returnValue"] = posRes;
    output["success"] = true;
    return output;
}

json init(const json &input) {
    json output;

    windowProps.sizeOptions = __jsonToSizeOptions(input, true);

    if(helpers::hasField(input, "fullScreen"))
        windowProps.fullScreen = input["fullScreen"].get<bool>();

    if(helpers::hasField(input, "alwaysOnTop"))
        windowProps.alwaysOnTop = input["alwaysOnTop"].get<bool>();

    if(helpers::hasField(input, "title"))
        windowProps.title = input["title"].get<string>();

    if(helpers::hasField(input, "url"))
        windowProps.url = input["url"].get<string>();

    if(helpers::hasField(input, "icon"))
        windowProps.icon = input["icon"].get<string>();

    if(helpers::hasField(input, "enableInspector"))
        windowProps.enableInspector = input["enableInspector"].get<bool>();

    if(helpers::hasField(input, "borderless"))
        windowProps.borderless = input["borderless"].get<bool>();

    if(helpers::hasField(input, "maximize"))
        windowProps.maximize = input["maximize"].get<bool>();

    if(helpers::hasField(input, "hidden"))
        windowProps.hidden = input["hidden"].get<bool>();

    if(helpers::hasField(input, "exitProcessOnClose"))
        windowProps.exitProcessOnClose = input["exitProcessOnClose"].get<bool>();

    __createWindow();
    output["success"] = true;
    return output;
}

} // namespace controllers

} // namespace window
