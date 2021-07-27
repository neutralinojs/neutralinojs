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
#define EXEC_BUFSIZE 4096

#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment (lib,"Gdiplus.lib")
#endif

#include "lib/tray/tray.h"
#include "../../helpers.h"
#include "api/window/window.h"
#include "api/filesystem/filesystem.h"
#include "api/debug/debug.h"
#include "api/os/os.h"
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

    string execCommand(string command, bool shouldCombineErrorStream) {
        if(shouldCombineErrorStream)
            command += " 2>&1";
        #if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
        std::array<char, 128> buffer;
        std::string result = "";
        std::shared_ptr<FILE> pipe(popen((command + " 2>&1").c_str(), "r"), pclose);
        if (!pipe) {
            debug::log("ERROR", "Pipe open failed.");
        }
        else {
            while (!feof(pipe.get())) {
                if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
                    result += buffer.data();
            }
        }
        // Erase ending new line charactor
        if (!result.empty() && result[result.length() - 1] == '\n')
            result.erase(result.length() - 1);
        return result;

        #elif defined(_WIN32)
        // A modified version of https://stackoverflow.com/a/59523254
        string output = "";
        HANDLE g_hChildStd_OUT_Rd = NULL;
        HANDLE g_hChildStd_OUT_Wr = NULL;

        SECURITY_ATTRIBUTES sa;
        // Set the bInheritHandle flag so pipe handles are inherited.
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;
        if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &sa, 0))     { return output; } // Create a pipe for the child process's STDOUT.
        if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) { return output; } // Ensure the read handle to the pipe for STDOUT is not inherited

        PROCESS_INFORMATION piProcInfo;
        STARTUPINFO siStartInfo;
        bool bSuccess = FALSE;

        // Set up members of the PROCESS_INFORMATION structure.
        ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

        // Set up members of the STARTUPINFO structure.
        // This structure specifies the STDERR and STDOUT handles for redirection.
        ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
        siStartInfo.cb = sizeof(STARTUPINFO);
        siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
        siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

        // Create the child process.
        bSuccess = CreateProcess(
            NULL,             // program name
            (LPSTR)("cmd /c " + command).c_str(),       // command line
            NULL,             // process security attributes
            NULL,             // primary thread security attributes
            TRUE,             // handles are inherited
            CREATE_NO_WINDOW, // creation flags (this is what hides the window)
            NULL,             // use parent's environment
            NULL,             // use parent's current directory
            &siStartInfo,     // STARTUPINFO pointer
            &piProcInfo       // receives PROCESS_INFORMATION
        );
        CloseHandle(g_hChildStd_OUT_Wr);

        // read output
        DWORD dwRead;
        CHAR chBuf[EXEC_BUFSIZE];
        bool bSuccess2 = FALSE;
        for (;;) { // read stdout
            bSuccess2 = ReadFile(g_hChildStd_OUT_Rd, chBuf, EXEC_BUFSIZE, &dwRead, NULL);
            if(!bSuccess2 || dwRead == 0) break;
            std::string s(chBuf, dwRead);
            output += s;
        }

        // The remaining open handles are cleaned up when this process terminates.
        // To avoid resource leaks in a larger application,
        // close handles explicitly.
        return output;
        #endif
    }
    
    os::MessageBoxResult showMessageBox(os::MessageBoxOptions msgboxOptions) {
        MessageBoxResult result;
        #if defined(__linux__) || defined(__FreeBSD__)
        map <string, string> messageTypes = {{"INFO", "info"}, {"WARN", "warning"},
                                            {"ERROR", "error"}, {"QUESTION", "question"}};
        string messageType;
        messageType = msgboxOptions.type;
        if(messageTypes.find(messageType) == messageTypes.end()) {
            result.hasError = true;
            result.error = "Invalid message type: '" + messageType + "' provided";
            return result;
        }
        string command = "zenity --no-wrap --" + messageTypes[messageType] + " --title=\"" +
                            msgboxOptions.title + "\" --text=\"" +
                            msgboxOptions.content + "\" && echo $?";
        string response = os::execCommand(command);
        if(messageType == "QUESTION")
            result.yesButtonClicked =  response.find("0") != std::string::npos;
        
        #elif defined(__APPLE__) || defined(_WIN32)
            string title = msgboxOptions.title;
            string content = msgboxOptions.content;
            string type = msgboxOptions.type;

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
                result.yesButtonClicked =  msgSel == boxer::Selection::Yes;
            }
            else {
                result.hasError = true;
                result.error = "Invalid message type: '" + type + "' provided";
            }
        
        #endif
        return result;
    }
    
