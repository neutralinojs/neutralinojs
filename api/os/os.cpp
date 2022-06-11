#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cstdio>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <array>
#include <map>
#include <cstring>
#include <vector>

#include "resources.h"
#include "lib/tinyprocess/process.hpp"
#include "lib/platformfolders/platform_folders.h"
#include "lib/filedialogs/portable-file-dialogs.h"

#if defined(_WIN32)
#define _WINSOCKAPI_
#include <windows.h>
#include <gdiplus.h>
#include <shlwapi.h>

#pragma comment(lib, "Shell32.lib")
#pragma comment (lib,"Gdiplus.lib")
#endif

#include "lib/json/json.hpp"
#include "lib/tray/tray.h"
#include "helpers.h"
#include "settings.h"
#include "resources.h"
#include "api/events/events.h"
#include "api/filesystem/filesystem.h"
#include "api/debug/debug.h"
#include "api/os/os.h"
#include "api/window/window.h"

#define MAX_TRAY_MENU_ITEMS 50

using namespace std;
using json = nlohmann::json;
#if defined(_WIN32)
using namespace Gdiplus;
#endif

namespace os {

struct tray_menu menus[MAX_TRAY_MENU_ITEMS];
struct tray tray;
bool trayInitialized = false;
map<int, TinyProcessLib::Process*> spawnedProcesses;
mutex spawnedProcessesLock;

void __dispatchSpawnedProcessEvt(int virtualPid, const string &action, const json &data) {
    json evt;
    evt["id"] = virtualPid;
    evt["action"] = action;
    evt["data"] = data;
    events::dispatch("spawnedProcess", evt);
}

bool isTrayInitialized() {
    return trayInitialized;
}

void cleanupTray() {
    if(os::isTrayInitialized()) {
        tray_exit();
    }
}

void open(const string &url) {
    #if defined(__linux__) || defined(__FreeBSD__)
    os::execCommand("xdg-open \"" + url + "\"", "", true);
    #elif defined(__APPLE__)
    os::execCommand("open \"" + url + "\"", "", true);
    #elif defined(_WIN32)
    ShellExecute(0, 0, url.c_str(), 0, 0, SW_SHOW );
    #endif
}

os::CommandResult execCommand(string command, const string &input, bool background) {
    #if defined(_WIN32)
    command = "cmd.exe /c \"" + command + "\"";
    #endif

    os::CommandResult commandResult;
    TinyProcessLib::Process *childProcess;

    if(!background)
        childProcess = new TinyProcessLib::Process(
            command, "",
            [&](const char *bytes, size_t n) {
                commandResult.stdOut += string(bytes, n);
            },
            [&](const char *bytes, size_t n) {
                commandResult.stdErr += string(bytes, n);
            }, !input.empty()
        );
    else {
        childProcess = new TinyProcessLib::Process(command, "", nullptr, nullptr);
    }
    commandResult.pid = childProcess->get_id();

    if(!background) {
        if(!input.empty()) {
            childProcess->write(input);
            childProcess->close_stdin();
        }
        commandResult.exitCode = childProcess->get_exit_status(); // sync wait
    }
    delete childProcess;
    return commandResult;
}

pair<int, int> spawnProcess(string command) {
    #if defined(_WIN32)
    command = "cmd.exe /c \"" + command + "\"";
    #endif

    TinyProcessLib::Process *childProcess;
    lock_guard<mutex> guard(spawnedProcessesLock);
    int virtualPid = spawnedProcesses.size();

    childProcess = new TinyProcessLib::Process(
        command, "",
        [=](const char *bytes, size_t n) {
            __dispatchSpawnedProcessEvt(virtualPid, "stdOut", string(bytes, n));
        },
        [=](const char *bytes, size_t n) {
            __dispatchSpawnedProcessEvt(virtualPid, "stdErr", string(bytes, n));
        }, true
    );

    spawnedProcesses[virtualPid] = childProcess;

    thread processThread([=](){
        int exitCode = childProcess->get_exit_status(); // sync wait
        __dispatchSpawnedProcessEvt(virtualPid, "exit", exitCode);
        lock_guard<mutex> guard(spawnedProcessesLock);
        spawnedProcesses.erase(virtualPid);
        delete childProcess;
    });
    processThread.detach();

    return make_pair(virtualPid, childProcess->get_id());
}

bool updateSpawnedProcess(const os::SpawnedProcessEvent &evt) {
    if(spawnedProcesses.find(evt.id) == spawnedProcesses.end()) {
        return false;
    }

    TinyProcessLib::Process *childProcess = spawnedProcesses[evt.id];

    if(evt.type == "exit") {
        childProcess->kill();
    }
    else if(evt.type == "stdIn") {
        childProcess->write(evt.stdIn);
    }
    else if(evt.type == "stdInEnd") {
        childProcess->close_stdin();
    }
    else {
        return false;
    }

    return true;
}

string getPath(const string &name) {
    string path = "";
    if(name == "config")
        path = sago::getConfigHome();
    else if(name == "data")
        path = sago::getDataHome();
    else if(name == "cache")
        path = sago::getCacheDir();
    else if(name == "documents")
        path = sago::getDocumentsFolder();
    else if(name == "pictures")
        path = sago::getPicturesFolder();
    else if(name == "music")
        path = sago::getMusicFolder();
    else if(name == "video")
        path = sago::getVideoFolder();
    else if(name == "downloads")
        path = sago::getDownloadFolder();
    else if(name == "saveGames1")
        path = sago::getSaveGamesFolder1();
    else if(name == "saveGames2")
        path = sago::getSaveGamesFolder2();
    return helpers::normalizePath(path);
}

string getEnv(const string &key) {
    char *value;
    value = getenv(key.c_str());
    return value == nullptr ? "" : string(value);
}

namespace controllers {

vector<string> __extensionsToVector(const json &filters) {
    vector <string> filtersV = {};
    for (auto &filter: filters) {
        filtersV.push_back(filter["name"].get<string>());
        string extensions = "";
        for (auto &extension: filter["extensions"]) {
            extensions += "*." + extension.get<string>() + " ";
        }
        filtersV.push_back(extensions);
    }
    return filtersV;
}

json execCommand(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"command"})) {
        output["error"] = helpers::makeMissingArgErrorPayload();
        return output;
    }
    string command = input["command"].get<string>();
    bool background = false;
    string stdIn = "";
    if(helpers::hasField(input, "stdIn")) {
        stdIn = input["stdIn"].get<string>();
    }
    if(helpers::hasField(input, "background")) {
        background = input["background"].get<bool>();
    }
    os::CommandResult commandResult = os::execCommand(command, stdIn, background);

    json retVal;
    retVal["pid"] = commandResult.pid;
    retVal["exitCode"] = commandResult.exitCode;
    retVal["stdOut"] = commandResult.stdOut;
    retVal["stdErr"] = commandResult.stdErr;

    output["returnValue"] = retVal;
    output["success"] = true;
    return output;
}

