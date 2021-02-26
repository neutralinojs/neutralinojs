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

#include <thread>
#include <winsock2.h>
#include <windows.h>
#include <gdiplus.h>
#include <Shlwapi.h>
#include "settings.h"
#include "resources.h"
#include "auth/authbasic.h"
#include "ping/ping.h"
#include "cloud/privileges.h"
#include "webview/webview_c.h"
#include "lib/json.hpp"
#include "server/serverlistener.h"


using namespace std;
using namespace Gdiplus;
using json = nlohmann::json;

void uiThread(string appname, string port, int width, int height, bool fullscreen, string title, bool always_on_top, bool borderless, void *icon, bool maximize, string url) {
      web_view(title.c_str(), url.c_str(), width, height, fullscreen, always_on_top, borderless, maximize, icon);
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow) {
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    json args;
    for(int i = 0; i < __argc; i++) {
        args.push_back(__argv[i]);
    }
    settings::setGlobalArgs(args);
    if(!loadResFromDir)
        resources::makeFileTree();
    settings::getSettings();
    authbasic::generateToken();
    ping::startPingReceiver();
    privileges::getBlacklist();
    
    json options = settings::getOptions();
    string appname = options["appname"];
    string appport = options["appport"];
    string mode = settings::getMode();
    ServerListener serverListener(stoi(appport));

    serverListener.init();

    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(serverListener.listen_socket, (struct sockaddr *)&sin, &len) == -1) {
        perror("getsockname");
    }
    else {
        int port = ntohs(sin.sin_port);
        settings::setOption("appport", std::to_string(port));
        appport = std::to_string(port);
    }
    string navigateUrl = "http://localhost:" + appport + "/" + appname;
    if(!options["url"].is_null() && options["url"].get<string>() != "/")
        navigateUrl = options["url"];

    if(mode == "browser") {
        ShellExecute(0, 0, navigateUrl.c_str(), 0, 0 , SW_SHOW );
    }
    else if(mode == "window"){
        int width = 800;
        int height = 600;
        bool fullscreen = false;
        bool is_always_on_top = false;
        bool is_borderless_window = false;
        HICON icon = nullptr; 
        string title = "Neutralino window";
        bool maximize = false;

        if(!options["window"].is_null()) {
            json windowProp = options["window"];
            width =  stoi(windowProp["width"].get<std::string>());
            height =  stoi(windowProp["height"].get<std::string>());
            if(!windowProp["fullscreen"].is_null())
                fullscreen =  windowProp["fullscreen"].get<bool>();
            
            if(!windowProp["alwaysontop"].is_null())
                is_always_on_top = windowProp["alwaysontop"].get<bool>();

            if(!windowProp["borderless"].is_null())
                is_borderless_window = windowProp["borderless"].get<bool>();
            
            if(!windowProp["iconfile"].is_null()) {
                string iconfile = windowProp["iconfile"].get<std::string>();
                string iconDataStr = settings::getFileContent(iconfile);
                const char *iconData = iconDataStr.c_str();
                unsigned char *uiconData = reinterpret_cast<unsigned char*>(const_cast<char*>(iconData));
                IStream *pStream = SHCreateMemStream((BYTE *) uiconData, iconDataStr.length());
                Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromStream(pStream);
                bitmap->GetHICON(&icon);
                pStream->Release();
            }

            if(!windowProp["title"].is_null())
                title = windowProp["title"].get<std::string>();
            
            if (!windowProp["maximize"].is_null())
                maximize = windowProp["maximize"].get<bool>();
        }
        std::thread ren(uiThread, appname, appport, width, height, fullscreen, title, is_always_on_top, is_borderless_window, icon, maximize, navigateUrl);
        ren.detach();
    }
    serverListener.run();
    GdiplusShutdown(gdiplusToken);
    return 0;
}

