#include <string>
#include <iostream>
#include <filesystem>
#include <regex>

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
#define NSPNGFileType 4

#define kCGWindowListOptionIncludingWindow 8
#define kCGWindowImageBoundsIgnoreFraming 1

#elif defined(_WIN32)
#define _WINSOCKAPI_
#include <windows.h>
#include <gdiplus.h>
#include <winuser.h>
#pragma comment(lib, "Gdiplus.lib")
#pragma comment(lib, "WebView2LoaderStatic.lib")
#endif

#include "lib/json/json.hpp"
#include "lib/webview/webview.h"
#include "settings.h"
#include "resources.h"
#include "helpers.h"
#include "errors.h"
#include "server/neuserver.h"
#include "auth/permission.h"
#include "api/app/app.h"
#include "api/window/window.h"
#include "api/events/events.h"
#include "api/fs/fs.h"
#include "api/debug/debug.h"

using namespace std;
using json = nlohmann::json;
#if defined(_WIN32)
using namespace Gdiplus;
#endif

namespace window {

webview::webview *nativeWindow;
#if defined(__linux__) || defined(__FreeBSD__)
bool isGtkWindowFullScreen = false;
bool isGtkWindowMinimized = false;

#elif defined(_WIN32)
bool isWinWindowFullScreen = false;
DWORD savedStyle;
DWORD savedStyleX;
RECT savedRect;
#endif

window::WindowOptions windowProps;
bool savedState = false;
NEU_W_HANDLE windowHandle;

namespace handlers {

void windowStateChange(int state) {
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
            break;
        case WEBVIEW_WINDOW_FULLSCREEN:
            #if defined(__linux__) || defined(__FreeBSD__)
                isGtkWindowFullScreen = true;
            #endif
            break;
        case WEBVIEW_WINDOW_UNFULLSCREEN:
            #if defined(__linux__) || defined(__FreeBSD__)
                isGtkWindowFullScreen = false;
            #endif
            break;
        case WEBVIEW_WINDOW_MINIMIZED:
            #if defined(__linux__) || defined(__FreeBSD__)
                isGtkWindowMinimized = true;
            #endif
            break;
        case WEBVIEW_WINDOW_UNMINIMIZED:
            #if defined(__linux__) || defined(__FreeBSD__)
                isGtkWindowMinimized = false;
            #endif
            break;
    }
}

} // namespace handlers


