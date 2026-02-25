#ifndef NEU_EXTENSIONS_H
#define NEU_EXTENSIONS_H

#include <string>
#include <vector>

#include "lib/json/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace extensions {

struct LoadedExtension {
    string id;
    int virtualPid;
};

void init();
void loadOne(const string &extensionId, int virtualPid = -1);
vector<string> getLoaded();
vector<LoadedExtension> getLoadedExtensions();
bool isLoaded(const string &extensionId);
bool isInitialized();
void cleanup();

} // namesapce extensions

#endif // #define NEU_EXTENSIONS_H

