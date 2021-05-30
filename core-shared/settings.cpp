#include <iostream>
#include <fstream>
#include <algorithm>
#include "lib/easylogging/easylogging++.h"
#include "lib/json.hpp"
#include "auth/authbasic.h"
#include "resources.h"

#if defined(__linux__)
#include "../core-linux/src/platform/linux.h"
#define OS_NAME "Linux"
#define PLATFORM_NS linux

#elif defined(_WIN32)
#include "../core-windows/src/platform/windows.h"
#define OS_NAME "Windows"
#define PLATFORM_NS windows

#elif defined(__APPLE__)
#include "../core-macos/src/platform/macos.h"
#define OS_NAME "Darwin"
#define PLATFORM_NS macos
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
        jsSnippet += "var NL_VERSION='2.1.0';";
        jsSnippet += "var NL_APPID='" + options["applicationId"].get<std::string>() + "';";
        jsSnippet += "var NL_PORT=" + std::to_string(options["port"].get<int>()) + ";";
        jsSnippet += "var NL_MODE='" + options["defaultMode"].get<std::string>() + "';";
        jsSnippet += "var NL_TOKEN='" + authbasic::getToken() + "';";
        jsSnippet += "var NL_CWD='" + PLATFORM_NS::getCurrentDirectory() + "';";
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
        appPath = PLATFORM_NS::getDirectoryName(args[0].get<std::string>());
        if(appPath == "")
            appPath = PLATFORM_NS::getCurrentDirectory();
        globalArgs = args;
        loadResFromDir = std::find(globalArgs.begin(), globalArgs.end(), "--load-dir-res") != globalArgs.end();
    }

    string getMode() {
        return options["defaultMode"].get<std::string>();
    }

    void setPort(int port) {
      options["port"] = port;
    }

}
