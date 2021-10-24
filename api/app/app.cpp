#include <iostream>
#include <string>

#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#include <signal.h>
#include <unistd.h>

#elif defined(_WIN32)
#define _WINSOCKAPI_
#include <windows.h>
#endif

#include "lib/json/json.hpp"
#include "settings.h"
#include "server/ping.h"
#include "server/neuserver.h"
#include "api/app/app.h"
#include "api/window/window.h"

using namespace std;
using json = nlohmann::json;

namespace app {
    void exit(int code) {
        json jEnableServer = settings::getOptionForCurrentMode("enableServer");
        if(!jEnableServer.is_null() && jEnableServer.get<bool>()) {
            neuserver::stop();
        }
        if(settings::getMode() == "window")
            window::_close(code);
        else 
            std::exit(code);
    }
    
    unsigned int getProcessId() {
        #if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)
        return getpid();
        #elif defined(_WIN32)
        return GetCurrentProcessId();
        #endif
    }

namespace controllers {
    json exit(const json &input) {
        int code = 0;
        if(input.contains("code")) {
            code = input["code"].get<int>();
        }
        app::exit(code);
        return nullptr;
    }
    
    json killProcess(const json &input) {
        #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
        kill(getpid(),SIGINT);
        #elif defined(_WIN32)
        DWORD pid = GetCurrentProcessId();
        HANDLE hnd = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, TRUE, pid);
        TerminateProcess(hnd, 0);
        #endif
        return nullptr;
    }

    json keepAlive(const json &input) {
        json output;
        ping::pingReceived();
        output["message"] = "Keep alive call was successful. Server will not be terminated automatically.";
        output["success"] = true;
        return output;
    }

    json getConfig(const json &input) {
        json output;
        output["returnValue"] = settings::getConfig();
        output["success"] = true;
        return output;
    }
    
} // namespace controllers
} // namespace app
