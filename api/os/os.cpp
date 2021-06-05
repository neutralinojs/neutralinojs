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

#if defined(__APPLE__)
#include <dispatch/dispatch.h>
#include <lib/boxer/boxer.h>
#include <objc/objc-runtime.h>

#elif defined(_WIN32)
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <lib/boxer/boxer.h>

#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "Shell32.lib")
#endif

#include "platform/platform.h"



using namespace std;
using json = nlohmann::json;

namespace os {
    string execCommand(json input) {
        json output;
        string command = input["command"];
        output["output"] = platform::execCommand(command);
        output["success"] = true;
        return output.dump();
    }

    string getEnvar(json input) {
        json output;
        string varKey = input["key"];
        char *varValue;
        varValue = getenv(varKey.c_str());
        if(varValue == NULL) {
            output["error"] =  varKey + " is not defined";
        }
        else {
            output["value"] = varValue;
            output["success"] = true;
        }
        return output.dump();

    }

    string dialogOpen(json input) {
        json output;
        #if defined(__linux__)
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
            bInfo.hwndOwner = NULL;
            bInfo.pidlRoot = NULL;
            bInfo.pszDisplayName = szDir;
            bInfo.lpszTitle = const_cast<char *>(title.c_str());
            bInfo.ulFlags = 0 ;
            bInfo.lpfn = NULL;
            bInfo.lParam = 0;
            bInfo.iImage = -1;

            LPITEMIDLIST lpItem = SHBrowseForFolder( &bInfo);
            if( lpItem != NULL ) {
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
            ofn.lpstrTitle = const_cast<char *>(title.c_str());
            ofn.lStructSize = sizeof(ofn);
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
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
        return output.dump();
    }


    string dialogSave(json input) {
        json output;
        #if defined(__linux__)
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
        ofn.lpstrTitle = const_cast<char *>(title.c_str());
        ofn.lStructSize = sizeof(ofn);
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST;

        if (GetSaveFileName(&ofn)) {
            output["selectedEntry"] = ofn.lpstrFile;
        }
        else {
            output["selectedEntry"] = nullptr;
        }
        #endif
        output["success"] = true;
        return output.dump();
    }

    string showNotification(json input) {
        json output;
        #if defined(__linux__)
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
        return output.dump();
    }

    string showMessageBox(json input) {
        #if defined(__linux__)
        json output;
        map <string, string> messageTypes = {{"INFO", "info"}, {"WARN", "warning"},
                                            {"ERROR", "error"}, {"QUESTION", "question"}};
        string messageType;
        messageType = input["type"].get<string>();
        if(messageTypes.find(messageType) == messageTypes.end()) {
            output["error"] = "Invalid message type: '" + messageType + "' provided";
            return output.dump();
        }
        string command = "zenity --" + messageTypes[messageType] + " --title=\"" +
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

            #if defined(__APPLE__)
            __block json output;
            __block boxer::Selection msgSel;
            #elif defined(_WIN32)
            json output;
            boxer::Selection msgSel;
            #endif
            
            #if defined(__APPLE__)
            dispatch_sync(dispatch_get_main_queue(), ^{
            #endif
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
            #if defined(__APPLE__)
            });
            #endif
            if(output["error"].is_null())
                output["success"] = true;
        
        #endif
        return output.dump();
    }
}
