#include <iostream>

#if defined(__linux__) || defined(__FreeBSD__)
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
#if defined(__linux__) || defined(__FreeBSD__)
GtkWidget* windowHandle;
#elif defined(__APPLE__)
id windowHandle;
#elif defined(_WIN32)
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
    #if defined(__linux__) || defined(__FreeBSD__)
    GdkPixbuf *icon = nullptr;
    #elif defined(__APPLE__)
    id icon = nullptr;
    #elif defined(_WIN32)
    HICON icon = nullptr;
    #endif
};

namespace window {
    #if defined(__linux__) || defined(__FreeBSD__)
    void __createWindow(WindowOptions windowProps) {
        nativeWindow = new webview::webview(windowProps.enableInspector, nullptr);
        nativeWindow->set_title(windowProps.title);
        nativeWindow->set_size(windowProps.width, windowProps.height, windowProps.minWidth,
                        windowProps.minHeight, windowProps.maxWidth, windowProps.maxHeight, 
                        windowProps.resizable);
        windowHandle = (GtkWidget*) nativeWindow->window();

        // Window properties/modes
        if(windowProps.fullScreen)
            gtk_window_fullscreen(GTK_WINDOW(windowHandle));

        gtk_window_set_keep_above(GTK_WINDOW(windowHandle), windowProps.alwaysOnTop);

        if(windowProps.maximize)
            gtk_window_maximize(GTK_WINDOW(windowHandle));
 
        if(windowProps.hidden)
            gtk_widget_hide(windowHandle);

        gtk_window_set_decorated(GTK_WINDOW(windowHandle), !windowProps.borderless);

        if(windowProps.icon)
            gtk_window_set_icon(GTK_WINDOW(windowHandle), (GdkPixbuf*)windowProps.icon);
        
        nativeWindow->navigate(windowProps.url);
        nativeWindow->run();
    }
    #elif defined(__APPLE__)
    void __createWindow(WindowOptions windowProps) {
        nativeWindow = new webview::webview(windowProps.enableInspector, nullptr);
        nativeWindow->set_title(windowProps.title);
        nativeWindow->set_size(windowProps.width, windowProps.height, windowProps.minWidth,
                        windowProps.minHeight, windowProps.maxWidth, windowProps.maxHeight, 
                        windowProps.resizable);
        windowHandle = (id) nativeWindow->window();

        // Window properties/modes
        ((void (*)(id, SEL, bool))objc_msgSend)((id) windowHandle, 
                    "setHasShadow:"_sel, true);

        if(windowProps.fullScreen)
            ((void (*)(id, SEL, id))objc_msgSend)((id) windowHandle, 
                    "toggleFullScreen:"_sel, NULL);
        
        if(windowProps.alwaysOnTop)
            ((void (*)(id, SEL, int))objc_msgSend)((id) windowHandle, 
                    "setLevel:"_sel, NSFloatingWindowLevel);

        if(windowProps.borderless) {
            unsigned long windowStyleMask = ((unsigned long (*)(id, SEL))objc_msgSend)(
                (id) windowHandle, "styleMask"_sel);
            windowStyleMask &= ~NSWindowStyleMaskTitled;
            ((void (*)(id, SEL, int))objc_msgSend)((id) windowHandle, 
                    "setStyleMask:"_sel, windowStyleMask);
        }

        if(windowProps.maximize)
            ((void (*)(id, SEL, id))objc_msgSend)((id) windowHandle, 
                    "zoom:"_sel, NULL);
        
        ((void (*)(id, SEL, bool))objc_msgSend)((id) windowHandle, 
                    "setIsVisible:"_sel, !windowProps.hidden);
        
        if(windowProps.icon) {
            ((void (*)(id, SEL, id))objc_msgSend)(((id (*)(id, SEL))objc_msgSend)("NSApplication"_cls,
                                        "sharedApplication"_sel),
                        "setApplicationIconImage:"_sel, windowProps.icon);
        }
        
        nativeWindow->navigate(windowProps.url);
        nativeWindow->run();
    }
    #elif defined(_WIN32)
    void __createWindow(WindowOptions windowProps) {

        nativeWindow = new webview::webview(windowProps.enableInspector, nullptr);
        nativeWindow->set_title(windowProps.title);
        nativeWindow->set_size(windowProps.width, windowProps.height, windowProps.minWidth,
                        windowProps.minHeight, windowProps.maxWidth, windowProps.maxHeight, 
                        windowProps.resizable);
        windowHandle = (HWND) nativeWindow->window();
        DWORD currentStyle = GetWindowLong(windowHandle, GWL_STYLE);
        DWORD currentStyleX = GetWindowLong(windowHandle, GWL_EXSTYLE);

        // Window properties/modes
        if(windowProps.fullScreen) {
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

        if(windowProps.alwaysOnTop)
            SetWindowPos(windowHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );

        if(windowProps.maximize)
            ShowWindow(windowHandle, SW_MAXIMIZE);

        if(windowProps.hidden)
            ShowWindow(windowHandle, SW_HIDE);

        if(windowProps.borderless) {
            currentStyle &= ~(WS_CAPTION | WS_THICKFRAME);
            SetWindowLong(windowHandle, GWL_STYLE, currentStyle);
            SetWindowPos(windowHandle, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |
                            SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
        }

        if(windowProps.icon) {
            SendMessage(windowHandle, WM_SETICON, ICON_SMALL, (LPARAM)windowProps.icon);
            SendMessage(windowHandle, WM_SETICON, ICON_BIG, (LPARAM)windowProps.icon);
        }
        nativeWindow->navigate(windowProps.url);
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
        WindowOptions windowProps;
        json output;
        #if defined(_WIN32)
        GdiplusStartupInput gdiplusStartupInput;
        ULONG_PTR gdiplusToken;
        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
        #endif

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

        if(!input["icon"].is_null()) {
            #if defined(__linux__) || defined(__FreeBSD__)
            GdkPixbufLoader *loader;
            GdkPixbuf *pixbuf;
            loader = gdk_pixbuf_loader_new();
            std::string iconFile = input["icon"].get<std::string>();
            std::string iconDataStr = settings::getFileContent(iconFile);

            const char * iconData = iconDataStr.c_str();
            unsigned char * uiconData = reinterpret_cast <unsigned char *> (const_cast <char *> (iconData));
            gdk_pixbuf_loader_write(loader, uiconData, iconDataStr.length(), NULL);
            windowProps.icon = gdk_pixbuf_loader_get_pixbuf(loader);
            
            #elif defined(__APPLE__)
            string iconfile = input["icon"].get<std::string>();
            string iconDataStr = settings::getFileContent(iconfile);
            const char *iconData = iconDataStr.c_str();
            windowProps.icon =
                ((id (*)(id, SEL))objc_msgSend)("NSImage"_cls, "alloc"_sel);
            
            id nsIconData = ((id (*)(id, SEL, const char*, int))objc_msgSend)("NSData"_cls,
                      "dataWithBytes:length:"_sel, iconData, iconDataStr.length());

            ((void (*)(id, SEL, id))objc_msgSend)(windowProps.icon, "initWithData:"_sel, nsIconData);
            
            #elif defined(_WIN32)
            string iconfile = input["icon"].get<std::string>();
            string iconDataStr = settings::getFileContent(iconfile);
            const char *iconData = iconDataStr.c_str();
            unsigned char *uiconData = reinterpret_cast<unsigned char*>(const_cast<char*>(iconData));
            IStream *pStream = SHCreateMemStream((BYTE *) uiconData, iconDataStr.length());
            Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromStream(pStream);
            bitmap->GetHICON(&windowProps.icon);
            pStream->Release();
            #endif
        }

        if (!input["enableInspector"].is_null())
            windowProps.enableInspector = input["enableInspector"];

        if (!input["borderless"].is_null())
            windowProps.borderless = input["borderless"];

        if (!input["maximize"].is_null())
            windowProps.maximize = input["maximize"];

        if (!input["hidden"].is_null())
            windowProps.hidden = input["hidden"];
            
        if (!input["resizable"].is_null())
            windowProps.resizable = input["resizable"];

        __createWindow(windowProps);
        #if defined(_WIN32)
        GdiplusShutdown(gdiplusToken);
        #endif
        output["success"] = true;
        return output.dump();
    }

}
