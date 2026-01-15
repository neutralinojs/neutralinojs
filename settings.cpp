#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <regex>
#include <vector>
#include <map>
#include <set>
#include <clocale>

#include "lib/json/json.hpp"
#include "settings.h"
#include "extensions_loader.h"
#include "resources.h"
#include "helpers.h"
#include "auth/authbasic.h"
#include "api/fs/fs.h"
#include "api/debug/debug.h"
#include "api/app/app.h"
#include "api/custom/custom.h"
#include "api/window/window.h"
#include "api/computer/computer.h"

#include "lib/platformfolders/platform_folders.h"

using namespace std;
using json = nlohmann::json;

namespace settings {

json options;
json globalArgs;
string appPath;
string systemDataPath;
string appDataPath; // appPath or systemDataPath based on config.dataLocation
string configFile = NEU_APP_CONFIG_FILE;
string localeName;

vector<settings::ConfigOverride> configOverrides;

string joinAppPath(const string &filename) {
    return appPath + filename;
}

string joinAppDataPath(const string &filename) {
    return appDataPath + filename;
}

string joinSystemDataPath(const string &filename) {
    return systemDataPath + filename;
}

string getAppPath() {
    return appPath;
}

string getConfigFile() {
    return configFile;
}

bool init() {
    #if defined(_WIN32)
    localeName = helpers::wstr2str(_wsetlocale(LC_ALL, L""));
    #else
    localeName = setlocale(LC_ALL, "");
    #endif
    options = json::object();
    json config;
    fs::FileReaderResult fileReaderResult = resources::getFile(configFile);
    if(fileReaderResult.status == errors::NE_ST_OK) {
        try {
            config = json::parse(fileReaderResult.data);
            options = config;
        }
        catch(exception e) {
            debug::log(debug::LogTypeError, errors::makeErrorMsg(errors::NE_CF_UNBPRCF, string(configFile)));
            return false;
        }
    }
    else {
        debug::log(debug::LogTypeError, errors::makeErrorMsg(errors::NE_CF_UNBLDCF, string(configFile)));
    }

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

    systemDataPath = sago::getDataHome() + "/" + settings::getAppId();
    systemDataPath = helpers::normalizePath(systemDataPath);

    string dataLoc = "app";
    json jLoc = settings::getOptionForCurrentMode("dataLocation");
    if(!jLoc.is_null()) {
        dataLoc = jLoc.get<string>();
    }
    
    appDataPath = dataLoc == "system" ? systemDataPath : appPath;

    return true;
}

json getConfig() {
    return options;
}

string getAppId() {
    if(!options["applicationId"].is_null()) {
        string appId = options["applicationId"].get<string>();
        appId = regex_replace(appId, regex("[^\\w.]"), "");
        return regex_replace(appId, regex("[.]{2,}"), ".");
    }
    return "js.neutralino.framework";
}

string getNavigationUrl() {
    return !settings::getOptionForCurrentMode("url").is_null() ?
        settings::getOptionForCurrentMode("url").get<string>() : "https://neutralino.js.org";
}

string getGlobalVars(){
    string jsSnippet = "var NL_OS='" + string(NEU_OS_NAME) + "';";
    jsSnippet += "var NL_ARCH='" + computer::getArch() + "';";
    jsSnippet += "var NL_VERSION='" + string(NEU_VERSION) + "';";
    jsSnippet += "var NL_COMMIT='" + string(NEU_COMMIT) + "';";
    jsSnippet += "var NL_APPID='" + settings::getAppId() + "';";
    if(!options["version"].is_null()) {
        jsSnippet += "var NL_APPVERSION='" + options["version"].get<string>() + "';";
    }
    jsSnippet += "var NL_PORT=" + to_string(settings::getOptionForCurrentMode("port").get<int>()) + ";";
    jsSnippet += "var NL_MODE='" + helpers::appModeToStr(settings::getMode()) + "';";
    jsSnippet += "var NL_TOKEN='" + authbasic::getToken() + "';";
    jsSnippet += "var NL_CWD='" + fs::getCurrentDirectory() + "';";
    jsSnippet += "var NL_ARGS=" + helpers::jsonToString(globalArgs) + ";";
    jsSnippet += "var NL_PATH='" + appPath + "';";
    jsSnippet += "var NL_DATAPATH='" + appDataPath + "';";
    jsSnippet += "var NL_PID=" + to_string(app::getProcessId()) + ";";
    jsSnippet += "var NL_RESMODE='" + resources::getModeString() + "';";
    jsSnippet += "var NL_EXTENABLED=" + helpers::jsonToString(json(extensions::isInitialized())) + ";";
    jsSnippet += "var NL_CMETHODS=" + helpers::jsonToString(json(custom::getMethods())) + ";";
    jsSnippet += "var NL_WSAVSTLOADED=" + helpers::jsonToString(json(window::isSavedStateLoaded())) + ";";
    jsSnippet += "var NL_CONFIGFILE='" + settings::getConfigFile() + "';";
    jsSnippet += "var NL_LOCALE='" + localeName + "';";
    jsSnippet += "var NL_COMPDATA='" + string(NEU_COMPILATION_DATA) + "';";

    json jGlobalVariables = settings::getOptionForCurrentMode("globalVariables");
    if(!jGlobalVariables.is_null()) {
        for(const auto &it: jGlobalVariables.items()) {
            jsSnippet += "var NL_" + it.key() +  "=JSON.parse('" + helpers::jsonToString(it.value()) + "');";
        }
    }
    return jsSnippet;
}

settings::CliArg _parseArg(const string &argStr) {
    settings::CliArg arg;
    vector<string> argParts = helpers::splitTwo(argStr, '=');
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

        // DEPRECATED: Resources read mode (resources.neu or from directory)
        if(cliArg.key == "--load-dir-res") {
            resources::setMode(resources::ResourceModeDir);
            continue;
        }

        // Resource mode (resources.neu, from directory or embedded)
        if(cliArg.key == "--res-mode") {
            if (cliArg.value == "directory") {
                resources::setMode(resources::ResourceModeDir);
            }
            else if (cliArg.value == "bundle") {
                resources::setMode(resources::ResourceModeBundle);
            }
            else if (cliArg.value == "embedded") {
                resources::setMode(resources::ResourceModeEmbedded);
            }
            continue;
        }

        // Set app path context
        if(cliArg.key == "--path") {
            appPath = cliArg.value;
            continue;
        }

        // Set app config file name
        if(cliArg.key == "--config-file") {
            configFile = cliArg.value;
            continue;
        }

        // Enable dev tools connection (as an extension)
        // Not available for production (resources.neu-based) apps
        if(cliArg.key == "--neu-dev-extension" && !resources::isBundleMode() && !resources::isEmbeddedMode()) {
            extensions::loadOne("js.neutralino.devtools");
            continue;
        }

        // Override app configs
        applyConfigOverride(cliArg);

        argIndex++;
    }
}

