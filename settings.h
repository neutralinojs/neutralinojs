#include <string>

#include "lib/json.hpp"
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
    fs::FileReaderResult getFileContent(string filename);
    string getGlobalVars();
    void setGlobalArgs(json args);
    string joinAppPath(string filename);
    string getMode();
    void setPort(int port);
    settings::CliArg _parseArg(string arg);
    void applyConfigOverride(settings::CliArg arg);
    json getOptionForCurrentMode(string key);
}

