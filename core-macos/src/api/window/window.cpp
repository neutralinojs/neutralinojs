#include <iostream>
#include <signal.h>
#include <unistd.h>
#include "lib/json.hpp"
#include "ping/ping.h"
#include "settings.h"
#define WEBVIEW_IMPLEMENTATION
#include "lib/webview/webview.h"

#define NSFloatingWindowLevel 5

using namespace std;
using json = nlohmann::json;

// Helpers to avoid too much typing
id operator"" _cls(const char *s, std::size_t) { return (id)objc_getClass(s); }
SEL operator"" _sel(const char *s, std::size_t) { return sel_registerName(s); }
id operator"" _str(const char *s, std::size_t) {
  return ((id(*)(id, SEL, const char *))objc_msgSend)(
      "NSString"_cls, "stringWithUTF8String:"_sel, s);
}

webview::webview *nativeWindow;
id windowHandle;

namespace window {

    void __createWindow(int height, int width,
        bool fullScreen, string title, bool alwaysOnTop, id icon,
        bool enableInspector, bool borderless, bool maximize, bool hidden, string url) {
        nativeWindow = new webview::webview(enableInspector, nullptr);
        nativeWindow->set_title(title);
        nativeWindow->set_size(width, height, WEBVIEW_HINT_NONE);
        windowHandle = (id) nativeWindow->window();

        // Window properties/modes
        if(fullScreen)
            ((void (*)(id, SEL, id))objc_msgSend)((id) windowHandle, 
                    sel_registerName("toggleFullScreen:"), NULL);
        
        if(alwaysOnTop)
            ((void (*)(id, SEL, int))objc_msgSend)((id) windowHandle, 
                    sel_registerName("setLevel:"), NSFloatingWindowLevel);

        if(borderless) {
            unsigned long windowStyleMask = ((unsigned long (*)(id, SEL))objc_msgSend)(
                (id) windowHandle, sel_registerName("styleMask"));
            windowStyleMask &= ~NSWindowStyleMaskTitled;
            ((void (*)(id, SEL, int))objc_msgSend)((id) windowHandle, 
                    sel_registerName("setStyleMask:"), windowStyleMask);
        }

        if(maximize)
            ((void (*)(id, SEL, id))objc_msgSend)((id) windowHandle, 
                    sel_registerName("zoom:"), NULL);
        
        ((void (*)(id, SEL, bool))objc_msgSend)((id) windowHandle, 
                    sel_registerName("setIsVisible:"), !hidden);
        
        if(icon) {
            ((void (*)(id, SEL, id))objc_msgSend)(((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSApplication"),
                                        sel_registerName("sharedApplication")),
                        sel_registerName("setApplicationIconImage:"), icon);
        }
        
        nativeWindow->navigate(url);
        nativeWindow->run();
    }

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
        id icon = nullptr;
        string title = "Neutralinojs window";
        string url = "https://neutralino.js.org";
        json output;

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
            string iconfile = input["icon"].get<std::string>();
            string iconDataStr = settings::getFileContent(iconfile);
            const char *iconData = iconDataStr.c_str();
            icon =
                ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSImage"), sel_registerName("alloc"));
            
            id nsIconData = ((id (*)(id, SEL, const char*, int))objc_msgSend)((id)objc_getClass("NSData"),
                      sel_registerName("dataWithBytes:length:"), iconData, iconDataStr.length());

            ((void (*)(id, SEL, id))objc_msgSend)(icon, sel_registerName("initWithData:"), nsIconData);
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
        output["success"] = true;
        return output.dump();
    }

}
