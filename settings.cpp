#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <regex>

#include "lib/json/json.hpp"
#include "settings.h"
#include "resources.h"
#include "helpers.h"
#include "auth/authbasic.h"
#include "api/filesystem/filesystem.h"
#include "api/debug/debug.h"
#include "api/app/app.h"

#if defined(__linux__)
#define OS_NAME "Linux"

#elif defined(_WIN32)
#define OS_NAME "Windows"

#elif defined(__APPLE__)
#define OS_NAME "Darwin"

#elif defined(__FreeBSD__)
#define OS_NAME "FreeBSD"
#endif
#define NL_VERSION "3.0.0"

#define APP_CONFIG_FILE "/neutralino.config.json"

using namespace std;
using json = nlohmann::json;

json options;
json globalArgs;
bool loadResFromDir = false;
string appPath;

vector <settings::ConfigOverride> configOverrides;

namespace settings {

    string joinAppPath(const string &filename) {
        return appPath + filename;
    }

    fs::FileReaderResult getFileContent(const string &filename) {
        if(!loadResFromDir)
            return resources::getFileContent(filename);
        fs::FileReaderResult fileReaderResult = fs::readFile(settings::joinAppPath(filename));
        
        if(fileReaderResult.hasError) {
            debug::log("ERROR", fileReaderResult.error);
        }
        return fileReaderResult;
    }

    json getConfig() {
        if(!options.is_null())
            return options;
        json config;
        try {
            fs::FileReaderResult fileReaderResult = settings::getFileContent(APP_CONFIG_FILE);
            config = json::parse(fileReaderResult.data);
            options = config;

            // Apply config overrides
            json patches;
            for(auto cfgOverride: configOverrides) {
                json patch;

                patch["op"] = options[json::json_pointer(cfgOverride.key)].is_null() 
                                    ? "add" : "replace";
                patch["path"] = cfgOverride.key;
                
                // String to actual types
                if(cfgOverride.convertTo == "int") {
                    patch["value"] = stoi(cfgOverride.value);
                }
                else if(cfgOverride.convertTo == "bool") {
                    patch["value"] = cfgOverride.value == "true";
                }
                else {
                    patch["value"] = cfgOverride.value;
                }
                
                patches.push_back(patch);
            }

            if(!patches.is_null()) {
                options = options.patch(patches);
            }
        }
        catch(exception e){
            debug::log("ERROR", "Unable to load: " + string(APP_CONFIG_FILE));
        }
        return options;
    }

    string getGlobalVars(){
        string jsSnippet = "var NL_OS='" + string(OS_NAME) + "';";
        jsSnippet += "var NL_VERSION='" + string(NL_VERSION) + "';";
        jsSnippet += "var NL_APPID='" + options["applicationId"].get<string>() + "';";
        jsSnippet += "var NL_PORT=" + to_string(settings::getOptionForCurrentMode("port").get<int>()) + ";";
        jsSnippet += "var NL_MODE='" + options["defaultMode"].get<string>() + "';";
        jsSnippet += "var NL_TOKEN='" + authbasic::getToken() + "';";
        jsSnippet += "var NL_CWD='" + fs::getCurrentDirectory() + "';";
        jsSnippet += "var NL_ARGS=" + globalArgs.dump() + ";";
        jsSnippet += "var NL_PATH='" + appPath + "';";
        jsSnippet += "var NL_PID=" + to_string(app::getProcessId()) + ";";

        json jGlobalVariables = settings::getOptionForCurrentMode("globalVariables");
        if(!jGlobalVariables.is_null()) {
            for ( auto it: jGlobalVariables.items()) {
                jsSnippet += "var NL_" + it.key() +  "='" + it.value().get<string>() + "';";
            }
        }
        return jsSnippet;
    }

    void setGlobalArgs(const json &args) {
        int argIndex = 0;
        globalArgs = args;
        for(string arg: args) {
            settings::CliArg cliArg = _parseArg(arg); 

            // Set default path
            if(argIndex == 0) {
                appPath = fs::getDirectoryName(arg);
                if(appPath == "")
                    appPath = fs::getCurrentDirectory();
            }
            
            // Resources read mode (res.neu or from directory)
            if(cliArg.key == "--load-dir-res") {
                loadResFromDir = true;
            }
            
            // Set app path context
            if(cliArg.key == "--path") {
                appPath = cliArg.value;
            }
            
            // Override app configs
            applyConfigOverride(cliArg);
            
            argIndex++;
        }
    }

