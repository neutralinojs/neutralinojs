#include <iostream>

#if defined(__linux__)
#include <gtk/gtk.h>
#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#elif defined(__APPLE__)
#include <objc/objc-runtime.h>
#include "platform/platform.h"
#define NSFloatingWindowLevel 5

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

using namespace std;
using json = nlohmann::json;
#if defined(_WIN32)
using namespace Gdiplus;
#endif

webview::webview *nativeWindow;
#if defined(__linux__)
GtkWidget* windowHandle;
#elif defined(__APPLE__)
id windowHandle;
#elif defined(_WIN32)
HWND windowHandle;
#endif

namespace window {
    
    #if defined(__linux__)
    void __createWindow(int height, int width,
        bool fullScreen, string title, bool alwaysOnTop, void* icon,
        bool enableInspector, bool borderless, bool maximize, bool hidden, string url) {
        nativeWindow = new webview::webview(enableInspector, nullptr);
        nativeWindow->set_title(title);
        nativeWindow->set_size(width, height, WEBVIEW_HINT_NONE);
        windowHandle = (GtkWidget*) nativeWindow->window();

        // Window properties/modes
        if(fullScreen)
            gtk_window_fullscreen(GTK_WINDOW(windowHandle));

        gtk_window_set_keep_above(GTK_WINDOW(windowHandle), alwaysOnTop);

        if(maximize)
            gtk_window_maximize(GTK_WINDOW(windowHandle));
        if(hidden)
            gtk_widget_hide(windowHandle);

        gtk_window_set_decorated(GTK_WINDOW(windowHandle), !borderless);

        if(icon)
            gtk_window_set_icon(GTK_WINDOW(windowHandle), (GdkPixbuf*)icon);
        nativeWindow->navigate(url);
        nativeWindow->run();
    }
    #elif defined(__APPLE__)
    void __createWindow(int height, int width,
        bool fullScreen, string title, bool alwaysOnTop, id icon,
        bool enableInspector, bool borderless, bool maximize, bool hidden, string url) {
        nativeWindow = new webview::webview(enableInspector, nullptr);
        nativeWindow->set_title(title);
        nativeWindow->set_size(width, height, WEBVIEW_HINT_NONE);
        windowHandle = (id) nativeWindow->window();

        // Window properties/modes
        ((void (*)(id, SEL, bool))objc_msgSend)((id) windowHandle, 
                    "setHasShadow:"_sel, true);

        if(fullScreen)
            ((void (*)(id, SEL, id))objc_msgSend)((id) windowHandle, 
                    "toggleFullScreen:"_sel, NULL);
        
        if(alwaysOnTop)
            ((void (*)(id, SEL, int))objc_msgSend)((id) windowHandle, 
                    "setLevel:"_sel, NSFloatingWindowLevel);

        if(borderless) {
            unsigned long windowStyleMask = ((unsigned long (*)(id, SEL))objc_msgSend)(
                (id) windowHandle, "styleMask"_sel);
            windowStyleMask &= ~NSWindowStyleMaskTitled;
            ((void (*)(id, SEL, int))objc_msgSend)((id) windowHandle, 
                    "setStyleMask:"_sel, windowStyleMask);
        }

        if(maximize)
            ((void (*)(id, SEL, id))objc_msgSend)((id) windowHandle, 
                    "zoom:"_sel, NULL);
        
        ((void (*)(id, SEL, bool))objc_msgSend)((id) windowHandle, 
                    "setIsVisible:"_sel, !hidden);
        
        if(icon) {
            ((void (*)(id, SEL, id))objc_msgSend)(((id (*)(id, SEL))objc_msgSend)("NSApplication"_cls,
                                        "sharedApplication"_sel),
                        "setApplicationIconImage:"_sel, icon);
        }
        
        nativeWindow->navigate(url);
        nativeWindow->run();
    }
    #elif defined(_WIN32)
    void __createWindow(int height, int width,
        bool fullScreen, string title, bool alwaysOnTop, void* icon,
        bool enableInspector, bool borderless, bool maximize, bool hidden, string url) {

        nativeWindow = new webview::webview(enableInspector, nullptr);
        nativeWindow->set_title(title);
        nativeWindow->set_size(width, height, WEBVIEW_HINT_NONE);
        windowHandle = (HWND) nativeWindow->window();
        DWORD currentStyle = GetWindowLong(windowHandle, GWL_STYLE);
        DWORD currentStyleX = GetWindowLong(windowHandle, GWL_EXSTYLE);

        // Window properties/modes
        if(fullScreen) {
            MONITORINFO monitor_info;
            currentStyle &= ~(WS_CAPTION | WS_THICKFRAME);
            currentStyleX &= ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE |
                                WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
            SetWindowLong(windowHandle, GWL_STYLE, currentStyle);
            SetWindowLong(windowHandle, GWL_EXSTYLE, currentStyleX);
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
        }

        if(alwaysOnTop)
            SetWindowPos(windowHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );

        if(maximize)
            ShowWindow(windowHandle, SW_MAXIMIZE);

        if(hidden)
            ShowWindow(windowHandle, SW_HIDE);

        if(borderless) {
            currentStyle &= ~(WS_CAPTION | WS_THICKFRAME);
            SetWindowLong(windowHandle, GWL_STYLE, currentStyle);
            SetWindowPos(windowHandle, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |
                            SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
        }

        if(icon) {
            SendMessage(windowHandle, WM_SETICON, ICON_SMALL, (LPARAM)icon);
            SendMessage(windowHandle, WM_SETICON, ICON_BIG, (LPARAM)icon);
        }
        nativeWindow->navigate(url);
        nativeWindow->run();
    }
    #endif

    string setTitle(json input) {
        json output;
        string title = "";
        if(!input["title"].is_null())
            title = input["title"];
        nativeWindow->set_title(title);
        output["success"] = true;
        return output.dump();
    }

    string show(json input) {
        int width = 800;
        int height = 600;
        bool fullScreen = false;
        bool alwaysOnTop = false;
        bool enableInspector = false;
        bool borderless= false;
        bool maximize = false;
        bool hidden = false;
        string title = "Neutralinojs window";
        string url = "https://neutralino.js.org";
        json output;
        
        #if defined(__linux__)
        GdkPixbuf *icon = nullptr;
        #elif defined(__APPLE__)
        id icon = nullptr;
        #elif defined(_WIN32)
        GdiplusStartupInput gdiplusStartupInput;
        ULONG_PTR gdiplusToken;
        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
        HICON icon = nullptr;
        #endif

        if(!input["width"].is_null())
            width = input["width"];

        if(!input["height"].is_null())
            height = input["height"];

        if(!input["fullScreen"].is_null())
            fullScreen = input["fullScreen"];

        if(!input["alwaysOnTop"].is_null())
            alwaysOnTop = input["alwaysOnTop"];

        if(!input["title"].is_null())
            title = input["title"];

        if(!input["url"].is_null())
            url = input["url"];

        if(!input["icon"].is_null()) {
            #if defined(__linux__)
            GdkPixbufLoader *loader;
            GdkPixbuf *pixbuf;
            loader = gdk_pixbuf_loader_new();
            std::string iconFile = input["icon"].get<std::string>();
            std::string iconDataStr = settings::getFileContent(iconFile);

            const char * iconData = iconDataStr.c_str();
            unsigned char * uiconData = reinterpret_cast <unsigned char *> (const_cast <char *> (iconData));
            gdk_pixbuf_loader_write(loader, uiconData, iconDataStr.length(), NULL);
            icon = gdk_pixbuf_loader_get_pixbuf(loader);
            
            #elif defined(__APPLE__)
            string iconfile = input["icon"].get<std::string>();
            string iconDataStr = settings::getFileContent(iconfile);
            const char *iconData = iconDataStr.c_str();
            icon =
                ((id (*)(id, SEL))objc_msgSend)("NSImage"_cls, "alloc"_sel);
            
            id nsIconData = ((id (*)(id, SEL, const char*, int))objc_msgSend)("NSData"_cls,
                      "dataWithBytes:length:"_sel, iconData, iconDataStr.length());

            ((void (*)(id, SEL, id))objc_msgSend)(icon, "initWithData:"_sel, nsIconData);
            
            #elif defined(_WIN32)
            string iconfile = input["icon"].get<std::string>();
            string iconDataStr = settings::getFileContent(iconfile);
            const char *iconData = iconDataStr.c_str();
            unsigned char *uiconData = reinterpret_cast<unsigned char*>(const_cast<char*>(iconData));
            IStream *pStream = SHCreateMemStream((BYTE *) uiconData, iconDataStr.length());
            Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromStream(pStream);
            bitmap->GetHICON(&icon);
            pStream->Release();
            #endif
        }

        if (!input["enableInspector"].is_null())
            enableInspector = input["enableInspector"];

        if (!input["borderless"].is_null())
            borderless = input["borderless"];

        if (!input["maximize"].is_null())
            maximize = input["maximize"];

        if (!input["hidden"].is_null())
            hidden = input["hidden"];

        __createWindow(height, width,
            fullScreen, title, alwaysOnTop, icon,
            enableInspector, borderless, maximize, hidden, url);
        #if defined(_WIN32)
        GdiplusShutdown(gdiplusToken);
        #endif
        output["success"] = true;
        return output.dump();
    }

}
