#include <iostream>
#include <fstream>
#include "lib/json.hpp"
#include <stdlib.h>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <map>
#include <cstring>

#if defined(__linux__) || defined(__FreeBSD__)
#define TRAY_APPINDICATOR 1

#elif defined(__APPLE__)
#include <lib/boxer/boxer.h>
#define TRAY_APPKIT 1

#elif defined(_WIN32)
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <lib/boxer/boxer.h>
#include <gdiplus.h>
#include <shlwapi.h>
#define TRAY_WINAPI 1

#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment (lib,"Gdiplus.lib")
#endif

#include "platform/platform.h"
#include "lib/tray/tray.h"
#include "../../helpers.h"
#include "api/window/window.h"
#include "api/filesystem/filesystem.h"
#include "settings.h"
#include "resources.h"

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
namespace controllers {
    json execCommand(json input) {
        json output;
        string command = input["command"];
        output["output"] = platform::execCommand(command);
        output["success"] = true;
        return output;
    }

    json getEnvar(json input) {
        json output;
        string varKey = input["key"];
        char *varValue;
        varValue = getenv(varKey.c_str());
        if(varValue == nullptr) {
            output["error"] =  varKey + " is not defined";
        }
        else {
            output["value"] = varValue;
            output["success"] = true;
        }
        return output;
    }

    json dialogOpen(json input) {
        json output;
        #if defined(__linux__) || defined(__FreeBSD__)
        string command = "zenity --file-selection";
        if(!input["title"].is_null())
            command += " --title \"" + input["title"].get<std::string>() + "\"";
        if(!input["isDirectoryMode"].is_null() && input["isDirectoryMode"].get<bool>())
            command += " --directory";
        output["selectedEntry"] = platform::execCommand(command);
        
        #elif defined(__APPLE__)
        string command = "osascript -e 'POSIX path of (choose ";
        if(!input["isDirectoryMode"].is_null() && input["isDirectoryMode"].get<bool>())
            command += "folder";
        else
            command += "file";
        if(!input["title"].is_null())
            command += " with prompt \"" + input["title"].get<std::string>() + "\"";
        command += ")'";
        output["selectedEntry"] = platform::execCommand(command);
        
        #elif defined(_WIN32)
        string title = input["title"];
        if(!input["isDirectoryMode"].is_null() && input["isDirectoryMode"].get<bool>()) {
            TCHAR szDir[MAX_PATH];
            BROWSEINFO bInfo;
            ZeroMemory(&bInfo, sizeof(bInfo));
            bInfo.hwndOwner = GetForegroundWindow();
            bInfo.pidlRoot = nullptr;
            bInfo.pszDisplayName = szDir;
            bInfo.lpszTitle = const_cast<char *>(title.c_str());
            bInfo.ulFlags = 0 ;
            bInfo.lpfn = nullptr;
            bInfo.lParam = 0;
            bInfo.iImage = -1;

            LPITEMIDLIST lpItem = SHBrowseForFolder( &bInfo);
            if( lpItem != nullptr ) {
                SHGetPathFromIDList(lpItem, szDir );
                output["selectedEntry"] = szDir;
            }
            else {
                output["selectedEntry"] = nullptr;
            }
        }
        else {
            OPENFILENAME ofn;
            TCHAR szFile[MAX_PATH] = { 0 };
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.hwndOwner = GetForegroundWindow();
            ofn.lpstrTitle = const_cast<char *>(title.c_str());
            ofn.lStructSize = sizeof(ofn);
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = nullptr;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = nullptr;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetOpenFileName(&ofn)) {
                output["selectedEntry"] = ofn.lpstrFile;
            }
            else {
                output["selectedEntry"] = nullptr;
            }
        }
        #endif
        