namespace controllers {
    json execCommand(json input) {
        json output;
        string command = input["command"];
        output["output"] = os::execCommand(command, true);
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
        bool isDirectoryMode = false;
        if(!input["isDirectoryMode"].is_null() && input["isDirectoryMode"].get<bool>())
            isDirectoryMode = true;
        
        #if defined(__linux__) || defined(__FreeBSD__)
        string command = "zenity --file-selection";

        if(!input["title"].is_null())
            command += " --title \"" + input["title"].get<std::string>() + "\"";
        if(isDirectoryMode)
            command += " --directory";

        if(!isDirectoryMode && !input["filter"].is_null()) {
            vector<string> filters = input["filter"];
            for(int i = 0; i < filters.size(); i++) {
                command += " --file-filter=\"*." + filters[i] + "\"";
            }
        }
	    
        string commandOutput = os::execCommand(command);
        if(!commandOutput.empty())
            output["selectedEntry"] = commandOutput;
        else
            output["selectedEntry"] = nullptr;
        
        #elif defined(__APPLE__)
        string command = "osascript -e 'POSIX path of (choose ";

        if(isDirectoryMode)
            command += "folder";
        else
            command += "file";
        if(!input["title"].is_null())
            command += " with prompt \"" + input["title"].get<std::string>() + "\"";

        if(!isDirectoryMode && !input["filter"].is_null()) {
            string filterCommand = "of type {\"\"";
            vector<string> filters = input["filter"];
            for(int i = 0; i < filters.size(); i++) {
                filterCommand += ", \"" + filters[i] + "\"";
            }
            filterCommand += "}";
            command += " " + filterCommand;
	    }
        command += ")'";

        string commandOutput = os::execCommand(command);
        if(!commandOutput.empty())
            output["selectedEntry"] = commandOutput;
        else
            output["selectedEntry"] = nullptr;
        
        #elif defined(_WIN32)
        string title = input["title"];
        if(isDirectoryMode) {
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

            LPITEMIDLIST lpItem = SHBrowseForFolder(&bInfo);
            if( lpItem != nullptr ) {
                SHGetPathFromIDList(lpItem, szDir );
                output["selectedEntry"] = szDir;
            }
            else {
                output["selectedEntry"] = nullptr;
            }
        }
        else {
            string filterStr = "";
            OPENFILENAME ofn;
            TCHAR szFile[MAX_PATH] = { 0 };
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.hwndOwner = GetForegroundWindow();
            ofn.lpstrTitle = const_cast<char *>(title.c_str());
            ofn.lStructSize = sizeof(ofn);
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.nFilterIndex = 1;
            if(!input["filter"].is_null()) {
                vector<string> filters = input["filter"];
                for(int i = 0; i < filters.size(); i++) {
                   filterStr.append("." + filters[i] + " files");
                   filterStr.push_back('\0');
                   filterStr.append("*." + filters[i]);
                   filterStr.push_back('\0');
                }
                ofn.lpstrFilter = filterStr.c_str();
            }
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

        string commandOutput = os::execCommand(command);
        if(!commandOutput.empty())
            output["selectedEntry"] = commandOutput;
        else
            output["selectedEntry"] = nullptr;
        
        #elif defined(__APPLE__)
        string command = "osascript -e 'POSIX path of (choose file name";
        if(!input["title"].is_null())
            command += " with prompt \"" + input["title"].get<std::string>() + "\"";
        command += ")'";
        string commandOutput = os::execCommand(command);
        if(!commandOutput.empty())
            output["selectedEntry"] = commandOutput;
        else
            output["selectedEntry"] = nullptr;
        
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
        os::execCommand(command);
        output["success"] = true;
        
        #elif defined(_WIN32)
        string command = "powershell -Command \"& {Add-Type -AssemblyName System.Windows.Forms;"
                        "Add-Type -AssemblyName System.Drawing;"
                        "$notify = New-Object System.Windows.Forms.NotifyIcon;"
                        "$notify.Icon = [System.Drawing.SystemIcons]::Information;"
                        "$notify.Visible = $true;"
                        "$notify.ShowBalloonTip(0 ,'"+ input["summary"].get<string>() + "','" + input["body"].get<string>() + "',[System.Windows.Forms.TooltipIcon]::None)}\"";

        string commandOutput = os::execCommand(command);

        output["success"] = true;
        #endif
        return output;
    }

    json showMessageBox(json input) {
        json output;
        os::MessageBoxOptions msgBoxOptions;
        os::MessageBoxResult msgBoxResult;
        msgBoxOptions.type = input["type"];
        msgBoxOptions.title = input["title"];
        msgBoxOptions.content = input["content"];
        msgBoxResult = os::showMessageBox(msgBoxOptions);
        if(msgBoxResult.hasError) {
            output["error"] = msgBoxResult.error;
        }
        else {
            if(msgBoxOptions.type == "QUESTION")
                output["yesButtonClicked"] = msgBoxResult.yesButtonClicked;
            output["success"] = true;
        }
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
                fullIconPath = fs::getFullPathFromRelative(settings::joinAppPath("")) + iconPath;
            }
            else {
                json createDirParams;
                createDirParams["path"] = settings::joinAppPath("/.tmp");
                fs::createDirectory(createDirParams);
                string tempIconPath = settings::joinAppPath("/.tmp/tray_icon_linux.png");
                resources::extractFile(iconPath, tempIconPath);
                fullIconPath = fs::getFullPathFromRelative(tempIconPath);
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
