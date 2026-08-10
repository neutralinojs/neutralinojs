#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <regex>
#include <cstdlib>
#include <cstring>

#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#include <climits>
#include <signal.h>
#include <unistd.h>

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

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

namespace {

string __sanitizeAutoStartName(string name) {
    if(name.empty()) {
        name = settings::getAppId();
    }
    name = regex_replace(name, regex("[^A-Za-z0-9._-]"), "_");
    return name.empty() ? "neutralinojs" : name;
}

string __escapeXml(const string &value) {
    string escaped;
    for(char ch: value) {
        switch(ch) {
            case '&': escaped += "&amp;"; break;
            case '<': escaped += "&lt;"; break;
            case '>': escaped += "&gt;"; break;
            case '"': escaped += "&quot;"; break;
            case '\'': escaped += "&apos;"; break;
            default: escaped += ch;
        }
    }
    return escaped;
}

string __quoteDesktopArg(const string &value) {
    string quoted = "\"";
    for(char ch: value) {
        if(ch == '"' || ch == '\\' || ch == '`' || ch == '$') {
            quoted += '\\';
        }
        if(ch == '%') {
            quoted += '%';
        }
        quoted += ch;
    }
    quoted += "\"";
    return quoted;
}

string __quoteCommandArg(const string &value) {
    if(value.empty()) {
        return "\"\"";
    }

    bool shouldQuote = value.find_first_of(" \t\n\v\"") != string::npos;
    if(!shouldQuote) {
        return value;
    }

    string quoted = "\"";
    unsigned int backslashes = 0;
    for(char ch: value) {
        if(ch == '\\') {
            backslashes++;
            continue;
        }
        if(ch == '"') {
            quoted.append(backslashes * 2 + 1, '\\');
            quoted += ch;
            backslashes = 0;
            continue;
        }
        quoted.append(backslashes, '\\');
        backslashes = 0;
        quoted += ch;
    }
    quoted.append(backslashes * 2, '\\');
    quoted += "\"";
    return quoted;
}

string __getExecutablePath() {
    #if defined(_WIN32)
    wchar_t path[MAX_PATH];
    DWORD size = GetModuleFileNameW(nullptr, path, MAX_PATH);
    return size > 0 ? helpers::wstr2str(wstring(path, size)) : "";
    #elif defined(__APPLE__)
    char path[PATH_MAX];
    uint32_t size = sizeof(path);
    if(_NSGetExecutablePath(path, &size) != 0) {
        return "";
    }
    char resolvedPath[PATH_MAX];
    return realpath(path, resolvedPath) == nullptr ? string(path) : string(resolvedPath);
    #elif defined(__linux__)
    char path[PATH_MAX];
    ssize_t size = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if(size == -1) {
        return "";
    }
    path[size] = '\0';
    return string(path);
    #else
    return "";
    #endif
}

string __getHomePath() {
    const char *home = getenv("HOME");
    return home == nullptr ? "" : string(home);
}

} // namespace

