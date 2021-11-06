#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <regex>
#include <vector>

#include "settings.h"
#include "auth/authbasic.h"
#include "api/os/os.h"

using namespace std;
using json = nlohmann::json;

namespace extensions {
    string __buildExtensionArgs() {
        string options = "";
        options += " --nl-port=" + to_string(settings::getOptionForCurrentMode("port").get<int>());
        options += " --nl-token=" + authbasic::getToken();
        return options;
    }

    void init() {
        json jExtensions = settings::getOptionForCurrentMode("extensions");
        if(jExtensions.is_null())
            return;
        vector<json> extensions = jExtensions.get<vector<json>>();
        for(json extension: extensions) {
            string commandKeyForOs = "command" + string(OS_NAME);

            if(!extension.contains("id") || (!extension.contains("command") &&
                !extension.contains(commandKeyForOs)) ) {
                continue;
            }

            string command = extension.contains(commandKeyForOs) ? extension[commandKeyForOs].get<string>()
                                : extension["command"].get<string>();
            command = regex_replace(command, regex("\\$\\{NL_PATH\\}"), settings::getAppPath());
            command += __buildExtensionArgs();

            os::execCommand(command, "", true); // async
        }
    }
} // namespace extensions
