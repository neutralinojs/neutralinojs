// MIT License

// Copyright (c) 2018 Neutralinojs

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <iostream>
#include <thread>
#include <signal.h>
#include <unistd.h>
#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "lib/json.hpp"
#include "ping/ping.h"
#include "settings.h"
#define WEBVIEW_IMPLEMENTATION
#include "webview/webview.h"

using namespace std;
using json = nlohmann::json;

namespace app {

    void __showWindow(int height, int width,
        bool fullScreen, string title, bool alwaysOnTop, void* icon,
        bool enableInspector, bool borderless, bool maximize, string url) {
        struct webview webview;
        memset( & webview, 0, sizeof(webview));
        webview.title = title.c_str();
        webview.url = url.c_str();
        webview.width = width;
        webview.height = height;
        webview.resizable = 1;
        webview.alwaysOnTop = alwaysOnTop;
        webview.icon = icon;
        webview.debug = enableInspector;
        webview.borderless= borderless;
        webview.maximize = maximize;
        int r = webview_init( & webview);
        webview_set_fullscreen( & webview, fullScreen);
        if (r != 0) {
            return;
        }
        while (webview_loop( & webview, 1) == 0) {}
        webview_exit( & webview);
    }

    string exit(json input) {
        kill(getpid(),SIGINT);
        return "";
    }

    string keepAlive(json input) {
        json output;
        ping::receivePing();
        output["message"] = "Keep alive call was successful. Server will not be terminated automatically.";
        output["success"] = true;
        return output.dump();
    }

    string showWindow(json input) {
        int width = 800;
        int height = 600;
        bool fullScreen = false;
        bool alwaysOnTop = false;
        bool enableInspector = false;
        bool borderless= false;
        bool maximize = false;
        GdkPixbuf *icon = nullptr;
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
            title = input["title"].get<std::string>();

        if(!input["url"].is_null())
            url = input["url"].get<std::string>();

        if(!input["icon"].is_null()) {
            GdkPixbufLoader *loader;
            GdkPixbuf *pixbuf;
            loader = gdk_pixbuf_loader_new();
            std::string iconFile = input["icon"].get<std::string>();
            std::string iconDataStr = settings::getFileContent(iconFile);

            const char * iconData = iconDataStr.c_str();
            unsigned char * uiconData = reinterpret_cast <unsigned char *> (const_cast <char *> (iconData));
            gdk_pixbuf_loader_write(loader, uiconData, iconDataStr.length(), NULL);
            icon = gdk_pixbuf_loader_get_pixbuf(loader);
        }

        if (!input["enableInspector"].is_null())
            enableInspector = input["enableInspector"];

        if (!input["borderless"].is_null())
            borderless = input["borderless"];

        if (!input["maximize"].is_null())
            maximize = input["maximize"];

        thread uiThread(__showWindow, height, width,
            fullScreen, title, alwaysOnTop, icon,
            enableInspector, borderless, maximize, url);
        uiThread.detach();
        output["success"] = true;
        return output.dump();
    }

    string open(json input) {
        json output;
        string url = input["url"];
        system(("xdg-open \"" + url + "\"").c_str());
        output["success"] = true;
        return output.dump();
    }

}
