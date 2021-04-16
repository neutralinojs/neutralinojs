#include "lib/json.hpp"

using namespace std;
using json = nlohmann::json;
extern bool loadResFromDir;

namespace settings {
    json getConfig();
    string getFileContent(string filename);
    string getGlobalVars();
    string getCurrentDir();
    void setGlobalArgs(json args);
    string joinAppPath(string filename);
    string getMode();
    void setPort(int port);
}

