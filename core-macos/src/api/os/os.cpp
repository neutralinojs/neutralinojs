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
#include <dispatch/dispatch.h>
#include "../../platform/macos.h"
#include <lib/boxer/boxer.h>
#include <objc/objc-runtime.h>

using namespace std;
using json = nlohmann::json;

namespace os {

    string execCommand(json input) {
        json output;
        string command = input["command"];
        output["output"] = macos::execCommand(command);
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
        string command = "osascript -e 'POSIX path of (choose ";
        if(!input["isDirectoryMode"].is_null() && input["isDirectoryMode"].get<bool>())
            command += "folder";
        else
            command += "file";
        if(!input["title"].is_null())
            command += " with prompt \"" + input["title"].get<std::string>() + "\"";
        command += ")'";
        output["selectedEntry"] = macos::execCommand(command);
        output["success"] = true;
        return output.dump();
    }

    string dialogSave(json input) {
        json output;
        string command = "osascript -e 'POSIX path of (choose file name";
        if(!input["title"].is_null())
            command += " with prompt \"" + input["title"].get<std::string>() + "\"";
        command += ")'";
        output["selectedEntry"] = macos::execCommand(command);
        output["success"] = true;
        return output.dump();
    }

    string showNotification(json input) {
        json output;
        string command = "osascript -e 'display notification \"" + 
        input["body"].get<std::string>() + "\"";
        if(!input["summary"].is_null())
            command += " with title \"" + input["summary"].get<std::string>() + "\"";
        command += "'";
        macos::execCommand(command);
        output["success"] = true;
        return output.dump();
    }

    string showMessageBox(json input) {
        __block json output;
        __block boxer::Selection msgSel;
        string title = input["title"];
        string content = input["content"];
        string type = input["type"];

        dispatch_sync(dispatch_get_main_queue(), ^{
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
        });
        if(output["error"].is_null())
            output["success"] = true;
        return output.dump();
    }
}
