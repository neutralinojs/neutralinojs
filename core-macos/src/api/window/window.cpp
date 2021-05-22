#include <iostream>
#include <signal.h>
#include <unistd.h>
#include "lib/json.hpp"
#include "ping/ping.h"
#include "settings.h"
#define WEBVIEW_IMPLEMENTATION
#include "lib/webview/webview.h"

using namespace std;
using json = nlohmann::json;

webview::webview *nativeWindow;
// TODO: Implement for macos

namespace window {

    void __createWindow(int height, int width,
        bool fullScreen, string title, bool alwaysOnTop, void* icon,
        bool enableInspector, bool borderless, bool maximize, bool hidden, string url) {
        nativeWindow = new webview::webview(enableInspector, nullptr);
        nativeWindow->set_title(title);
        nativeWindow->set_size(width, height, WEBVIEW_HINT_NONE);

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
        void *icon = nullptr;
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
