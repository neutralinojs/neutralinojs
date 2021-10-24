#ifndef NEU_SETTINGS_H
#define NEU_SETTINGS_H

#include <string>

#include "lib/json/json.hpp"
#include "api/filesystem/filesystem.h"

using namespace std;
using json = nlohmann::json;
extern bool loadResFromDir;

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
    fs::FileReaderResult getFileContent(const string &filename);
    string getGlobalVars();
    void setGlobalArgs(const json &args);
    string joinAppPath(const string &filename);
    string getMode();
    void setPort(int port);
    settings::CliArg _parseArg(const string &arg);
    void applyConfigOverride(const settings::CliArg &arg);
    json getOptionForCurrentMode(const string &key);
} // namesapce settings

#endif // #define NEU_SETTINGS_H

