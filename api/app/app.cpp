#include <iostream>

#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#include <signal.h>
#include <unistd.h>

#elif defined(_WIN32)
#include <Shlwapi.h>
#pragma comment (lib,"Shell32.lib")
#endif

#include "lib/json.hpp"
#include "server/ping.h"
#include "settings.h"
#include "api/app/app.h"

using namespace std;
using json = nlohmann::json;

namespace app {
    void exit() {
        #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
        kill(getpid(),SIGINT);
        #elif defined(_WIN32)
        DWORD pid = GetCurrentProcessId();
        HANDLE hnd = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, TRUE, pid);
        TerminateProcess(hnd, 0);
        #endif
    }
    
    void open(string url) {
        #if defined(__linux__) || defined(__FreeBSD__)
        int status = system(("xdg-open \"" + url + "\"").c_str());
        #elif defined(__APPLE__)
        system(("open \"" + url + "\"").c_str());
        #elif defined(_WIN32)
        ShellExecute(0, 0, url.c_str(), 0, 0, SW_SHOW );
        #endif
    }

namespace controllers {
    json exit(json input) {
        app::exit();
        return nullptr;
    }

    json keepAlive(json input) {
        json output;
        ping::receivePing();
        output["message"] = "Keep alive call was successful. Server will not be terminated automatically.";
        output["success"] = true;
        return output;
    }

    json getConfig(json input) {
        json output;
        output["returnValue"] = settings::getConfig();
        output["success"] = true;
        return output;
    }

    json open(json input) {
        json output;
        string url = input["url"];
        app::open(url);
        output["success"] = true;
        return output;
    }
    
} // namespace controllers
} // namespace app
