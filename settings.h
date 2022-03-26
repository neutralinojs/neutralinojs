#ifndef NEU_SETTINGS_H
#define NEU_SETTINGS_H

#if defined(__linux__)
#define OS_NAME "Linux"

#elif defined(_WIN32)
#define OS_NAME "Windows"

#elif defined(__APPLE__)
#define OS_NAME "Darwin"

#elif defined(__FreeBSD__)
#define OS_NAME "FreeBSD"
#endif

#define APP_CONFIG_FILE "/neutralino.config.json"

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

json getConfig();
string getGlobalVars();
void setGlobalArgs(const json &args);
string joinAppPath(const string &filename);
string getAppPath();
string getMode();
void setPort(int port);
void applyConfigOverride(const settings::CliArg &arg);
json getOptionForCurrentMode(const string &key);

} // namesapce settings

#endif // #define NEU_SETTINGS_H

