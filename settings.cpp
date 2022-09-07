#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <regex>
#include <vector>
#include <map>
#include <set>

#include "lib/json/json.hpp"
#include "settings.h"
#include "extensions_loader.h"
#include "resources.h"
#include "helpers.h"
#include "auth/authbasic.h"
#include "api/filesystem/filesystem.h"
#include "api/debug/debug.h"
#include "api/app/app.h"

using namespace std;
using json = nlohmann::json;

namespace settings {

json options;
json globalArgs;
string appPath;

vector<settings::ConfigOverride> configOverrides;

string joinAppPath(const string &filename) {
    return appPath + filename;
}

string getAppPath() {
    return appPath;
}

json getConfig() {
    if(!options.is_null())
        return options;
    json config;
    try {
        fs::FileReaderResult fileReaderResult = resources::getFile(APP_CONFIG_FILE);
        config = json::parse(fileReaderResult.data);
        options = config;

        // Apply config overrides
        json patches;
        for(const auto &cfgOverride: configOverrides) {
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
    catch(exception e) {
        debug::log(debug::LogTypeError, errors::makeErrorMsg(errors::NE_CF_UNBLDCF, string(APP_CONFIG_FILE)));
    }
    return options;
}

string getGlobalVars(){
    string jsSnippet = "var NL_OS='" + string(OS_NAME) + "';";
    jsSnippet += "var NL_VERSION='" + string(NL_VERSION) + "';";
    jsSnippet += "var NL_COMMIT='" + string(NL_COMMIT) + "';";
    jsSnippet += "var NL_APPID='" + options["applicationId"].get<string>() + "';";
    if(!options["version"].is_null()) {
        jsSnippet += "var NL_APPVERSION='" + options["version"].get<string>() + "';";
    }
    jsSnippet += "var NL_PORT=" + to_string(settings::getOptionForCurrentMode("port").get<int>()) + ";";
    jsSnippet += "var NL_MODE='" + options["defaultMode"].get<string>() + "';";
    jsSnippet += "var NL_TOKEN='" + authbasic::getToken() + "';";
    jsSnippet += "var NL_CWD='" + fs::getCurrentDirectory() + "';";
    jsSnippet += "var NL_ARGS=" + globalArgs.dump() + ";";
    jsSnippet += "var NL_PATH='" + appPath + "';";
    jsSnippet += "var NL_PID=" + to_string(app::getProcessId()) + ";";
    jsSnippet += "var NL_RESMODE='" + resources::getModeString() + "';";
    jsSnippet += "var NL_EXTENABLED=" + json(extensions::isInitialized()).dump() + ";";

    json jGlobalVariables = settings::getOptionForCurrentMode("globalVariables");
    if(!jGlobalVariables.is_null()) {
        for(const auto &it: jGlobalVariables.items()) {
            jsSnippet += "var NL_" + it.key() +  "=JSON.parse('" + it.value().dump() + "');";
        }
    }
    return jsSnippet;
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

void setGlobalArgs(const json &args) {
    int argIndex = 0;
    globalArgs = args;
    for(const string &arg: args) {
        settings::CliArg cliArg = _parseArg(arg);

        // Set default path
        if(argIndex == 0) {
            appPath = fs::getDirectoryName(arg);
            if(appPath == "")
                appPath = fs::getCurrentDirectory();
        }

        // Resources read mode (resources.neu or from directory)
        if(cliArg.key == "--load-dir-res") {
            resources::setMode(resources::ResourceModeDir);
            continue;
        }

        // Set app path context
        if(cliArg.key == "--path") {
            appPath = cliArg.value;
            continue;
        }

        // Enable dev tools connection (as an extension)
        // Not available for production (resources.neu-based) apps
        if(cliArg.key == "--neu-dev-extension" && resources::getMode() == resources::ResourceModeDir) {
            extensions::loadOne("js.neutralino.devtools");
            continue;
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

    options["/modes/window/port"_json_pointer] = port;
    options["/modes/browser/port"_json_pointer] = port;
    options["/modes/cloud/port"_json_pointer] = port;
}

void applyConfigOverride(const settings::CliArg &arg) {
    map<string, vector<string>> cliMappings = {
        // Top level
        {"--mode", {"/defaultMode", "string"}},
        {"--document-root", {"/documentRoot", "string"}},
        {"--url", {"/url", "string"}},
        {"--port", {"/port", "int"}},
        {"--logging-enabled", {"/logging/enabled", "bool"}},
        {"--logging-write-to-log-file", {"/logging/writeToLogFile", "bool"}},
        {"--enable-server", {"/enableServer", "bool"}},
        {"--enable-extensions", {"/enableExtensions", "bool"}},
        {"--export-auth-info", {"/exportAuthInfo", "bool"}},
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
        {"--window-icon", {"/modes/window/icon", "string"}},
        // Chrome mode
        {"--chrome-width", {"/modes/chrome/width", "int"}},
        {"--chrome-height", {"/modes/chrome/height", "int"}},
        {"--chrome-args", {"/modes/chrome/args", "string"}}
    };

    // Allows overriding from modes
    // So, update all modes' values from CLI args
    set<string> cliMappingAliases = {
        "/port",
        "/url",
        "/logging/enabled",
        "/logging/writeToLogFile",
        "/enableServer",
        "/enableExtensions",
        "/exportAuthInfo"
    };

    if(cliMappings.find(arg.key) != cliMappings.end()) {
        if(arg.key == "--mode") {
            vector<string> modes = helpers::getModes();

            if(find(modes.begin(), modes.end(), arg.value) == modes.end()) {
                debug::log(debug::LogTypeError,  errors::makeErrorMsg(errors::NE_CF_UNSUPMD, arg.value));
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
        if(cliMappingAliases.find(cfgOverride.key) != cliMappingAliases.end()) {
            vector<string> modes = helpers::getModes();
            for(const string &mode: modes) {
                settings::ConfigOverride cfgOverrideAlias = cfgOverride;
                cfgOverrideAlias.key = "/modes/" + mode + cfgOverride.key;

                configOverrides.push_back(cfgOverrideAlias);
            }
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
