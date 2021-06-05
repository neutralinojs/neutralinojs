#include <iostream>
#include <fstream>
#include <algorithm>
#include <regex>
#include "lib/easylogging/easylogging++.h"
#include "lib/json.hpp"
#include "auth/authbasic.h"
#include "resources.h"
#include "helpers.h"
#include "platform/platform.h"

#if defined(__linux__)
#define OS_NAME "Linux"

#elif defined(_WIN32)
#define OS_NAME "Windows"

#elif defined(__APPLE__)
#define OS_NAME "Darwin"
#endif

#define APP_CONFIG_FILE "/neutralino.config.json"

using namespace std;
using json = nlohmann::json;
json options;
json globalArgs;
bool loadResFromDir = false;
string appPath;

namespace settings {

    string joinAppPath(string filename) {
        return appPath + filename;
    }

    string getFileContent(string filename) {
        if(!loadResFromDir)
            return resources::getFileContent(filename);
        filename = settings::joinAppPath(filename);
        vector<char> buffer;
        ifstream ifd(filename.c_str(), ios::binary | ios::ate);
        if(!ifd.is_open())
            return "";
        int size = ifd.tellg();
        ifd.seekg(0, ios::beg);
        buffer.resize(size);
        ifd.read(buffer.data(), size);
        string result(buffer.begin(), buffer.end());
        return result;
    }

    json getConfig() {
        if(!options.is_null())
            return options;
        json config;
        try {
            config = json::parse(settings::getFileContent(APP_CONFIG_FILE));
        }
        catch(exception e){
            LOG(ERROR) << e.what();
        }
        options = config;
        return options;
    }

    string getGlobalVars(){
        string jsSnippet = "var NL_OS='" + std::string(OS_NAME) + "';";
        jsSnippet += "var NL_VERSION='2.1.1';";
        jsSnippet += "var NL_APPID='" + options["applicationId"].get<std::string>() + "';";
        jsSnippet += "var NL_PORT=" + std::to_string(options["port"].get<int>()) + ";";
        jsSnippet += "var NL_MODE='" + options["defaultMode"].get<std::string>() + "';";
        jsSnippet += "var NL_TOKEN='" + authbasic::getToken() + "';";
        jsSnippet += "var NL_CWD='" + platform::getCurrentDirectory() + "';";
        jsSnippet += "var NL_ARGS=" + globalArgs.dump() + ";";
        jsSnippet += "var NL_PATH='" + appPath + "';";

        if(!options["globalVariables"].is_null()) {
            for ( auto it: options["globalVariables"].items()) {
                jsSnippet += "var NL_" + it.key() +  "='" + it.value().get<std::string>() + "';";
            }
        }
        return jsSnippet;
    }

    void setGlobalArgs(json args) {
        int argIndex = 0;
        for(string arg : args) {
            // -- Set default path
            if(argIndex == 0) {
                appPath = platform::getDirectoryName(args[argIndex].get<std::string>());
                if(appPath == "")
                    appPath = platform::getCurrentDirectory();
                globalArgs = args;
            }
            
            // Resources read mode (res.neu or from directory)
            if(arg == "--load-dir-res") {
                loadResFromDir = true;
            }
            
            // Override default path
            if(regex_match(arg, regex("--path=.*"))) {
                vector <string> pathArgParts = helpers::split(arg, '=');
                if(pathArgParts.size() == 2 && pathArgParts[1].length() > 0)
                    appPath = pathArgParts[1];
            }
            argIndex++;
        }
    }

    string getMode() {
        return options["defaultMode"].get<std::string>();
    }

    void setPort(int port) {
      options["port"] = port;
    }

}