json spawnProcess(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"command"})) {
        output["error"] = helpers::makeMissingArgErrorPayload();
        return output;
    }
    string command = input["command"].get<string>();

    auto spawnedData = os::spawnProcess(command);

    json process;
    process["id"] = spawnedData.first;
    process["pid"] = spawnedData.second;
    output["returnValue"] = process;
    output["success"] = true;
    return output;
}

json updateSpawnedProcess(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"id", "event"})) {
        output["error"] = helpers::makeMissingArgErrorPayload();
        return output;
    }

    os::SpawnedProcessEvent processEvt;
    processEvt.id = input["id"].get<int>();
    processEvt.type = input["event"].get<string>();

    if(helpers::hasField(input, "data")) {
        if(processEvt.type == "stdIn") {
            processEvt.stdIn = input["data"].get<string>();
        }
    }

    if(os::updateSpawnedProcess(processEvt)) {
        output["success"] = true;
    }
    else {
        output["error"] = helpers::makeErrorPayload("NE_OS_UNLTOUP",
                                "Unable to update process id: " + to_string(processEvt.id));
    }
    return output;
}

json getSpawnedProcesses(const json &input) {
    json output;
    json processes = json::array();
    lock_guard<mutex> guard(spawnedProcessesLock);
    for(const auto &[id, childProcess]: spawnedProcesses) {
        json process;
        process["id"] = id;
        process["pid"] = childProcess->get_id();
        processes.push_back(process);
    }
    output["returnValue"] = processes;
    output["success"] = true;
    return output;
}

json getEnv(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"key"})) {
        output["error"] = helpers::makeMissingArgErrorPayload();
        return output;
    }
    string key = input["key"].get<string>();

    output["returnValue"] = os::getEnv(key);
    output["success"] = true;
    return output;
}

json showOpenDialog(const json &input) {
    json output;
    string title = "Open a file";
    string defaultPath = "";
    vector <string> filters = {"All files", "*"};
    pfd::opt option = pfd::opt::none;

    if(helpers::hasField(input, "title")) {
        title = input["title"].get<string>();
    }

    if(helpers::hasField(input, "multiSelections") && input["multiSelections"].get<bool>()) {
        option = pfd::opt::multiselect;
    }

    if(helpers::hasField(input, "filters")) {
        filters.clear();
        filters = __extensionsToVector(input["filters"]);
    }
    
    if(helpers::hasField(input, "defaultPath")) {
        defaultPath = input["defaultPath"].get<string>();
    }

    vector<string> selectedEntries = pfd::open_file(title, defaultPath, filters, option).result();

    for(string &entry: selectedEntries) {
        entry = helpers::normalizePath(entry);
    }

    output["returnValue"] = selectedEntries;
    output["success"] = true;
    return output;
}

