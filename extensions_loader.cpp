#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>

#include "extensions_loader.h"
#include "settings.h"
#include "helpers.h"
#include "auth/authbasic.h"
#include "api/os/os.h"
#include "api/fs/fs.h"

using namespace std;
using json = nlohmann::json;

namespace extensions {

vector<string> loadedExtensions;
bool initialized = false;

json __buildExtensionProcessInput(const string &extensionId) {
    json options = {
        {"nlPort", to_string(settings::getOptionForCurrentMode("port").get<int>())},
        {"nlToken", authbasic::getTokenInternal()},
        {"nlConnectToken", authbasic::getConnectTokenInternal()},
        {"nlExtensionId", extensionId}
    };
    return options;
}

void init() {
    json jExtensions = settings::getOptionForCurrentMode("extensions");
    if(jExtensions.is_null())
        return;
    vector<json> extensions = jExtensions.get<vector<json>>();
    for(const json &extension: extensions) {
        string commandKeyForOs = "command" + string(NEU_OS_NAME);

        if(!helpers::hasField(extension, "id")) {
            continue;
        }

        string extensionId = extension["id"].get<string>();

        if(helpers::hasField(extension, "command") || helpers::hasField(extension, commandKeyForOs)) {
            string command = helpers::hasField(extension, commandKeyForOs) ? extension[commandKeyForOs].get<string>()
                                : extension["command"].get<string>();
            command = fs::applyPathConstants(command);

            os::ChildProcessOptions processOptions;
            processOptions.events = false;
            processOptions.stdOutHandler = [=](const char *bytes, size_t n){
                cout << string(bytes, n) << flush;
            };
            processOptions.stdErrHandler = [=](const char *bytes, size_t n){
                cerr << string(bytes, n) << flush;
            };
            
            auto process = os::spawnProcess(command, processOptions);
            os::updateSpawnedProcess({process.first, "stdIn", helpers::jsonToString(__buildExtensionProcessInput(extensionId))});
            os::updateSpawnedProcess({process.first, "stdInEnd"});
            
        }

        extensions::loadOne(extensionId);
    }
    initialized = true;
}

void loadOne(const string &extensionId) {
    loadedExtensions.push_back(extensionId);
}

vector<string> getLoaded() {
    return loadedExtensions;
}

bool isLoaded(const string &extensionId) {
    return find(loadedExtensions.begin(), loadedExtensions.end(), extensionId)
            != loadedExtensions.end();
}

bool isInitialized() {
    return initialized;
}

} // namespace extensions
