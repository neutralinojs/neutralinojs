#include <iostream>

#if defined(__linux__) || defined(__FreeBSD__)
#include <gtk/gtk.h>
#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#elif defined(__APPLE__)
#include <objc/objc-runtime.h>
#define NSFloatingWindowLevel 5
#define NSWindowStyleMaskFullScreen 16384

#elif defined(_WIN32)
#include <windows.h>
#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")
#pragma comment(lib, "WebView2Loader.dll.lib")
#endif

#include "lib/json.hpp"
#include "settings.h"
#define WEBVIEW_IMPLEMENTATION
#include "lib/webview/webview.h"
#include "api/window/window.h"

using namespace std;
using json = nlohmann::json;
#if defined(_WIN32)
using namespace Gdiplus;
#endif

webview::webview *nativeWindow;
#if defined(__linux__) || defined(__FreeBSD__)
bool isGtkWindowFullScreen = false;
GtkWidget* windowHandle;
#elif defined(__APPLE__)
id windowHandle;
#elif defined(_WIN32)
bool isWinWindowFullScreen = false;
DWORD savedStyle;
DWORD savedStyleX;
RECT savedRect;
HWND windowHandle;
#endif

struct WindowOptions {
    int width = 800;
    int height = 600;
    int minWidth = -1;
    int minHeight = -1;
    int maxWidth = -1;
    int maxHeight = -1;
    bool fullScreen = false;
    bool alwaysOnTop = false;
    bool enableInspector = false;
    bool borderless= false;
    bool maximize = false;
    bool hidden = false;
    bool resizable = true;
    bool maximizable = true;
    string title = "Neutralinojs window";
    string url = "https://neutralino.js.org";
    string icon = "";
};

namespace window {
    void executeJavaScript(string js) {
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
    
    void setIcon(string iconFile) {
        string iconDataStr = settings::getFileContent(iconFile);
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
    
    void setAlwaysOnTop() {
        #if defined(__linux__) || defined(__FreeBSD__)
        gtk_window_set_keep_above(GTK_WINDOW(windowHandle), true);
        #elif defined(__APPLE__)
        ((void (*)(id, SEL, int))objc_msgSend)((id) windowHandle, 
                "setLevel:"_sel, NSFloatingWindowLevel);
        #elif defined(_WIN32)
        SetWindowPos(windowHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
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

namespace controllers {
    void __createWindow(WindowOptions windowProps) {
        nativeWindow = new webview::webview(windowProps.enableInspector, nullptr);
        nativeWindow->set_title(windowProps.title);
        nativeWindow->set_size(windowProps.width, windowProps.height, windowProps.minWidth,
                        windowProps.minHeight, windowProps.maxWidth, windowProps.maxHeight, 
                        windowProps.resizable);
                        
        #if defined(__linux__) || defined(__FreeBSD__)
        windowHandle = (GtkWidget*) nativeWindow->window();
        
        g_signal_connect(G_OBJECT(windowHandle), "window-state-event",
             G_CALLBACK(+[](GtkWidget *widget, GdkEventWindowState *event, gpointer user_data) {
                 isGtkWindowFullScreen = event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN;
             }),
        nullptr);

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

        if(windowProps.fullScreen)
            window::setFullScreen();
            
        if(windowProps.icon != "")
            window::setIcon(windowProps.icon);
        
        if(windowProps.alwaysOnTop)
            window::setAlwaysOnTop();
        
        if(windowProps.borderless)
            window::setBorderless();

        nativeWindow->navigate(windowProps.url);
        nativeWindow->run();
    }

    json setTitle(json input) {
        json output;
        string title = "";
        if(!input["title"].is_null()) 
            title = input["title"];
        nativeWindow->set_title(title);
        output["success"] = true;
        return output;
    }

    json maximize(json input) {
        json output;
        window::maximize();
        output["success"] = true;
        return output;
    }

    json unmaximize(json input) {
        json output;
        window::unmaximize();
        output["success"] = true;
        return output;
    }
    
    json isMaximized(json input) {
        json output;
        output["returnValue"] = window::isMaximized();
        output["success"] = true;
        return output;
    }

    json minimize(json input) {
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
    
    json show(json input) {
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

    json hide(json input) {
        json output;
        window::hide();
        output["success"] = true;
        return output;
    }
    
    json isVisible(json input) {
        json output;
        output["returnValue"] = window::isVisible();
        output["success"] = true;
        return output;
    }
    
    json setFullScreen(json input) {
        json output;
        window::setFullScreen();
        output["success"] = true;
        return output;
    }

    json exitFullScreen(json input) {
        json output;
        window::exitFullScreen();
        output["success"] = true;
        return output;
    }
    
    json isFullScreen(json input) {
        json output;
        output["returnValue"] = window::isFullScreen();
        output["success"] = true;
        return output;
    }
    
    json focus(json input) {
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
    
    json setIcon(json input) {
        json output;
        string icon = input["icon"];
        window::setIcon(icon);
        output["success"] = true;
        return output;
    }
    
    json move(json input) {
        json output; 
        int x = input["x"];
        int y = input["y"];
        #if defined(__linux__) || defined(__FreeBSD__)
        gtk_window_move(GTK_WINDOW(windowHandle), x, y);
        #elif defined(__APPLE__)

        #elif defined(_WIN32)

        #endif
        output["success"] = true;
        return output;
    }
    
    json init(json input) {
        WindowOptions windowProps;
        json output;

        if(!input["width"].is_null())
            windowProps.width = input["width"];

        if(!input["height"].is_null())
            windowProps.height = input["height"];
            
        if(!input["minWidth"].is_null())
            windowProps.minWidth = input["minWidth"];

        if(!input["minHeight"].is_null())
            windowProps.minHeight = input["minHeight"];
            
        if(!input["maxWidth"].is_null())
            windowProps.maxWidth = input["maxWidth"];

        if(!input["maxHeight"].is_null())
            windowProps.maxHeight = input["maxHeight"];

        if(!input["fullScreen"].is_null())
            windowProps.fullScreen = input["fullScreen"];

        if(!input["alwaysOnTop"].is_null())
            windowProps.alwaysOnTop = input["alwaysOnTop"];

        if(!input["title"].is_null())
            windowProps.title = input["title"];

        if(!input["url"].is_null())
            windowProps.url = input["url"];

        if(!input["icon"].is_null())
            windowProps.icon = input["icon"];

        if(!input["enableInspector"].is_null())
            windowProps.enableInspector = input["enableInspector"];

        if(!input["borderless"].is_null())
            windowProps.borderless = input["borderless"];

        if(!input["maximize"].is_null())
            windowProps.maximize = input["maximize"];

        if(!input["hidden"].is_null())
            windowProps.hidden = input["hidden"];
            
        if(!input["resizable"].is_null())
            windowProps.resizable = input["resizable"];

        __createWindow(windowProps);
        output["success"] = true;
        return output;
    }

} // namespace controllers
} // namespace window
