#include <thread>
#include <Shlwapi.h>
#include "lib/json.hpp"
#include "ping/ping.h"
#include "settings.h"

#pragma comment (lib,"Shell32.lib")

using namespace std;
using json = nlohmann::json;

namespace app {

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

    string open(json input) {
        json output;
        string url = input["url"];
        ShellExecute(0, 0, url.c_str(), 0, 0, SW_SHOW );
        output["success"] = true;
        return output.dump();
    }

}
