#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

#include "lib/json/json.hpp"
#include "settings.h"
#include "resources.h"
#include "helpers.h"
#include "errors.h"

#include "api/res/res.h"
#include "api/fs/fs.h"

#include "lib/base64/base64.hpp"

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

string __getResourcesDirectory() {
    json options = settings::getConfig();
    json jResourcesPath = options["cli"]["resourcesPath"];
    string resourcesPath = "";
    
    if(!jResourcesPath.is_null()) {
        resourcesPath = jResourcesPath.get<string>();
    }

    return settings::joinAppPath(resourcesPath);
}

string __convertPath(const string &path) {
    string resPath = FS_CONVWSTR(filesystem::relative(CONVSTR(path), 
                        CONVSTR(settings::getAppPath())));
    return "/" + helpers::normalizePath(resPath);
};

json getFiles(const json &input) {
    json output;
    json files = json::array();
    if(resources::isBundleMode()) {
        files = __getFiles(resources::getFileTree());
    }
    else {
        string resourcesPath = __getResourcesDirectory(); 
        
        files.push_back(settings::getConfigFile());
        files.push_back(__convertPath(resourcesPath));
        
        fs::DirReaderResult dirResult = fs::readDirectory(resourcesPath, true);
        for(const fs::DirReaderEntry &entry: dirResult.entries) {
            files.push_back(__convertPath(entry.path));
        }
    }
    
    output["returnValue"] = files;
    output["success"] = true;
    return output;
}

json extractFile(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"path", "destination"})) {
        output["error"] = errors::makeMissingArgErrorPayload();
        return output;
    }
    string path = input["path"].get<string>();
    string destination = input["destination"].get<string>();
    
    if(resources::isBundleMode() && resources::extractFile(path, destination)) {
        output["success"] = true;
    }
    else if(!resources::isBundleMode()) {
        path = settings::joinAppPath(path);
        
        error_code ec;
        auto copyOptions = filesystem::copy_options::recursive |
                        filesystem::copy_options::overwrite_existing;

        filesystem::copy(CONVSTR(path), CONVSTR(destination), copyOptions, ec);
        if(!ec) {
            output["success"] = true;
        }
        else {
            output["error"] = errors::makeErrorPayload(errors::NE_RS_FILEXTF, destination);
        }
    }
    else {
        output["error"] = errors::makeErrorPayload(errors::NE_RS_FILEXTF, destination);
    }
    return output;
}

json readFile(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"path"})) {
        output["error"] = errors::makeMissingArgErrorPayload();
        return output;
    }
    string path = input["path"].get<string>();
    fs::FileReaderResult fileReaderResult;
    fileReaderResult = resources::getFile(path);
    if(fileReaderResult.status != errors::NE_ST_OK) {
        output["error"] = errors::makeErrorPayload(fileReaderResult.status, path);
    }
    else {
        output["returnValue"] = fileReaderResult.data;
        output["success"] = true;
    }
    return output;
}

json readBinaryFile(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"path"})) {
        output["error"] = errors::makeMissingArgErrorPayload();
        return output;
    }
    string path = input["path"].get<string>();
    fs::FileReaderResult fileReaderResult;
    fileReaderResult = resources::getFile(path);
    if(fileReaderResult.status != errors::NE_ST_OK) {
        output["error"] = errors::makeErrorPayload(fileReaderResult.status, path);
    }
    else {
        output["returnValue"] = base64::to_base64(fileReaderResult.data);
        output["success"] = true;
    }
    return output;
}

} // namespace controllers
} // namespace res
