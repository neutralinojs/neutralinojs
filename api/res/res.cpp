#include <iostream>
#include <string>
#include <vector>

#include "lib/json/json.hpp"
#include "settings.h"
#include "resources.h"
#include "helpers.h"
#include "errors.h"

#include "api/res/res.h"

using namespace std;
using json = nlohmann::json;

namespace res {
namespace controllers {

vector<string> __getFiles(const json &fileTree) {
    vector<string> files = {};
    std::function<void(const string &, const json &)> parseBundleHeader;
    parseBundleHeader = [&](const string &basePath, const json &data) {
        if (!helpers::hasField(data, "files")) {
            return;
        }

        for(auto &[childPath, childData]: data["files"].items()) {
            string path = basePath + "/" + childPath;
            files.push_back(path);
            parseBundleHeader(path, childData);
        }
    };

    parseBundleHeader("", fileTree);
    return files;
}

json getFiles(const json &input) {
    json output;
    if(!resources::isBundleMode()) {
        output["error"] = errors::makeErrorPayload(errors::NE_RS_APIRQRF);
        return output;
    }
    output["returnValue"] = __getFiles(resources::getFileTree());
    output["success"] = true;
    return output;
}

json extractFile(const json &input) {
    json output;
    if(!resources::isBundleMode()) {
        output["error"] = errors::makeErrorPayload(errors::NE_RS_APIRQRF);
        return output;
    }
    if(!helpers::hasRequiredFields(input, {"path", "destination"})) {
        output["error"] = errors::makeMissingArgErrorPayload();
        return output;
    }
    string path = input["path"].get<string>();
    string destination = input["destination"].get<string>();
    if(resources::extractFile(path, destination)) {
        output["success"] = true;
    }
    else {
        output["error"] = errors::makeErrorPayload(errors::NE_RS_TREEGER);
    }
    return output;
}

} // namespace controllers
} // namespace res
