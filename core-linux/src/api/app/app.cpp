#include <iostream>
#include <thread>
#include <signal.h>
#include <unistd.h>
#include <gtk/gtk.h>
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
        webview::webview nativeWindow(enableInspector, nullptr);
        nativeWindow.set_title(title);
        nativeWindow.set_size(width, height, WEBVIEW_HINT_NONE);
        GtkWidget* windowHandle = (GtkWidget*) nativeWindow.window();

        // Window properties/modes
        if(fullScreen) {
            gtk_window_fullscreen(GTK_WINDOW(windowHandle));
        }
        gtk_window_set_keep_above(GTK_WINDOW(windowHandle), alwaysOnTop);

        if(maximize) {
            gtk_window_maximize(GTK_WINDOW(windowHandle));
        }
        gtk_window_set_decorated(GTK_WINDOW(windowHandle), !borderless);

        if(icon) {
            gtk_window_set_icon(GTK_WINDOW(windowHandle), (GdkPixbuf*)icon);
        }
        nativeWindow.navigate(url);
        nativeWindow.run();
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

    string getConfig(json input) {
        json output;
        output["config"] = settings::getConfig();
        output["success"] = true;
        return output.dump();
    }

    // NOTEXPOSED: This is not exposed to the client.
    // Second time it crashes the process.
    // Check with the latest version of webview lib.
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
            title = input["title"];

        if(!input["url"].is_null())
            url = input["url"];

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
