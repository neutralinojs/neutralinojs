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

vector<string> __getAllFiles() {
    vector<string> files = {};
    std::function<void(const string &, const json &)> parseBundleHeader;
    parseBundleHeader = [&](const string &basePath, const json &data) {
        if(!helpers::hasField(data, "files")) {
            return;
        }

        for(auto &[childPath, childData]: data["files"].items()) {
            string path = basePath + "/" + childPath;
            files.push_back(path);  
            parseBundleHeader(path, childData);
        }
    };

    parseBundleHeader("", resources::getFileTree());
    return files;
}

vector<string> __getFiles(const string &rootPath) {
    vector<string> files = {};
    std::function<void(const string &, const json &)> parseBundleHeader;
    parseBundleHeader = [&](const string &path, const json &data) {    
        if(!helpers::hasField(data, "files")) {
            if(path.rfind(rootPath, 0) != string::npos) {
                files.push_back(path);
            }
            return;
        }
        else if(rootPath.rfind(path, 0) == string::npos && path.rfind(rootPath, 0) == string::npos) {
            return;
        }

        for(auto &[childPath, childData]: data["files"].items()) {
            parseBundleHeader(path + "/" + childPath, childData);
        }
    };

    parseBundleHeader("", resources::getFileTree());
    return files;
}

pair<unsigned long, bool> __getStats(const string &path) {
    vector<string> pathSegments = helpers::split(path, '/');
    json json = resources::getFileTree();
    for(const auto &pathSegment: pathSegments) {
        if(pathSegment.size() == 0) {
            continue;
        }
        if(json.is_null()) {
            break;
        }
        json = json["files"][pathSegment];
    }
    if(!json.is_null()) {
        bool isFile = json["files"].is_null();
        return make_pair(
            isFile ? json["size"].get<unsigned long>() : 4096, isFile);
    }
    return make_pair(-1, false);
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
    if(resources::isBundleMode() || resources::isEmbeddedMode()) {
        files = __getAllFiles();
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

json getStats(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"path"})) {
        output["error"] = errors::makeMissingArgErrorPayload("path");
        return output;
    }
    string path = input["path"].get<string>();
    if(resources::isBundleMode() || resources::isEmbeddedMode()) {
        auto resStats = __getStats(path);
        if(resStats.first != -1) {
            json stats;
            stats["size"] = resStats.first;
            stats["isFile"] = resStats.second;
            stats["isDirectory"] = !resStats.second;
            
            output["returnValue"] = stats;
            output["success"] = true;
        }
        else {
            output["error"] = errors::makeErrorPayload(errors::NE_RS_NOPATHE, path);
        }
    }
    else {
        path = settings::joinAppPath(path);
        fs::FileStats fileStats = fs::getStats(path);
        if(fileStats.status == errors::NE_ST_OK) {
            json stats;
            stats["size"] = fileStats.size;
            stats["isFile"] = fileStats.entryType == fs::EntryTypeFile;
            stats["isDirectory"] = fileStats.entryType == fs::EntryTypeDir;
            
            output["returnValue"] = stats;
            output["success"] = true;
        }
        else {
            output["error"] = errors::makeErrorPayload(fileStats.status, path);
        }
    }
    return output;
}

json extractFile(const json &input) {
    json output;
    const auto missingRequiredField = helpers::missingRequiredField(input, {"path", "destination"});
    if(missingRequiredField) {
        output["error"] = errors::makeMissingArgErrorPayload(missingRequiredField.value());
        return output;
    }
    string path = input["path"].get<string>();
    string destination = input["destination"].get<string>();
    
    if(resources::isBundleMode() && resources::extractFile(path, destination)) {
        output["success"] = true;
    }
    else if(resources::isEmbeddedMode() && resources::extractFile(path, destination)) {
        output["success"] = true;
    }
    else if(resources::isDirMode()) {
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


json extractDirectory(const json &input) {
    json output;
    const auto missingRequiredField = helpers::missingRequiredField(input, {"path", "destination"});
    if(missingRequiredField) {
        output["error"] = errors::makeMissingArgErrorPayload(missingRequiredField.value());
        return output;
    }
    string path = input["path"].get<string>();
    string destination = input["destination"].get<string>();

    if(resources::isBundleMode() || resources::isEmbeddedMode()) {
        auto resPaths = __getFiles(path);
        for(const auto &resPath: resPaths) {
            string extractPath = resPath;
            extractPath = helpers::normalizePath(extractPath)
                                .substr(path == "/" ? 1 : path.size() + 1);
            resources::extractFile(resPath, FS_CONVWSTR(filesystem::path(CONVSTR(destination)) / 
                filesystem::path(CONVSTR(extractPath))));
        }
        output["success"] = true;
    }
    else if(resources::isDirMode()) {
        path = settings::joinAppPath(path);
        
        error_code ec;
        auto copyOptions = filesystem::copy_options::recursive |
                        filesystem::copy_options::overwrite_existing;

        filesystem::copy(CONVSTR(path), CONVSTR(destination), copyOptions, ec);
        if(!ec) {
            output["success"] = true;
        }
        else {
            output["error"] = errors::makeErrorPayload(errors::NE_RS_DIREXTF, destination);
        }
    }
    else {
        output["error"] = errors::makeErrorPayload(errors::NE_RS_DIREXTF, destination);
    }
    return output;
}

json readFile(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"path"})) {
        output["error"] = errors::makeMissingArgErrorPayload("path");
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
        output["error"] = errors::makeMissingArgErrorPayload("path");
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
