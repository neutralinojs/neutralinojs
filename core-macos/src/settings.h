#ifndef SETTINGS_H
#define SETTINGS_H
#include <string>
#include "nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace settings {
    json getSettings();
    json getOptions();
    void setOption(string key, string value);
    string getFileContent(string filename);
    string getFileContentBinary(string filename);
    string getGlobalVars();
}
#endif