json showFolderDialog(const json &input) {
    json output;
    string title = "Select a folder";
    string defaultPath = "";

    if(helpers::hasField(input, "title")) {
        title = input["title"].get<string>();
    }
    
    if(helpers::hasField(input, "defaultPath")) {
        defaultPath = input["defaultPath"].get<string>();
    }

    string selectedEntry = pfd::select_folder(title, defaultPath, pfd::opt::none).result();

    output["returnValue"] = helpers::normalizePath(selectedEntry);
    output["success"] = true;
    return output;
}

json showSaveDialog(const json &input) {
    json output;
    string title = "Save a file";
    string defaultPath = "";
    vector <string> filters = {"All files", "*"};
    pfd::opt option = pfd::opt::none;

    if(helpers::hasField(input, "title")) {
        title = input["title"].get<string>();
    }

    if(helpers::hasField(input, "forceOverwrite") && input["forceOverwrite"].get<bool>()) {
        option = pfd::opt::force_overwrite;
    }

    if(helpers::hasField(input, "filters")) {
        filters.clear();
        filters = __extensionsToVector(input["filters"]);
    }

    if(helpers::hasField(input, "defaultPath")) {
        defaultPath = input["defaultPath"].get<string>();
    }

    string selectedEntry = pfd::save_file(title, defaultPath, filters, option).result();

    output["returnValue"] = helpers::normalizePath(selectedEntry);
    output["success"] = true;
    return output;
}

json showNotification(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"title", "content"})) {
        output["error"] = helpers::makeMissingArgErrorPayload();
        return output;
    }
    string title = input["title"].get<string>();
    string content = input["content"].get<string>();
    string icon = "INFO";

    if(helpers::hasField(input, "icon")) {
        icon = input["icon"].get<string>();
    }

    map<string, pfd::icon> iconMap = {
        {"INFO", pfd::icon::info},
        {"WARNING", pfd::icon::warning},
        {"ERROR", pfd::icon::error},
        {"QUESTION", pfd::icon::question}
    };

    if(iconMap.find(icon) != iconMap.end()) {
        pfd::notify(title, content, iconMap[icon]);
    }
    else {
        output["error"] = helpers::makeErrorPayload("NE_OS_INVNOTA",
                                "Invalid notification style arguments: " + icon);
    }
    output["success"] = true;
    return output;
}

json showMessageBox(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"title", "content"})) {
        output["error"] = helpers::makeMissingArgErrorPayload();
        return output;
    }
    string icon = "INFO";
    string choice = "OK";
    string title = input["title"].get<string>();
    string content = input["content"].get<string>();

    if(helpers::hasField(input, "icon")) {
        icon = input["icon"].get<string>();
    }

    if(helpers::hasField(input, "choice")) {
        choice = input["choice"].get<string>();
    }

    map<string, pfd::choice> choiceMap = {
        {"OK", pfd::choice::ok},
        {"OK_CANCEL", pfd::choice::ok_cancel},
        {"YES_NO", pfd::choice::yes_no},
        {"YES_NO_CANCEL", pfd::choice::yes_no_cancel},
        {"RETRY_CANCEL", pfd::choice::retry_cancel},
        {"ABORT_RETRY_IGNORE", pfd::choice::abort_retry_ignore}
    };

    map<string, pfd::icon> iconMap = {
        {"INFO", pfd::icon::info},
        {"WARNING", pfd::icon::warning},
        {"ERROR", pfd::icon::error},
        {"QUESTION", pfd::icon::question}
    };

    if(choiceMap.find(choice) != choiceMap.end() &&  iconMap.find(icon) != iconMap.end()) {
        pfd::button button = pfd::message(title, content, choiceMap[choice], iconMap[icon]).result();
        string selectedBtn = "IGNORE";
        switch(button) {
            case pfd::button::cancel: selectedBtn = "CANCEL"; break;
            case pfd::button::ok: selectedBtn = "OK"; break;
            case pfd::button::yes: selectedBtn = "YES"; break;
            case pfd::button::no: selectedBtn = "NO"; break;
            case pfd::button::abort: selectedBtn = "ABORT"; break;
            case pfd::button::retry: selectedBtn = "RETRY"; break;
            case pfd::button::ignore: selectedBtn = "IGNORE"; break;
        }
        output["returnValue"] = selectedBtn;
        output["success"] = true;
    }
    else {
        output["error"] = helpers::makeErrorPayload("NE_OS_INVMSGA",
                                "Invalid message box style arguments: " + choice + " or/and " + icon);
    }
    return output;
}

