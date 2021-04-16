#include <thread>
#include <windows.h>
#include <gdiplus.h>
#include <Shlwapi.h>
#include "lib/json.hpp"
#include "ping/ping.h"
#include "settings.h"
#include "webview/webview.h"

#pragma comment (lib,"Gdiplus.lib")
#pragma comment (lib,"Shell32.lib")
#pragma comment(lib, "WebView2Loader.dll.lib")


using namespace std;
using namespace Gdiplus;
using json = nlohmann::json;

namespace app {

    void __showWindow(int height, int width,
        bool fullScreen, string title, bool alwaysOnTop, void* icon,
        bool enableInspector, bool borderless, bool maximize, string url) {

        webview::webview nativeWindow(enableInspector, nullptr);
        nativeWindow.set_title(title);
        nativeWindow.set_size(width, height, WEBVIEW_HINT_NONE);
        HWND windowHandle = (HWND) nativeWindow.window();
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

        if(alwaysOnTop) {
            SetWindowPos(windowHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
        }

        if(maximize) {
            ShowWindow(windowHandle, SW_MAXIMIZE);
        }

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
        nativeWindow.navigate(url);
        nativeWindow.run();
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
