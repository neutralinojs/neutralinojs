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

    string runCommand(string jso) {
        json input;
        json output;
        try {
            input = json::parse(jso);
        }
        catch(exception e){
            output["error"] = "JSON parse error is occurred!";
            return output.dump();
        }
        string command = input["command"];
        output["stdout"] = linux::execCommand(command);
        return output.dump();
    }

    string getEnvar(string jso) {
        json input;
        json output;
        try {
            input = json::parse(jso);
        }
        catch(exception e){
            output["error"] = "JSON parse error is occurred!";
            return output.dump();
        }
        string i = input["name"];
        char *o;
        o = getenv(i.c_str());
        if(o == NULL) {
            output["error"] =  i + " is not defined";
        }
        else {
            output["value"] = o;
        }
        return output.dump();

    }


    string dialogOpen(string jso) {
        json input;
        json output;
        try {
            input = json::parse(jso);
        }
        catch(exception e){
            output["error"] = "JSON parse error is occurred!";
            return output.dump();
        }
        string command = "zenity --file-selection";
        if(!input["title"].is_null())
            command += " --title \"" + input["title"].get<std::string>() + "\"";
        if(!input["isDirectoryMode"].is_null() && input["isDirectoryMode"].get<bool>())
            command += " --directory";
        output["file"] = linux::execCommand(command);
        return output.dump();
    }


    string dialogSave(string jso) {
        json input;
        json output;
        try {
            input = json::parse(jso);
        }
        catch(exception e){
            output["error"] = "JSON parse error is occurred!";
            return output.dump();
        }
        string command = "zenity --file-selection --save";
        if(!input["title"].is_null())
            command += " --title \"" + input["title"].get<std::string>() + "\"";
        output["file"] = linux::execCommand(command);
        return output.dump();
    }

    string showNotification(string jso) {
        json input;
        json output;
        try {
            input = json::parse(jso);
        }
        catch(exception e){
            output["error"] = "JSON parse error is occurred!";
            return output.dump();
        }
        string command = "notify-send \"" + input["summary"].get<string>() + "\" \"" +
                            input["body"].get<string>() + "\"";
        if(system(command.c_str()) == 0)
            output["message"] = "Notification is pushed to the system";
        else
            output["error"] = "An error thrown while sending the notification";
        return output.dump();
    }

    string showMessageBox(string jso) {
        json input;
        json output;
        map <string, string> messageTypes = {{"INFO", "info"}, {"WARN", "warning"},
                                            {"ERROR", "error"}, {"QUESTION", "question"}};
        string messageType;
        try {
            input = json::parse(jso);
        }
        catch(exception e){
            output["error"] = "JSON parse error is occurred!";
            return output.dump();
        }
        messageType = input["type"].get<string>();
        if(messageTypes.find(messageType) == messageTypes.end()) {
            output["error"] = "Invalid message type: " + messageType + "' provided";
            return output.dump();
        }
        string command = "zenity --" + messageTypes[messageType] + " --title=\"" +
                            input["title"].get<string>() + "\" --text=\"" +
                            input["content"].get<string>() + "\" && echo $?";
        if(messageType == "QUESTION")
            output["yesClicked"] = linux::execCommand(command).find("0") != std::string::npos;
        return output.dump();
    }
}
