#include <iostream>
#include <fstream>
#include "nlohmann/json.hpp"
#include "auth/authbasic.h"
#include "core/include/log.h"
#include "functions.h"


using namespace std;
using json = nlohmann::json;
json options;

namespace settings {
    string getFileContent(string filename){
        filename = functions::getAppPath() + "/" + filename;
        ifstream t;
        t.open(filename);
        if(!t.is_open())
            return "";
        string buffer = "";
        string line;
        while(!t.eof()){
            getline(t, line);
            buffer += line + "\n";
        }
        t.close();
        return buffer;
    }

    string getFileContentBinary(string filename){
        filename = functions::getAppPath() + "/" + filename;
        string result = "";
        vector<char> buffer;
        ifstream ifd(filename.c_str(), ios::binary | ios::ate);
        if(!ifd.is_open())
            return "";
        int size = ifd.tellg();
        ifd.seekg(0, ios::beg);
        buffer.resize(size);
        ifd.read(buffer.data(), size);
        result = string(buffer.begin(), buffer.end());
        return result;
    }

    json getOptions(){
        return options;
    }

    void setOption(string key, string value) {
        options[key] = value;
    }

    json getSettings() {
        json settings;
        try {
            std::string raw = getFileContent("app/settings.json");
            settings = json::parse(raw);
        }
        catch(std::exception& e) {
            ERROR() << e.what();
        }
        options = settings;
        return options;
    }

    string getGlobalVars(){
        json settings = getOptions();
        string s = "var NL_OS='MacOS(Darwin)';";
        s += "var NL_VERSION='1.5.0';";
        s += "var NL_NAME='" + settings["appname"].get<std::string>() + "';";
        s += "var NL_PORT=" + settings["appport"].get<std::string>() + ";";
        s += "var NL_MODE='" + settings["mode"].get<std::string>() + "';";
        s += "var NL_TOKEN='" + authbasic::getToken() + "';";
        s += "var NL_CWD='" + functions::getAppPath() + "';";

        if(settings["globals"] != NULL) {
            for ( auto it: settings["globals"].items()) {
                s += "var NL_" + it.key() +  "='" + it.value().get<std::string>() + "';";
            }
        }
        return s;
    }

}