void __handleTrayMenuItem(struct tray_menu *item) {
    (void)item;
    if(item->id == nullptr)
        return;
    json eventData;
    eventData["id"] = string(item->id);
    eventData["text"] = string(item->text);
    eventData["isChecked"] = item->checked == 1;
    eventData["isDisabled"] = item->disabled == 1;
    events::dispatch("trayMenuItemClicked", eventData);
}

json setTray(const json &input) {
    #if defined(_WIN32)
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
    #endif
    json output;
    int menuCount = 1;

    if(helpers::hasField(input, "menuItems")) {
        menuCount += input["menuItems"].size();
    }

    menus[menuCount - 1] = { nullptr, nullptr, 0, 0, nullptr, nullptr };

    int i = 0;
    for (const auto &menuItem: input["menuItems"]) {
        char *id = nullptr;
        char *text = helpers::cStrCopy(menuItem["text"].get<string>());
        int disabled = 0;
        int checked = 0;
        if(helpers::hasField(menuItem, "id")) {
            id = helpers::cStrCopy(menuItem["id"].get<string>());
        }
        if(helpers::hasField(menuItem, "isDisabled")) {
            disabled = menuItem["isDisabled"].get<bool>() ? 1 : 0;
        }
        if(helpers::hasField(menuItem, "isChecked")) {
            checked = menuItem["isChecked"].get<bool>() ? 1 : 0;
        }

        delete[] menus[i].id;
        delete[] menus[i].text;
        menus[i] = { id, text, disabled, checked, __handleTrayMenuItem, nullptr };
        i++;
    }

    tray.menu = menus;

    if(helpers::hasField(input, "icon")) {
        string iconPath = input["icon"].get<string>();
        #if defined(__linux__)
        string fullIconPath;
        if(resources::getMode() == "directory") {
            fullIconPath = fs::getFullPathFromRelative(settings::joinAppPath("")) + iconPath;
        }
        else {
            string tempDirPath = settings::joinAppPath("/.tmp");
            fs::createDirectory(tempDirPath);
            string tempIconPath = settings::joinAppPath("/.tmp/tray_icon_linux.png");
            resources::extractFile(iconPath, tempIconPath);
            fullIconPath = fs::getFullPathFromRelative(tempIconPath);
        }
        delete[] tray.icon;
        tray.icon = helpers::cStrCopy(fullIconPath);

        #elif defined(_WIN32)
        fs::FileReaderResult fileReaderResult = resources::getFile(iconPath);
        string iconDataStr = fileReaderResult.data;
        const char *iconData = iconDataStr.c_str();
        unsigned char *uiconData = reinterpret_cast<unsigned char*>(const_cast<char*>(iconData));
        IStream *pStream = SHCreateMemStream((BYTE *) uiconData, iconDataStr.length());
        Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromStream(pStream);
        bitmap->GetHICON(&tray.icon);
        pStream->Release();

        #elif defined(__APPLE__)
        fs::FileReaderResult fileReaderResult = resources::getFile(iconPath);
        string iconDataStr = fileReaderResult.data;
        const char *iconData = iconDataStr.c_str();
        tray.icon =
            ((id (*)(id, SEL))objc_msgSend)("NSImage"_cls, "alloc"_sel);

        id nsIconData = ((id (*)(id, SEL, const char*, int))objc_msgSend)("NSData"_cls,
                    "dataWithBytes:length:"_sel, iconData, iconDataStr.length());

        ((void (*)(id, SEL, id))objc_msgSend)(tray.icon, "initWithData:"_sel, nsIconData);
        #endif
    }

    if(!trayInitialized) {
        trayInitialized = tray_init(&tray) == 0;
    }
    else {
        tray_update(&tray);
    }
    #if defined(_WIN32)
    GdiplusShutdown(gdiplusToken);
    #endif
    if(trayInitialized) {
        output["success"] = true;
    }
    else {
        output["error"] = helpers::makeErrorPayload("NE_OS_TRAYIER",
                    "Unable to initialize the tray menu");
    }
    return output;
}

json open(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"url"})) {
        output["error"] = helpers::makeMissingArgErrorPayload();
        return output;
    }
    string url = input["url"].get<string>();
    os::open(url);
    output["success"] = true;
    return output;
}

json getPath(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"name"})) {
        output["error"] = helpers::makeMissingArgErrorPayload();
        return output;
    }
    string name = input["name"].get<string>();
    string path = os::getPath(name);
    if(!path.empty()) {
        output["returnValue"] = path;
        output["success"] = true;
    }
    else {
        output["error"] = helpers::makeErrorPayload("NE_OS_INVKNPT",
                                "Invalid platform path name: " + name);
    }
    return output;
}
} // namespace controllers
} // namespace os
