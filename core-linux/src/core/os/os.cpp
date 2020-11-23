#include <iostream>
#include <fstream>
#include "../../../lib/json/json.hpp"
#include <stdlib.h>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <gtk/gtk.h>

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
        std::array<char, 128> buffer;
        std::string result;
        std::shared_ptr<FILE> pipe(popen(command.c_str(), "r"), pclose);
        if (!pipe) {
            output["error"] = "Pipe open failed";
        }
        else {
            while (!feof(pipe.get())) {
                if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
                    result += buffer.data();
            }
            output["stdout"] = result;
        }
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

        char file[1024];
        FILE *f = popen("zenity --file-selection --title \"Open a file\"", "r");
        fgets(file, 1024, f);

        output["file"] = file;

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

        char file[1024];
        FILE *f = popen("zenity --file-selection --title \"Save a file\" --save", "r");
        fgets(file, 1024, f);

        output["file"] = file;

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
}
