#ifndef NEU_EXTENSIONS_H
#define NEU_EXTENSIONS_H

#include <string>
#include <vector>

#include "lib/json/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace extensions {

void init();
void loadOne(const string &extensionId);
vector<string> getLoaded();
bool isLoaded(const string &extensionId);
bool isInitialized();

} // namesapce extensions

#endif // #define NEU_EXTENSIONS_H

