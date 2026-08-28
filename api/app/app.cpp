#include <iostream>
#include <string>
#include <filesystem>
#include <cctype>

#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#include <cerrno>
#include <fcntl.h>
#include <signal.h>
#include <sys/file.h>
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
#include "api/app/app.h"
#include "api/window/window.h"
#include "api/os/os.h"
#include "api/events/events.h"

using namespace std;
using json = nlohmann::json;

namespace app {

#if defined(_WIN32)
HANDLE singleInstanceMutex = nullptr;
#else
int singleInstanceLockFd = -1;
#endif
bool singleInstanceChecked = false;
bool singleInstanceFirst = false;

string getSingleInstanceId() {
    string appId = "neutralinojs-default";
    json config = settings::getConfig();

    if(helpers::hasField(config, "applicationId") && config["applicationId"].is_string()) {
        appId = config["applicationId"].get<string>();
    }

    string safeId;
    for(char ch: appId) {
        unsigned char uch = static_cast<unsigned char>(ch);
        if(isalnum(uch) || ch == '-' || ch == '_' || ch == '.') {
            safeId += ch;
        }
        else {
            safeId += '_';
        }
    }

    if(safeId.empty()) {
        safeId = "neutralinojs-default";
    }

    return safeId;
}

void releaseSingleInstanceLock() {
    #if defined(_WIN32)
    if(singleInstanceMutex) {
        ReleaseMutex(singleInstanceMutex);
        CloseHandle(singleInstanceMutex);
        singleInstanceMutex = nullptr;
    }
    #else
    if(singleInstanceLockFd != -1) {
        flock(singleInstanceLockFd, LOCK_UN);
        close(singleInstanceLockFd);
        singleInstanceLockFd = -1;
    }
    #endif
}

void exit(int code) {
    releaseSingleInstanceLock();
    if(neuserver::isInitialized()) {
        neuserver::stop();
    }
    if(settings::getMode() == settings::AppModeWindow) {
        os::cleanupTray();
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

bool singleInstance() {
    if(singleInstanceChecked) {
        return singleInstanceFirst;
    }

    singleInstanceChecked = true;
    string lockName = "neutralinojs-single-instance-" + getSingleInstanceId();

    #if defined(_WIN32)
    wstring mutexName = helpers::str2wstr("Local\\" + lockName);
    singleInstanceMutex = CreateMutexW(nullptr, TRUE, mutexName.c_str());

    if(!singleInstanceMutex) {
        singleInstanceFirst = false;
        return singleInstanceFirst;
    }

    singleInstanceFirst = GetLastError() != ERROR_ALREADY_EXISTS;
    if(!singleInstanceFirst) {
        CloseHandle(singleInstanceMutex);
        singleInstanceMutex = nullptr;
    }

    return singleInstanceFirst;
    #else
    filesystem::path lockPath = filesystem::temp_directory_path() / (lockName + ".lock");
    singleInstanceLockFd = open(lockPath.c_str(), O_RDWR | O_CREAT, 0600);

    if(singleInstanceLockFd == -1) {
        singleInstanceFirst = false;
        return singleInstanceFirst;
    }

    if(flock(singleInstanceLockFd, LOCK_EX | LOCK_NB) == -1) {
        if(errno == EWOULDBLOCK || errno == EAGAIN) {
            close(singleInstanceLockFd);
            singleInstanceLockFd = -1;
            singleInstanceFirst = false;
            return singleInstanceFirst;
        }

        close(singleInstanceLockFd);
        singleInstanceLockFd = -1;
        singleInstanceFirst = false;
        return singleInstanceFirst;
    }

    string pid = to_string(app::getProcessId()) + "\n";
    ftruncate(singleInstanceLockFd, 0);
    write(singleInstanceLockFd, pid.c_str(), pid.size());

    singleInstanceFirst = true;
    return singleInstanceFirst;
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
    os::cleanupTray();
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

json getProcessId(const json &input) {
    json output;
    output["returnValue"] = app::getProcessId();
    output["success"] = true;
    return output;
}

json singleInstance(const json &input) {
    json output;
    output["isFirstInstance"] = app::singleInstance();
    output["success"] = true;
    return output;
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
        output["error"] = errors::makeMissingArgErrorPayload("event");
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
        output["error"] = errors::makeMissingArgErrorPayload("data");
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
        output["error"] = errors::makeMissingArgErrorPayload("data");
        return output;
    }

    cerr << input["data"].get<string>() << flush;

    output["message"] = "Wrote data to stderr";
    output["success"] = true;
    return output;
}

} // namespace controllers
} // namespace app