    string getMode() {
        return options["defaultMode"].get<string>();
    }

    void setPort(int port) {
      options["port"] = port;
    }
    
    settings::CliArg _parseArg(const string &argStr) {
        settings::CliArg arg;
        vector<string> argParts = helpers::split(argStr, '=');
        if(argParts.size() == 2 && argParts[1].length() > 0) {
            arg.key = argParts[0];
            arg.value = argParts[1];
        }
        else {
            arg.key = argStr;
        }
        return arg;
    }
    
    void applyConfigOverride(const settings::CliArg &arg) {
        map<string, vector<string>> cliMappings = {
            // Top level
            {"--mode", {"/defaultMode", "string"}},
            {"--url", {"/url", "string"}},
            {"--port", {"/port", "int"}},
            {"--logging-enabled", {"/logging/enabled", "bool"}},
            {"--logging-write-to-log-file", {"/logging/writeToLogFile", "bool"}},
            {"--enable-server", {"/enableServer", "bool"}},
            // Window mode
            {"--window-title", {"/modes/window/title", "string"}},
            {"--window-width", {"/modes/window/width", "int"}},
            {"--window-height", {"/modes/window/height", "int"}},
            {"--window-min-width", {"/modes/window/minWidth", "int"}},
            {"--window-min-height", {"/modes/window/minHeight", "int"}},
            {"--window-max-width", {"/modes/window/maxWidth", "int"}},
            {"--window-max-height", {"/modes/window/maxHeight", "int"}},
            {"--window-full-screen", {"/modes/window/fullScreen", "bool"}},
            {"--window-always-on-top", {"/modes/window/alwaysOnTop", "bool"}},
            {"--window-enable-inspector", {"/modes/window/enableInspector", "bool"}},
            {"--window-borderless", {"/modes/window/borderless", "bool"}},
            {"--window-maximize", {"/modes/window/maximize", "bool"}},
            {"--window-hidden", {"/modes/window/hidden", "bool"}},
            {"--window-resizable", {"/modes/window/resizable", "bool"}},
            {"--window-maximizable", {"/modes/window/maximizable", "bool"}},
            {"--window-exit-process-on-close", {"/modes/window/exitProcessOnClose", "bool"}},
            {"--window-icon", {"/modes/window/icon", "string"}}
        };
        
        map<string, vector<string>> cliMappingAliases = {
            {"/port", {"/modes/window/port", "/modes/browser/port", "/modes/cloud/port"}},
            {"/url", {"/modes/window/url", "/modes/browser/url", "/modes/cloud/url"}},
            {"/logging/enabled", 
                {
                "/modes/window/logging/enabled",
                "/modes/browser/logging/enabled",
                "/modes/cloud/logging/enabled"
                }
            },
            {"/logging/writeToLogFile", 
                {
                "/modes/window/logging/writeToLogFile", 
                "/modes/browser/logging/writeToLogFile", 
                "/modes/cloud/logging/writeToLogFile"
                }
            },
            {"/enableServer", 
                {
                "/modes/window/enableServer", 
                "/modes/browser/enableServer", 
                "/modes/cloud/enableServer"
                }
            },
        };
        
        if(cliMappings.find(arg.key) != cliMappings.end()) {
            if(arg.key == "--mode") {
                if(arg.value != "browser" && arg.value != "window" && arg.value != "cloud") {
                    debug::log("ERROR", "Unsupported mode: '" + arg.value + "'. The default mode is selected.");
                    return;
                }
            }
            settings::ConfigOverride cfgOverride;
            cfgOverride.key = cliMappings[arg.key][0];
            cfgOverride.convertTo = cliMappings[arg.key][1];
            cfgOverride.value = arg.value;

            // Make cases like --window-full-screen -> window-full-screen=true
            if(cfgOverride.convertTo == "bool" && cfgOverride.value.empty()) {
                cfgOverride.value = "true";
            }
            
            // Add original
            configOverrides.push_back(cfgOverride);
            
            // Add aliases
            for(string alias: cliMappingAliases[cfgOverride.key]) {
                settings::ConfigOverride cfgOverrideAlias = cfgOverride;
                cfgOverrideAlias.key = alias;

                configOverrides.push_back(cfgOverrideAlias);
            }
        }
    }
    
    // Priority: mode -> root -> null
    json getOptionForCurrentMode(const string &key) {
        string mode = settings::getMode();
        json value = options["modes"][mode][key];
        if(value.is_null()) {
            value = options[key]; 
        }
        return value;
    }

} // namespace settings