pair<int, int> __getCenterPos(bool useConfigSizes = false) {
    int x, y = 0;
    int width, height = 0;
    if(useConfigSizes) {
        width = windowProps.sizeOptions.width;
        height = windowProps.sizeOptions.height;
    }
    else {
        window::SizeOptions opt = window::getSize();
        width = opt.width;
        height = opt.height;
    }
    #if defined(__linux__) || defined(__FreeBSD__)
    GdkRectangle screen;
    gdk_monitor_get_workarea(gdk_display_get_primary_monitor(gdk_display_get_default()), &screen);
    x = (screen.width - width) / 2;
    y = (screen.height - height) / 2;
    #elif defined(__APPLE__)
    auto displayId = CGMainDisplayID();
    x = (CGDisplayPixelsWide(displayId) - width) / 2;
    y = (CGDisplayPixelsHigh(displayId) - height) / 2;
    #elif defined(_WIN32)
	RECT screen;
	GetWindowRect(GetDesktopWindow(), &screen);
    x = ((screen.right - screen.left) - width) / 2;
    y = ((screen.bottom - screen.top) - height) / 2;
    #endif
    return make_pair(x, y);
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

#if defined(_WIN32)
bool __isFakeHidden() {
	// Checks whether the window is on the screen viewport
	RECT winPos;

	if(GetWindowRect( windowHandle, &winPos)) {
		return winPos.left > 9999;
	}
	return false;
}

void __undoFakeHidden() {
    int x = windowProps.x;
    int y = windowProps.y;
    if(!window::isSavedStateLoaded() && windowProps.center) {
        pair<int, int> pos = __getCenterPos(true);
        x = pos.first;
        y = pos.second;
    }
	ShowWindow(windowHandle, SW_HIDE);
	SetWindowLong(windowHandle, GWL_EXSTYLE, nativeWindow->m_originalStyleEx);
	SetWindowPos(windowHandle, nullptr,
        x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	ShowWindow(windowHandle, SW_SHOW);
}

bool __getEncoderClsid(const WCHAR *format, CLSID *pClsid) {
    UINT num = 0;
    UINT size = 0;

    GetImageEncodersSize(&num, &size);
    if(size == 0) return false; // Failure

    ImageCodecInfo *pImageCodecInfo = (ImageCodecInfo *)(malloc(size));
    if(pImageCodecInfo == NULL) return false;

    GetImageEncoders(num, size, pImageCodecInfo);
    for(UINT i = 0; i < num; ++i) {
        if(wcscmp(pImageCodecInfo[i].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[i].Clsid;
            free(pImageCodecInfo);
            return true;
        }
    }
    free(pImageCodecInfo);
    return false;
}
#endif

json __sizeOptionsToJson(const window::SizeOptions &opt) {
    json output = {
        {"width", opt.width},
        {"height", opt.height},
        {"minWidth", opt.minWidth},
        {"minHeight", opt.minHeight},
        {"maxWidth", opt.maxWidth},
        {"maxHeight", opt.maxHeight},
        {"resizable", opt.resizable}
    };
    return output;
}

void __saveWindowProps() {
    windowProps.sizeOptions = window::getSize();
    json options = __sizeOptionsToJson(windowProps.sizeOptions);
    pair<int, int> pos = window::getPosition();
    options["x"] = pos.first;
    options["y"] = pos.second;
    options["maximize"] = window::isMaximized();

    filesystem::create_directories(CONVSTR(settings::joinAppDataPath("/.tmp")));
    fs::FileWriterOptions writerOptions = { settings::joinAppDataPath(NEU_WIN_CONFIG_FILE), helpers::jsonToString(options) };
    fs::writeFile(writerOptions);
}

bool __loadSavedWindowProps() {
    fs::FileReaderResult readerResult = fs::readFile(settings::joinAppDataPath(NEU_WIN_CONFIG_FILE));
    if(readerResult.status != errors::NE_ST_OK) {
        return false;
    }

    try {
        json options = json::parse(readerResult.data);
        windowProps.x = options["x"].get<int>();
        windowProps.y = options["y"].get<int>();
        windowProps.maximize = options["maximize"].get<bool>();
        windowProps.sizeOptions.width = options["width"].get<int>();
        windowProps.sizeOptions.height = options["height"].get<int>();
    }
    catch(exception e) {
        debug::log(debug::LogTypeError, errors::makeErrorMsg(errors::NE_CF_UNBLWCF, string(NEU_WIN_CONFIG_FILE)));
        return false;
    }
    return true;
}

void _close(int exitCode) {
    if(nativeWindow) {
        if(windowProps.useSavedState) {
            __saveWindowProps();
        }
        nativeWindow->terminate(exitCode);
        #if defined(_WIN32)
        FreeConsole();
        #endif
        delete nativeWindow;
    }
}

NEU_W_HANDLE getWindowHandle() {
    return windowHandle;
}

bool isSavedStateLoaded() {
  return savedState;
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
    SetForegroundWindow(windowHandle);
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
    SetForegroundWindow(windowHandle);
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
    SetWindowPos(windowHandle, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

    if (!SetForegroundWindow(windowHandle)) {
        FLASHWINFO fi;
        fi.cbSize = sizeof(FLASHWINFO);
        fi.hwnd = windowHandle;
        fi.dwFlags = FLASHW_TRAY | FLASHW_TIMERNOFG;
        fi.uCount = 0; // Flash indefinitely until the window comes to the foreground
        fi.dwTimeout = 0;
        FlashWindowEx(&fi);
    }

    if (__isFakeHidden())
        __undoFakeHidden();
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

void move(int x, int y) {
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
}

window::SizeOptions getSize() {
    int width, height = 0;
    #if defined(__linux__) || defined(__FreeBSD__)
    gtk_window_get_size(GTK_WINDOW(windowHandle),
                        &width, &height);
    #elif defined(__APPLE__)
    CGRect winPos = __getWindowRect();

    width = winPos.size.width;
    height = winPos.size.height;

    #elif defined(_WIN32)
    RECT winPos;
    GetWindowRect(windowHandle, &winPos);
    width = winPos.right - winPos.left;
    height = winPos.bottom - winPos.top;
    #endif

    windowProps.sizeOptions.width = width;
    windowProps.sizeOptions.height = height;
    return windowProps.sizeOptions;
}

pair<int, int> getPosition() {
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

    return make_pair(x, y);
}

void center(bool useConfigSizes = false) {
    pair<int, int> pos = __getCenterPos(useConfigSizes);
    window::move(pos.first, pos.second);
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

bool snapshot(const string &filename) {
    #if defined(__linux__) || defined(__FreeBSD__)
    int width, height, x, y;
    GdkWindow *window = gtk_widget_get_window(windowHandle);
    gdk_window_get_geometry(window, &x, &y, &width, &height);
    GdkPixbuf *screenshot = gdk_pixbuf_get_from_window(window, x, y, width, height);
    return gdk_pixbuf_save(screenshot, filename.c_str(), "png", nullptr, nullptr);

    #elif defined(__APPLE__)
    CGRect frameRect = __getWindowRect();
    CGRect clientRect =
            ((CGRect (*)(id, SEL, CGRect))objc_msgSend)(windowHandle, "contentRectForFrameRect:"_sel, frameRect);
    clientRect.origin.y +=  frameRect.size.height - clientRect.size.height;

    long winId = ((long(*)(id, SEL))objc_msgSend)(windowHandle, "windowNumber"_sel);
    CGImageRef imgRef = CGWindowListCreateImage(clientRect, kCGWindowListOptionIncludingWindow, winId, kCGWindowImageBoundsIgnoreFraming); 
      
    id screenshot =
        ((id (*)(id, SEL))objc_msgSend)("NSBitmapImageRep"_cls, "alloc"_sel);
    ((void (*)(id, SEL, CGImageRef))objc_msgSend)(screenshot, "initWithCGImage:"_sel, imgRef);
    id screenshotData =
        ((id (*)(id, SEL, int, id))objc_msgSend)(screenshot, "representationUsingType:properties:"_sel, NSPNGFileType, nullptr);
    bool status = ((bool (*)(id, SEL, id, bool))objc_msgSend)(screenshotData, "writeToFile:atomically:"_sel, 
            ((id(*)(id, SEL, const char *))objc_msgSend)("NSString"_cls, "stringWithUTF8String:"_sel, filename.c_str())
    , true);
    
    return status;
   

    #elif defined(_WIN32)
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    RECT rect;
    GetClientRect(windowHandle, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    HDC hdcWindow = GetDC(windowHandle);
    HDC hdcMem = CreateCompatibleDC(hdcWindow);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcWindow, width, height);

    SelectObject(hdcMem, hBitmap);

    if(!PrintWindow(windowHandle, hdcMem, PW_CLIENTONLY | PW_RENDERFULLCONTENT)) return false;

    Bitmap *bitmap = Bitmap::FromHBITMAP(hBitmap, nullptr);
    CLSID clsid;
    if(!__getEncoderClsid(L"image/png", &clsid)) return false;

    bool status = bitmap->Save(CONVSTR(filename).c_str(), &clsid, nullptr) == Gdiplus::Ok;

    DeleteDC(hdcMem);
    ReleaseDC(windowHandle, hdcWindow);
    DeleteObject(hBitmap);
    GdiplusShutdown(gdiplusToken);

    return status;
    #endif
}

namespace controllers {

void __injectClientLibrary() {
    json options = settings::getConfig();
    json jClientLibrary = options["cli"]["clientLibrary"];
    if(!jClientLibrary.is_null()) {
        string clientLibPath = jClientLibrary.get<string>();
        fs::FileReaderResult fileReaderResult = resources::getFile(clientLibPath);
        if(fileReaderResult.status == errors::NE_ST_OK) {
            nativeWindow->init(settings::getGlobalVars() + "var NL_CINJECTED = true;" + 
                fileReaderResult.data);
        }
    }
}

void __injectScript() {
    json jInjectScript = settings::getOptionForCurrentMode("injectScript");
    if(!jInjectScript.is_null()) {
        string injectScript = jInjectScript.get<string>();
        fs::FileReaderResult fileReaderResult = resources::getFile(injectScript);
        if(fileReaderResult.status == errors::NE_ST_OK) {
            nativeWindow->init("var NL_SINJECTED = true;" + fileReaderResult.data);
        }
    }
}

void __createWindow() {
    savedState = windowProps.useSavedState && __loadSavedWindowProps();

    nativeWindow = new webview::webview(windowProps.enableInspector, nullptr, windowProps.transparent);
    nativeWindow->set_title(windowProps.title);
    if(windowProps.extendUserAgentWith != "") {
        nativeWindow->extend_user_agent(windowProps.extendUserAgentWith);
    }
    nativeWindow->set_size(windowProps.sizeOptions.width,
                    windowProps.sizeOptions.height,
                    windowProps.sizeOptions.minWidth, windowProps.sizeOptions.minHeight,
                    windowProps.sizeOptions.maxWidth, windowProps.sizeOptions.maxHeight,
                    windowProps.sizeOptions.resizable);
    nativeWindow->setEventHandler(&window::handlers::windowStateChange);

    if(windowProps.injectGlobals) 
        nativeWindow->init(settings::getGlobalVars() + "var NL_GINJECTED = true;");

    if(windowProps.injectClientLibrary)
        __injectClientLibrary();

    if(windowProps.injectScript != "")
        __injectScript();

    #if defined(__linux__) || defined(__FreeBSD__)
    windowHandle = (GtkWidget*) nativeWindow->window();

    #elif defined(__APPLE__)
    windowHandle = (id) nativeWindow->window();
    ((void (*)(id, SEL, bool))objc_msgSend)((id) windowHandle,
                "setHasShadow:"_sel, true);

    #elif defined(_WIN32)
    windowHandle = (HWND) nativeWindow->window();
    #endif

    #if !defined(_WIN32)
    if(!window::isSavedStateLoaded() && windowProps.center)
        window::center(true);
    else
        window::move(windowProps.x, windowProps.y);
    #endif

    if(windowProps.maximize)
        window::maximize();

    if(windowProps.hidden)
        window::hide();

    #if defined(_WIN32)
    if (!windowProps.hidden && __isFakeHidden())
		__undoFakeHidden();
    #endif

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

json unminimize(const json &input) {
    json output;
    #if defined(__linux__) || defined(__FreeBSD__)
    gtk_window_present(GTK_WINDOW(windowHandle));
    #elif defined(_WIN32)
    ShowWindow(windowHandle, SW_RESTORE);
    #elif defined(__APPLE__)
    ((void (*)(id, SEL, id))objc_msgSend)((id) windowHandle,
        "deminiaturize:"_sel, NULL);
    #endif
    output["success"] = true;
    return output;
}

json isMinimized(const json &input) {
    json output;
    bool minimized = false;
    #if defined(__linux__) || defined(__FreeBSD__)
    minimized = isGtkWindowMinimized;
    #elif defined(_WIN32)
    minimized = IsIconic(windowHandle) == 1;
    #elif defined(__APPLE__)
    minimized = ((bool (*)(id, SEL, id))objc_msgSend)((id) windowHandle,
        "isMiniaturized"_sel, NULL);
    #endif
    output["returnValue"] = minimized;
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
    window::move(x, y);
    output["success"] = true;
    return output;
}

json center(const json &input) {
    json output;
    window::center();
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
    windowProps.sizeOptions = window::getSize();

    output["returnValue"] = __sizeOptionsToJson(windowProps.sizeOptions);
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
    json posRes;
    pair<int, int> pos = window::getPosition();
    posRes["x"] = pos.first;
    posRes["y"] = pos.second;
    output["returnValue"] = posRes;
    output["success"] = true;
    return output;
}

json init(const json &input) {
    json output;

    windowProps.sizeOptions = __jsonToSizeOptions(input, true);

    if(helpers::hasField(input, "x"))
        windowProps.x = input["x"].get<int>();

    if(helpers::hasField(input, "y"))
        windowProps.y = input["y"].get<int>();

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

    if(helpers::hasField(input, "extendUserAgentWith"))
        windowProps.extendUserAgentWith = input["extendUserAgentWith"].get<string>();

    if(helpers::hasField(input, "injectScript"))
        windowProps.injectScript = input["injectScript"].get<string>();

    if(helpers::hasField(input, "enableInspector"))
        windowProps.enableInspector = input["enableInspector"].get<bool>();

    if(helpers::hasField(input, "borderless"))
        windowProps.borderless = input["borderless"].get<bool>();

    if(helpers::hasField(input, "maximize"))
        windowProps.maximize = input["maximize"].get<bool>();

    if(helpers::hasField(input, "hidden"))
        windowProps.hidden = input["hidden"].get<bool>();

    if(helpers::hasField(input, "center"))
        windowProps.center = input["center"].get<bool>();

    if(helpers::hasField(input, "transparent"))
        windowProps.transparent = input["transparent"].get<bool>();

    if(helpers::hasField(input, "exitProcessOnClose"))
        windowProps.exitProcessOnClose = input["exitProcessOnClose"].get<bool>();

    if(helpers::hasField(input, "useSavedState"))
        windowProps.useSavedState = input["useSavedState"].get<bool>();

    if(helpers::hasField(input, "injectGlobals"))
        windowProps.injectGlobals = input["injectGlobals"].get<bool>();

    if(helpers::hasField(input, "injectClientLibrary"))
        windowProps.injectClientLibrary = input["injectClientLibrary"].get<bool>();

    __createWindow();
    output["success"] = true;
    return output;
}

json snapshot(const json &input) {
    json output;
    if (!helpers::hasRequiredFields(input, {"path"})) {
        output["error"] = errors::makeMissingArgErrorPayload();
        return output;
    }

    string imageFile = input["path"].get<string>();
    if(window::snapshot(imageFile)) {
        output["success"] = true;
    }
    else {
        output["error"] = errors::makeErrorPayload(errors::NE_WI_UNBSWSR, imageFile);
    }
    return output;
}

bool applyMainMenu(const json &menu) {
    
    #if defined(_WIN32) || defined(__APPLE__) || defined(__linux__)
    auto mainMenuItemClicked = [](std::string itemId) {
        json eventPayload;
        eventPayload["id"] = itemId;
        Neutralino.events.dispatch("mainMenuItemClicked", eventPayload);
    };
    #endif

    #if defined(_WIN32)
    HMENU hMenu = CreateMenu();
    for (const auto &menuItem : menu) {
        HMENU hSubMenu = CreatePopupMenu();
        for (const auto &item : menuItem["menuItems"]) {
            if (item["text"] == "-") {
                AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
            } else {
                AppendMenu(hSubMenu, MF_STRING, item["id"].get<int>(), item["text"].get<std::string>().c_str());
            }
        }
        AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, menuItem["text"].get<std::string>().c_str());
    }
    SetMenu(windowHandle, hMenu);
    return true;

    #elif defined(__APPLE__)
    id mainMenu = ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSMenu"), sel_getUid("alloc"));
    mainMenu = ((id (*)(id, SEL))objc_msgSend)(mainMenu, sel_getUid("init"));

    for (const auto &menuItem : menu) {
        id menuItemObj = ((id (*)(id, SEL, id))objc_msgSend)(mainMenu, sel_getUid("addItemWithTitle:action:keyEquivalent:"), menuItem["text"].get<std::string>().c_str(), NULL, @"");
        id submenu = ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSMenu"), sel_getUid("alloc"));
        submenu = ((id (*)(id, SEL))objc_msgSend)(submenu, sel_getUid("init"));

        for (const auto &item : menuItem["menuItems"]) {
            if (item["text"] == "-") {
                ((void (*)(id, SEL, id))objc_msgSend)(submenu, sel_getUid("addItem:"), ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSMenuItem"), sel_getUid("separatorItem")));
            } else {
                id subItem = ((id (*)(id, SEL, id, SEL, id))objc_msgSend)((id)objc_getClass("NSMenuItem"), sel_getUid("alloc"), item["text"].get<std::string>().c_str(), sel_getUid("menuAction:"), @"");
                ((void (*)(id, SEL, id))objc_msgSend)(submenu, sel_getUid("addItem:"), subItem);
            }
        }
        ((void (*)(id, SEL, id))objc_msgSend)(menuItemObj, sel_getUid("setSubmenu:"), submenu);
    }
    ((void (*)(id, SEL, id))objc_msgSend)(((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSApplication"), sel_getUid("sharedApplication")), sel_getUid("setMainMenu:"), mainMenu);

    // lambda
    auto menuAction = [](id self, SEL _cmd) {
        id menuItem = self;
        const char *menuId = ((const char *(*)(id, SEL))objc_msgSend)(menuItem, sel_getUid("identifier"));
        json eventPayload;
        eventPayload["id"] = std::string(menuId);
        Neutralino.events.dispatch("mainMenuItemClicked", eventPayload);
    };

    return true;
    #endif
}

json setMainMenu(const json &input) {
    json output;

    if (!helpers::hasField(input, "menu")) {
        output["error"] = "Missing required field: menu";
        return output;
    }

    json menu = input["menu"];

    if (!applyMainMenu(menu)) {
        output["error"] = "Failed to set the menu";
        return output;
    }

    output["success"] = true;
    return output;
}

} // namespace controllers

} // namespace window