settings::AppMode getMode() {
    string mode = !options["defaultMode"].is_null() ?
                    options["defaultMode"].get<string>() : "window";
    if(mode == "window") return settings::AppModeWindow;
    if(mode == "browser") return settings::AppModeBrowser;
    if(mode == "cloud") return settings::AppModeCloud;
    if(mode == "chrome") return settings::AppModeChrome;
    return settings::AppModeWindow;
}

void setPort(int port) {
    options["port"] = port;

    options["/modes/window/port"_json_pointer] = port;
    options["/modes/browser/port"_json_pointer] = port;
    options["/modes/cloud/port"_json_pointer] = port;
    options["/modes/chrome/port"_json_pointer] = port;
}

void applyConfigOverride(const settings::CliArg &arg) {
    map<string, vector<string>> cliMappings = {
        // Top level
        {"--mode", {"/defaultMode", "string"}},
        {"--url", {"/url", "string"}},
        {"--document-root", {"/documentRoot", "string"}},
        {"--port", {"/port", "int"}},
        {"--logging-enabled", {"/logging/enabled", "bool"}},
        {"--logging-write-to-log-file", {"/logging/writeToLogFile", "bool"}},
        {"--enable-server", {"/enableServer", "bool"}},
        {"--enable-native-api", {"/enableNativeAPI", "bool"}},
        {"--single-page-serve", {"/singlePageServe", "bool"}},
        {"--enable-extensions", {"/enableExtensions", "bool"}},
        {"--export-auth-info", {"/exportAuthInfo", "bool"}},
        {"--data-location", {"/dataLocation", "string"}},
        {"--storage-location", {"/storageLocation", "string"}},
        // Window mode
        {"--window-title", {"/modes/window/title", "string"}},
        {"--window-width", {"/modes/window/width", "int"}},
        {"--window-height", {"/modes/window/height", "int"}},
        {"--window-min-width", {"/modes/window/minWidth", "int"}},
        {"--window-min-height", {"/modes/window/minHeight", "int"}},
        {"--window-max-width", {"/modes/window/maxWidth", "int"}},
        {"--window-max-height", {"/modes/window/maxHeight", "int"}},
        {"--window-x", {"/modes/window/x", "int"}},
        {"--window-y", {"/modes/window/y", "int"}},
        {"--window-full-screen", {"/modes/window/fullScreen", "bool"}},
        {"--window-always-on-top", {"/modes/window/alwaysOnTop", "bool"}},
        {"--window-enable-inspector", {"/modes/window/enableInspector", "bool"}},
        {"--window-open-inspector-on-startup", {"/modes/window/openInspectorOnStartup", "bool"}},
        {"--window-borderless", {"/modes/window/borderless", "bool"}},
        {"--window-maximize", {"/modes/window/maximize", "bool"}},
        {"--window-hidden", {"/modes/window/hidden", "bool"}},
        {"--window-resizable", {"/modes/window/resizable", "bool"}},
        {"--window-maximizable", {"/modes/window/maximizable", "bool"}},
        {"--window-center", {"/modes/window/center", "bool"}},
        {"--window-transparent", {"/modes/window/transparent", "bool"}},
        {"--window-skip-taskbar", {"/modes/window/skipTaskbar", "bool"}},
        {"--window-exit-process-on-close", {"/modes/window/exitProcessOnClose", "bool"}},
        {"--window-use-saved-state", {"/modes/window/useSavedState", "bool"}},
        {"--window-icon", {"/modes/window/icon", "string"}},
        {"--window-extend-user-agent-with", {"/modes/window/extendUserAgentWith", "string"}},
        {"--window-inject-globals", {"/modes/window/injectGlobals", "bool"}},
        {"--window-inject-client-library", {"/modes/window/injectClientLibrary", "bool"}},
        {"--window-inject-script", {"/modes/window/injectScript", "string"}},
        // Chrome mode
        {"--chrome-width", {"/modes/chrome/width", "int"}},
        {"--chrome-height", {"/modes/chrome/height", "int"}},
        {"--chrome-args", {"/modes/chrome/args", "string"}},
        {"--chrome-browser-binary", {"/modes/chrome/browserBinary", "string"}}
    };

    // Allows overriding from modes
    // So, update all modes' values from CLI args
    set<string> cliMappingAliases = {
        "/port",
        "/url",
        "/documentRoot",
        "/logging/enabled",
        "/logging/writeToLogFile",
        "/enableServer",
        "/enableNativeAPI",
        "/singlePageServe",
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
    string mode = helpers::appModeToStr(settings::getMode());
    json value = options["modes"][mode][key];
    if(value.is_null()) {
        value = options[key];
    }
    return value;
}

} // namespace settings
