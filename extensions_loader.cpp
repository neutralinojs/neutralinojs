#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <regex>
#include <vector>

#include "settings.h"
#include "helpers.h"
#include "auth/authbasic.h"
#include "api/os/os.h"

using namespace std;
using json = nlohmann::json;

vector<string> loadedExtensions;

namespace extensions {
    string __buildExtensionArgs(const string &extensionId) {
        string options = "";
        options += " --nl-port=" + to_string(settings::getOptionForCurrentMode("port").get<int>());
        options += " --nl-token=" + authbasic::getToken();
        options += " --nl-extension-id=" + extensionId;
        return options;
    }

    void init() {
        json jExtensions = settings::getOptionForCurrentMode("extensions");
        if(jExtensions.is_null())
            return;
        vector<json> extensions = jExtensions.get<vector<json>>();
        for(const json &extension: extensions) {
            string commandKeyForOs = "command" + string(OS_NAME);

            if(!helpers::hasField(extension, "id")) {
                continue;
            }

            string extensionId = extension["id"].get<string>();

            if(helpers::hasField(extension, "command") || helpers::hasField(extension, commandKeyForOs)) {
                string command = helpers::hasField(extension, commandKeyForOs) ? extension[commandKeyForOs].get<string>()
                                    : extension["command"].get<string>();
                command = regex_replace(command, regex("\\$\\{NL_PATH\\}"), settings::getAppPath());
                command += __buildExtensionArgs(extensionId);

                os::execCommand(command, "", true); // async
            }

            loadedExtensions.push_back(extensionId);
        }
    }

    vector<string> getLoaded() {
        return loadedExtensions;
    }

    bool isLoaded(const string &extensionId) {
        return find(loadedExtensions.begin(), loadedExtensions.end(), extensionId)
                != loadedExtensions.end();
    }

} // namespace extensions