        output["success"] = true;
        return output;
    }

    json dialogSave(json input) {
        json output;
        #if defined(__linux__) || defined(__FreeBSD__)
        string command = "zenity --file-selection --save";
        if(!input["title"].is_null())
            command += " --title \"" + input["title"].get<std::string>() + "\"";
        output["selectedEntry"] = platform::execCommand(command);
        
        #elif defined(__APPLE__)
        string command = "osascript -e 'POSIX path of (choose file name";
        if(!input["title"].is_null())
            command += " with prompt \"" + input["title"].get<std::string>() + "\"";
        command += ")'";
        output["selectedEntry"] = platform::execCommand(command);
        
        #elif defined(_WIN32)
        string title = input["title"];
        OPENFILENAME ofn;
        TCHAR szFile[260] = { 0 };

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.hwndOwner = GetForegroundWindow();
        ofn.lpstrTitle = const_cast<char *>(title.c_str());
        ofn.lStructSize = sizeof(ofn);
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = nullptr;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = nullptr;
        ofn.Flags = OFN_PATHMUSTEXIST;

        if (GetSaveFileName(&ofn)) {
            output["selectedEntry"] = ofn.lpstrFile;
        }
        else {
            output["selectedEntry"] = nullptr;
        }
        #endif
        output["success"] = true;
        return output;
    }

    json showNotification(json input) {
        json output;
        #if defined(__linux__) || defined(__FreeBSD__)
        string command = "notify-send \"" + input["summary"].get<string>() + "\" \"" +
                            input["body"].get<string>() + "\"";
        if(system(command.c_str()) == 0) {
            output["success"] = true;
            output["message"] = "Notification was sent to the system";
        }
        else
            output["error"] = "An error thrown while sending the notification";
        
        #elif defined(__APPLE__)
        string command = "osascript -e 'display notification \"" + 
        input["body"].get<std::string>() + "\"";
        if(!input["summary"].is_null())
            command += " with title \"" + input["summary"].get<std::string>() + "\"";
        command += "'";
        platform::execCommand(command);
        output["success"] = true;
        
        #elif defined(_WIN32)
        string command = "powershell -Command \"& {Add-Type -AssemblyName System.Windows.Forms;"
                        "Add-Type -AssemblyName System.Drawing;"
                        "$notify = New-Object System.Windows.Forms.NotifyIcon;"
                        "$notify.Icon = [System.Drawing.SystemIcons]::Information;"
                        "$notify.Visible = $true;"
                        "$notify.ShowBalloonTip(0 ,'"+ input["summary"].get<string>() + "','" + input["body"].get<string>() + "',[System.Windows.Forms.TooltipIcon]::None)}\"";

        string commandOutput = platform::execCommand(command);

        if(commandOutput.find("'powershell'") == string::npos) {
            output["success"] = true;
            output["message"] = "Notification was sent to the system";
        }
        else
            output["error"] = "An error thrown while sending the notification";
        #endif
        return output;
    }

    json showMessageBox(json input) {
        #if defined(__linux__) || defined(__FreeBSD__)
        json output;
        map <string, string> messageTypes = {{"INFO", "info"}, {"WARN", "warning"},
                                            {"ERROR", "error"}, {"QUESTION", "question"}};
        string messageType;
        messageType = input["type"].get<string>();
        if(messageTypes.find(messageType) == messageTypes.end()) {
            output["error"] = "Invalid message type: '" + messageType + "' provided";
            return output.dump();
        }
        string command = "zenity --no-wrap --" + messageTypes[messageType] + " --title=\"" +
                            input["title"].get<string>() + "\" --text=\"" +
                            input["content"].get<string>() + "\" && echo $?";
        string response = platform::execCommand(command);
        if(messageType == "QUESTION")
            output["yesButtonClicked"] =  response.find("0") != std::string::npos;
        output["success"] = true;
        
        #elif defined(__APPLE__) || defined(_WIN32)
            string title = input["title"];
            string content = input["content"];
            string type = input["type"];

            json output;
            boxer::Selection msgSel;

            if(type == "INFO")
                msgSel = boxer::show(content.c_str(), title.c_str(), boxer::Style::Info);
            else if(type == "WARN")
                msgSel = boxer::show(content.c_str(), title.c_str(), boxer::Style::Warning);
            else if(type == "ERROR")
                msgSel = boxer::show(content.c_str(), title.c_str(), boxer::Style::Error);
            else if(type == "QUESTION") {
                msgSel = boxer::show(content.c_str(), title.c_str(), boxer::Style::Question,
                                    boxer::Buttons::YesNo);
                output["yesButtonClicked"] =  msgSel == boxer::Selection::Yes;
            }
            else 
                output["error"] = "Invalid message type: '" + type + "' provided";

            if(output["error"].is_null())
                output["success"] = true;
        
        #endif
        return output;
    }
    
    void __handleTrayMenuItem(struct tray_menu *item) {
        (void)item;
        if(item->id == nullptr)
            return;
        string js = "if(window.Neutralino.events && window.Neutralino.events.onTrayMenuItemClicked) ";
        js += "window.Neutralino.events.onTrayMenuItemClicked({";
        js += "id: '" + std::string(item->id) + "',";
        js += "text: '" + std::string(item->text) + "',";
        js += "isChecked: " + std::string(item->checked ? "true" : "false") + ",";
        js += "isDisabled: " + std::string(item->disabled ? "true" : "false");
        js += "});";
    	window::executeJavaScript(js);
    }
    
    json setTray(json input) {
        #if defined(_WIN32)
        GdiplusStartupInput gdiplusStartupInput;
        ULONG_PTR gdiplusToken;
        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
        #endif
        json output;
        int menuCount = 1;
            
        if(!input["menuItems"].is_null()) {
            menuCount += input["menuItems"].size();
        }
        
        menus[menuCount - 1] = { nullptr, nullptr, 0, 0, nullptr, nullptr };
        
        int i = 0;
        for (auto &menuItem: input["menuItems"]) {
            char *id = nullptr;
            char *text = helpers::cStrCopy(menuItem["text"].get<std::string>());
            int disabled = 0;
            int checked = 0;
            if(!menuItem["id"].is_null())
                id = helpers::cStrCopy(menuItem["id"].get<std::string>());
            if(!menuItem["isDisabled"].is_null() && menuItem["isDisabled"].get<bool>())
                disabled = 1;
            if(!menuItem["isChecked"].is_null() && menuItem["isChecked"].get<bool>())
                checked = 1;
                
            menus[i++] = { id, text, disabled, checked, __handleTrayMenuItem, nullptr };
        }

        tray.menu = menus;

        if(!input["icon"].is_null()) {
            string iconPath = input["icon"];
            #if defined(__linux__)
            string fullIconPath;
            if(loadResFromDir) {
                fullIconPath = platform::getFullPathFromRelative(settings::joinAppPath("")) + iconPath;
            }
            else {
                json createDirParams;
                createDirParams["path"] = settings::joinAppPath("/.tmp");
                fs::createDirectory(createDirParams);
                string tempIconPath = settings::joinAppPath("/.tmp/tray_icon_linux.png");
                resources::extractFile(iconPath, tempIconPath);
                fullIconPath = platform::getFullPathFromRelative(tempIconPath);
            }
            tray.icon = helpers::cStrCopy(fullIconPath);

            #elif defined(_WIN32)
            string iconDataStr = settings::getFileContent(iconPath);
            const char *iconData = iconDataStr.c_str();
            unsigned char *uiconData = reinterpret_cast<unsigned char*>(const_cast<char*>(iconData));
            IStream *pStream = SHCreateMemStream((BYTE *) uiconData, iconDataStr.length());
            Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromStream(pStream);
            bitmap->GetHICON(&tray.icon);
            pStream->Release();

            #elif defined(__APPLE__)
            string iconDataStr = settings::getFileContent(iconPath);
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
} // namespace controllers
} // namespace os
