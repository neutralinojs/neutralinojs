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
#include <gtk/gtk.h>
#include "../../platform/linux.h"

using namespace std;
using json = nlohmann::json;

namespace os {

    string execCommand(json input) {
        json output;
        string command = input["command"];
        output["output"] = linux::execCommand(command);
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
        string command = "zenity --file-selection";
        if(!input["title"].is_null())
            command += " --title \"" + input["title"].get<std::string>() + "\"";
        if(!input["isDirectoryMode"].is_null() && input["isDirectoryMode"].get<bool>())
            command += " --directory";
        output["selectedEntry"] = linux::execCommand(command);
        output["success"] = true;
        return output.dump();
    }


    string dialogSave(json input) {
        json output;
        string command = "zenity --file-selection --save";
        if(!input["title"].is_null())
            command += " --title \"" + input["title"].get<std::string>() + "\"";
        output["selectedEntry"] = linux::execCommand(command);
        output["success"] = true;
        return output.dump();
    }

    string showNotification(json input) {
        json output;
        string command = "notify-send \"" + input["summary"].get<string>() + "\" \"" +
                            input["body"].get<string>() + "\"";
        if(system(command.c_str()) == 0) {
            output["success"] = true;
            output["message"] = "Notification was sent to the system";
        }
        else
            output["error"] = "An error thrown while sending the notification";
        return output.dump();
    }

    string showMessageBox(json input) {
        json output;
        map <string, string> messageTypes = {{"INFO", "info"}, {"WARN", "warning"},
                                            {"ERROR", "error"}, {"QUESTION", "question"}};
        string messageType;
        messageType = input["type"].get<string>();
        if(messageTypes.find(messageType) == messageTypes.end()) {
            output["error"] = "Invalid message type: " + messageType + "' provided";
            return output.dump();
        }
        string command = "zenity --" + messageTypes[messageType] + " --title=\"" +
                            input["title"].get<string>() + "\" --text=\"" +
                            input["content"].get<string>() + "\" && echo $?";
        string response = linux::execCommand(command);
        if(messageType == "QUESTION")
            output["yesButtonClicked"] =  response.find("0") != std::string::npos;
        output["success"] = true;
        return output.dump();
    }
}
