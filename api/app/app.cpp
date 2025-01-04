#include <iostream>
#include <string>
#include <filesystem>

#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#include <signal.h>
#include <unistd.h>

#elif defined(_WIN32)
#define _WINSOCKAPI_
#include <windows.h>
#endif

#include "lib/json/json.hpp"
#include "settings.h"
#include "helpers.h"
#include "errors.h"
#include "server/neuserver.h"
#include "server/router.h"
#include "api/app/app.h"
#include "api/window/window.h"
#include "api/os/os.h"
#include "api/events/events.h"

using namespace std;
using json = nlohmann::json;

namespace app {

void exit(int code) {
    if(neuserver::isInitialized()) {
        neuserver::stop();
    }
    if(settings::getMode() == settings::AppModeWindow) {
        if(os::isTrayInitialized()) {
            os::cleanupTray();
        }
        window::_close(code);
    }
    else {
        std::exit(code);
    }
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
    if(helpers::hasField(input, "code")) {
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
    TerminateProcess(hnd, 137);
    CloseHandle(hnd);
    #endif
    return nullptr;
}

json getConfig(const json &input) {
    json output;
    output["returnValue"] = settings::getConfig();
    output["success"] = true;
    return output;
}

json broadcast(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"event"})) {
        output["error"] = errors::makeMissingArgErrorPayload();
        return output;
    }
    string event = input["event"].get<string>();
    json data = nullptr;

    if(helpers::hasField(input, "data")) {
        data = input["data"];
    }

    events::dispatchToAllApps(event, data);
    output["success"] = true;

    return output;
}

json mount(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"mountPath"}) || !helpers::hasRequiredFields(input, {"targetPath"})) {
        output["error"] = errors::makeMissingArgErrorPayload();
        return output;
    }
    std::string mountPath = input["mountPath"];
    std::string targetPath = input["targetPath"];

    // Normalize paths to have a leading and a trailing slash
    if (mountPath.back() != '/') {
        mountPath += '/';
    }
    if (mountPath.empty() || mountPath[0] != '/') {
        mountPath = '/' + mountPath;
    }
    if (targetPath.back() != '/') {
        targetPath += '/';
    }

    // Validate paths
    if (!std::filesystem::exists(targetPath)) {
        output["error"] = errors::makeErrorPayload(errors::NE_FS_NOPATHE, targetPath);
        return output;
    }
    if (!std::filesystem::is_directory(targetPath)) {
        output["error"] = errors::makeErrorPayload(errors::NE_FS_NOTADIR, targetPath);
        return output;
    }
    if (router::isMounted(mountPath)) {
        output["error"] = errors::makeErrorPayload(errors::NE_AP_MPINUSE, targetPath);
        return output;
    }

    router::mountPath(mountPath, targetPath);

    output["success"] = true;
    return output;
}

json unmount(const json &input) {
    json output;
    std::string mountPath = input["mountPath"];

    // Normalize path to have a leading and a trailing slash
    if (mountPath.back() != '/') {
        mountPath += '/';
    }
    if (mountPath.empty() || mountPath[0] != '/') {
        mountPath = '/' + mountPath;
    }

    // Validate if the mount path exists
    if (!router::isMounted(mountPath)) {
        output["error"] = errors::makeErrorPayload(errors::NE_AP_NOMTPTH, mountPath);
        return output;
    }

    router::unmountPath(mountPath);

    output["success"] = true;
    return output;
}

json readProcessInput(const json &input) {
    json output;
    string line, lines;
    bool readAll = false;

    if(helpers::hasField(input, "readAll")) {
        readAll = input["readAll"].get<bool>();
    }

    while(getline(cin, line) && !line.empty()) {
        lines += line;
        if(!readAll) {
            break;
        }
        lines += "\n";
    }
    output["returnValue"] = lines;
    output["success"] = true;
    return output;
}

json writeProcessOutput(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"data"})) {
        output["error"] = errors::makeMissingArgErrorPayload();
        return output;
    }

    cout << input["data"].get<string>() << flush;

    output["message"] = "Wrote data to stdout";
    output["success"] = true;
    return output;
}

json writeProcessError(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"data"})) {
        output["error"] = errors::makeMissingArgErrorPayload();
        return output;
    }

    cerr << input["data"].get<string>() << flush;

    output["message"] = "Wrote data to stderr";
    output["success"] = true;
    return output;
}

} // namespace controllers
} // namespace app
