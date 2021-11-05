#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <map>
#include <cstring>
#include <vector>

#include "lib/tinyprocess/process.hpp"
#include "lib/platformfolders/platform_folders.h"
#include "lib/filedialogs/portable-file-dialogs.h"

#if defined(__linux__) || defined(__FreeBSD__)
#define TRAY_APPINDICATOR 1

#elif defined(__APPLE__)
#define TRAY_APPKIT 1

#elif defined(_WIN32)
#define TRAY_WINAPI 1
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

struct tray_menu menus[MAX_TRAY_MENU_ITEMS];
struct tray tray;
bool isTrayCreated = false;

namespace os {
    void open(const string &url) {
        #if defined(__linux__) || defined(__FreeBSD__)
        os::execCommand("xdg-open \"" + url + "\"");
        #elif defined(__APPLE__)
        os::execCommand("open \"" + url + "\"");
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

    string getPath(const string &name) {
        if(name == "config")
            return sago::getConfigHome();
        if(name == "data")
            return sago::getDataHome();
        if(name == "cache")
            return sago::getCacheDir();
        if(name == "documents")
            return sago::getDocumentsFolder();
        if(name == "pictures")
            return sago::getPicturesFolder();
        if(name == "music")
            return sago::getMusicFolder();
        if(name == "video")
            return sago::getVideoFolder();
        if(name == "downloads")
            return sago::getDownloadFolder();
        if(name == "saveGames1")
            return sago::getSaveGamesFolder1();
        if(name == "saveGames2")
            return sago::getSaveGamesFolder2();
        return string();
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
        if(!input.contains("command")) {
            output["error"] = helpers::makeMissingArgErrorPayload();
            return output;
        }
        string command = input["command"].get<string>();
        bool background = false;
        string stdIn = "";
        if(input.contains("stdIn")) {
            stdIn = input["stdIn"].get<string>();
        }
        if(input.contains("background")) {
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

    json getEnv(const json &input) {
        json output;
        if(!input.contains("key")) {
            output["error"] = helpers::makeMissingArgErrorPayload();
            return output;
        }
        string varKey = input["key"].get<string>();
        char *varValue;
        varValue = getenv(varKey.c_str());
        if(varValue == nullptr) {
            output["error"] = helpers::makeErrorPayload("NE_OS_ENVNOEX",
                                        varKey + " is not defined");
        }
        else {
            output["returnValue"] = varValue;
            output["success"] = true;
        }
        return output;
    }

    json showOpenDialog(const json &input) {
        json output;
        string title = "Open a file";
        vector <string> filters = {"All files", "*"};
        pfd::opt option = pfd::opt::none;

        if(input.contains("title")) {
            title = input["title"].get<string>();
        }

        if(input.contains("multiSelections") && input["multiSelections"].get<bool>()) {
            option = pfd::opt::multiselect;
        }

        if(input.contains("filters")) {
            filters.clear();
            filters = __extensionsToVector(input["filters"]);
        }

        vector<string> selectedEntries = pfd::open_file(title, "", filters, option).result();

        output["returnValue"] = selectedEntries;
        output["success"] = true;
        return output;
    }

    json showFolderDialog(const json &input) {
        json output;
        string title = "Select a folder";

        if(input.contains("title")) {
            title = input["title"].get<string>();
        }

        string selectedEntry = pfd::select_folder(title, "", pfd::opt::none).result();

        output["returnValue"] = selectedEntry;
        output["success"] = true;
        return output;
    }

    json showSaveDialog(const json &input) {
        json output;
        string title = "Save a file";
        vector <string> filters = {"All files", "*"};
        pfd::opt option = pfd::opt::none;

        if(input.contains("title")) {
            title = input["title"].get<string>();
        }

        if(input.contains("forceOverwrite") && input["forceOverwrite"].get<bool>()) {
            option = pfd::opt::force_overwrite;
        }

        if(input.contains("filters")) {
            filters.clear();
            filters = __extensionsToVector(input["filters"]);
        }

        string selectedEntry = pfd::save_file(title, "", filters, option).result();

        output["returnValue"] = selectedEntry;
        output["success"] = true;
        return output;
    }

    json showNotification(const json &input) {
        json output;
        if(!input.contains("title") || !input.contains("content")) {
            output["error"] = helpers::makeMissingArgErrorPayload();
            return output;
        }
        string title = input["title"].get<string>();
        string content = input["content"].get<string>();
        string icon = "INFO";

        if(input.contains("icon")) {
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
        if(!input.contains("title") || !input.contains("content")) {
            output["error"] = helpers::makeMissingArgErrorPayload();
            return output;
        }
        string icon = "INFO";
        string choice = "OK";
        string title = input["title"].get<string>();
        string content = input["content"].get<string>();

        if(input.contains("icon")) {
            icon = input["icon"].get<string>();
        }

        if(input.contains("choice")) {
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

        if(input.contains("menuItems")) {
            menuCount += input["menuItems"].size();
        }

        menus[menuCount - 1] = { nullptr, nullptr, 0, 0, nullptr, nullptr };

        int i = 0;
        for (const auto &menuItem: input["menuItems"]) {
            char *id = nullptr;
            char *text = helpers::cStrCopy(menuItem["text"].get<string>());
            int disabled = 0;
            int checked = 0;
            if(menuItem.contains("id")) {
                id = helpers::cStrCopy(menuItem["id"].get<string>());
            }
            if(menuItem.contains("isDisabled")) {
                disabled = menuItem["isDisabled"].get<bool>() ? 1 : 0;
            }
            if(menuItem.contains("isChecked")) {
                checked = menuItem["isChecked"].get<bool>() ? 1 : 0;
            }

            delete[] menus[i].id;
            delete[] menus[i].text;
            menus[i] = { id, text, disabled, checked, __handleTrayMenuItem, nullptr };
            i++;
        }

        tray.menu = menus;

        if(input.contains("icon")) {
            string iconPath = input["icon"].get<string>();
            #if defined(__linux__)
            string fullIconPath;
            if(loadResFromDir) {
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
            fs::FileReaderResult fileReaderResult = settings::getFileContent(iconPath);
            string iconDataStr = fileReaderResult.data;
            const char *iconData = iconDataStr.c_str();
            unsigned char *uiconData = reinterpret_cast<unsigned char*>(const_cast<char*>(iconData));
            IStream *pStream = SHCreateMemStream((BYTE *) uiconData, iconDataStr.length());
            Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromStream(pStream);
            bitmap->GetHICON(&tray.icon);
            pStream->Release();

            #elif defined(__APPLE__)
            fs::FileReaderResult fileReaderResult = settings::getFileContent(iconPath);
            string iconDataStr = fileReaderResult.data;
            const char *iconData = iconDataStr.c_str();
            tray.icon =
                ((id (*)(id, SEL))objc_msgSend)("NSImage"_cls, "alloc"_sel);

            id nsIconData = ((id (*)(id, SEL, const char*, int))objc_msgSend)("NSData"_cls,
                        "dataWithBytes:length:"_sel, iconData, iconDataStr.length());

            ((void (*)(id, SEL, id))objc_msgSend)(tray.icon, "initWithData:"_sel, nsIconData);
            #endif
        }

        if(!isTrayCreated) {
            tray_init(&tray);
            isTrayCreated = true;
        }
        else {
            tray_update(&tray);
        }
        #if defined(_WIN32)
        GdiplusShutdown(gdiplusToken);
        #endif
        output["success"] = true;
        return output;
    }

    json open(const json &input) {
        json output;
        if(!input.contains("url")) {
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
        if(!input.contains("name")) {
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