void exit(int code) {
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

bool setAutoStart(const AutoStartOptions &options) {
    string name = __sanitizeAutoStartName(options.name);
    string path = options.path.empty() ? __getExecutablePath() : options.path;

    if(options.enabled && path.empty()) {
        return false;
    }

    #ifdef _WIN32
    const wchar_t *runKeyPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    const wchar_t *startupApprovedRunKeyPath =
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run";
    wstring keyName = helpers::str2wstr(name);

    if(options.enabled) {
        HKEY runKey;
        if(RegCreateKeyExW(HKEY_CURRENT_USER, runKeyPath, 0, nullptr, 0,
                KEY_SET_VALUE, nullptr, &runKey, nullptr) != ERROR_SUCCESS) {
            return false;
        }

        string command = __quoteCommandArg(path);
        for(const string &arg: options.args) {
            command += " " + __quoteCommandArg(arg);
        }
        wstring commandW = helpers::str2wstr(command);
        LONG result = RegSetValueExW(runKey, keyName.c_str(), 0, REG_SZ,
            reinterpret_cast<const BYTE*>(commandW.c_str()),
            static_cast<DWORD>((commandW.size() + 1) * sizeof(wchar_t)));
        RegCloseKey(runKey);

        HKEY startupApprovedRunKey;
        if(RegOpenKeyExW(HKEY_CURRENT_USER, startupApprovedRunKeyPath, 0,
                KEY_SET_VALUE, &startupApprovedRunKey) == ERROR_SUCCESS) {
            RegDeleteValueW(startupApprovedRunKey, keyName.c_str());
            RegCloseKey(startupApprovedRunKey);
        }
        return result == ERROR_SUCCESS;
    }

    LONG runResult = ERROR_FILE_NOT_FOUND;
    HKEY runKey;
    if(RegOpenKeyExW(HKEY_CURRENT_USER, runKeyPath, 0,
            KEY_SET_VALUE, &runKey) == ERROR_SUCCESS) {
        runResult = RegDeleteValueW(runKey, keyName.c_str());
        RegCloseKey(runKey);
    }

    LONG startupApprovedResult = ERROR_FILE_NOT_FOUND;
    HKEY startupApprovedRunKey;
    if(RegOpenKeyExW(HKEY_CURRENT_USER, startupApprovedRunKeyPath, 0,
            KEY_SET_VALUE, &startupApprovedRunKey) == ERROR_SUCCESS) {
        startupApprovedResult = RegDeleteValueW(startupApprovedRunKey, keyName.c_str());
        RegCloseKey(startupApprovedRunKey);
    }
    return (runResult == ERROR_SUCCESS || runResult == ERROR_FILE_NOT_FOUND) &&
        (startupApprovedResult == ERROR_SUCCESS || startupApprovedResult == ERROR_FILE_NOT_FOUND);

    #elif __APPLE__
    string homePath = __getHomePath();
    if(homePath.empty()) {
        return false;
    }
    filesystem::path agentDir = filesystem::path(homePath) / "Library" / "LaunchAgents";
    filesystem::path agentPath = agentDir / (name + ".plist");

    try {
        if(!options.enabled) {
            return !filesystem::exists(agentPath) || filesystem::remove(agentPath);
        }

        filesystem::create_directories(agentDir);
        ofstream plist(agentPath);
        if(!plist) {
            return false;
        }
        plist << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              << "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" "
              << "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
              << "<plist version=\"1.0\">\n"
              << "<dict>\n"
              << "    <key>Label</key>\n"
              << "    <string>" << __escapeXml(name) << "</string>\n"
              << "    <key>ProgramArguments</key>\n"
              << "    <array>\n"
              << "        <string>" << __escapeXml(path) << "</string>\n";
        for(const string &arg: options.args) {
            plist << "        <string>" << __escapeXml(arg) << "</string>\n";
        }
        plist << "    </array>\n"
              << "    <key>RunAtLoad</key>\n"
              << "    <true/>\n"
              << "</dict>\n"
              << "</plist>\n";
        return plist.good();
    }
    catch(const exception&) {
        return false;
    }

    #elif __linux__
    string configHome;
    const char *xdgConfigHome = getenv("XDG_CONFIG_HOME");
    if(xdgConfigHome != nullptr && strlen(xdgConfigHome) > 0) {
        configHome = string(xdgConfigHome);
    }
    else {
        string homePath = __getHomePath();
        if(homePath.empty()) {
            return false;
        }
        configHome = homePath + "/.config";
    }

    filesystem::path autoStartDir = filesystem::path(configHome) / "autostart";
    filesystem::path desktopPath = autoStartDir / (name + ".desktop");

    try {
        if(!options.enabled) {
            return !filesystem::exists(desktopPath) || filesystem::remove(desktopPath);
        }

        filesystem::create_directories(autoStartDir);
        ofstream desktopFile(desktopPath);
        if(!desktopFile) {
            return false;
        }

        string command = __quoteDesktopArg(path);
        for(const string &arg: options.args) {
            command += " " + __quoteDesktopArg(arg);
        }

        desktopFile << "[Desktop Entry]\n"
                    << "Type=Application\n"
                    << "Version=1.0\n"
                    << "Name=" << name << "\n"
                    << "Exec=" << command << "\n"
                    << "Terminal=false\n"
                    << "X-GNOME-Autostart-enabled=true\n";
        return desktopFile.good();
    }
    catch(const exception&) {
        return false;
    }

    #else
    return false;
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

json setAutoStart(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"enabled"})) {
        output["error"] = errors::makeMissingArgErrorPayload("enabled");
        return output;
    }

    app::AutoStartOptions options;
    options.enabled = input["enabled"].get<bool>();

    if(helpers::hasField(input, "path")) {
        options.path = input["path"].get<string>();
    }
    if(helpers::hasField(input, "args")) {
        options.args = input["args"].get<vector<string>>();
    }
    if(helpers::hasField(input, "name")) {
        options.name = input["name"].get<string>();
    }

    if(app::setAutoStart(options)) {
        output["success"] = true;
    }
    else {
        output["error"] = errors::makeErrorPayload(errors::NE_APP_UNLSEAS);
    }
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
