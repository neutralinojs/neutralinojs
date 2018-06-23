#include "../lib/json/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace settings {
    json getSettings();
    json getOptions();
    string getFileContent(string filename);
    string getGlobalVars();
}

