#ifndef NEU_SETTINGS_H
#define NEU_SETTINGS_H

#if defined(__linux__)
#define NEU_OS_NAME "Linux"

#elif defined(_WIN32)
#define NEU_OS_NAME "Windows"

#elif defined(__APPLE__)
#define NEU_OS_NAME "Darwin"

#elif defined(__FreeBSD__)
#define NEU_OS_NAME "FreeBSD"

#else
#define NEU_OS_NAME "Unknown"

#endif

#define NEU_APP_CONFIG_FILE "/neutralino.config.json"

#if !defined(NEU_COMPILATION_DATA)
#define NEU_COMPILATION_DATA ""
#endif

#include <string>

#include "lib/json/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace settings {

struct CliArg {
    string key;
    string value;
};

struct ConfigOverride {
    string key;
    string value;
    string convertTo;
};

enum AppMode { AppModeWindow, AppModeBrowser, AppModeCloud, AppModeChrome };

bool init();
json getConfig();
string getAppId();
string getNavigationUrl();
string getGlobalVars();
void setGlobalArgs(const json &args);
string joinAppPath(const string &filename);
string joinSystemDataPath(const string &filename);
string joinAppDataPath(const string &filename);
string getAppPath();
string getConfigFile();
settings::AppMode getMode();
void setPort(int port);
void applyConfigOverride(const settings::CliArg &arg);
json getOptionForCurrentMode(const string &key);

} // namesapce settings

#endif // #define NEU_SETTINGS_H

