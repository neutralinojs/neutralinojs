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
#include <windows.h>
#include <gdiplus.h>
#include <Shlwapi.h>
#include "lib/json.hpp"
#include "ping/ping.h"
#include "settings.h"
#include "webview/webview_c.h"

using namespace std;
using namespace Gdiplus;
using json = nlohmann::json;

namespace app {

    void __showWindow(int height, int width,
        bool fullScreen, string title, bool alwaysOnTop, void* icon,
        bool enableInspector, bool borderless, bool maximize, string url) {
        web_view(title.c_str(), url.c_str(), width, height, fullScreen, alwaysOnTop,
                borderless, maximize, icon);
    }

    string exit(json input) {
        DWORD pid = GetCurrentProcessId();
        HANDLE hnd = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, TRUE, pid);
        TerminateProcess(hnd, 0);
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
        GdiplusStartupInput gdiplusStartupInput;
        ULONG_PTR gdiplusToken;
        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
        int width = 800;
        int height = 600;
        bool fullScreen = false;
        bool alwaysOnTop = false;
        bool enableInspector = false;
        bool borderless= false;
        bool maximize = false;
        HICON icon = nullptr;
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
            unsigned char *uiconData = reinterpret_cast<unsigned char*>(const_cast<char*>(iconData));
            IStream *pStream = SHCreateMemStream((BYTE *) uiconData, iconDataStr.length());
            Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromStream(pStream);
            bitmap->GetHICON(&icon);
            pStream->Release();
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
        GdiplusShutdown(gdiplusToken);
        output["success"] = true;
        return output.dump();
    }

    string open(json input) {
        json output;
        string url = input["url"];
        ShellExecute(0, 0, url.c_str(), 0, 0, SW_SHOW );
        output["success"] = true;
        return output.dump();
    }

}