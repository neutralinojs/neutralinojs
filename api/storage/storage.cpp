#include <string>
#include <iostream>
#include <fstream>
#include <regex>
#include <filesystem>

#include "lib/json/json.hpp"
#include "lib/platformfolders/platform_folders.h"

#include "settings.h"
#include "helpers.h"
#include "errors.h"
#include "api/fs/fs.h"

#if defined(_WIN32)
#include <windows.h>
#include <fileapi.h>
#endif

#define NEU_STORAGE_DIR "/.storage"
#define NEU_STORAGE_EXT ".neustorage"
#define NEU_STORAGE_EXT_REGEX ".*neustorage$"
#define NEU_STORAGE_KEY_REGEX "^[a-zA-Z-_0-9]{1,50}$"

using namespace std;
using json = nlohmann::json;

string storagePath;

namespace storage {

void init() {
    string storageLoc = "app";
    json jLoc = settings::getOptionForCurrentMode("storageLocation");
    if(!jLoc.is_null()) {
        storageLoc = jLoc.get<string>();
    }
    
    storagePath = storageLoc == "system" ? settings::joinSystemDataPath(NEU_STORAGE_DIR) : settings::joinAppPath(NEU_STORAGE_DIR);
}

namespace controllers {

json __validateStorageBucket(const string &key) {
    if(regex_match(key, regex(NEU_STORAGE_KEY_REGEX)))
        return nullptr;
    json output;
    output["error"] = errors::makeErrorPayload(errors::NE_ST_INVSTKY, string(NEU_STORAGE_KEY_REGEX));
    return output;
}

json __removeStorageBucket(const string &key) {
    json output;
    string filename = storagePath + "/" + key + NEU_STORAGE_EXT;
    if(!filesystem::remove(CONVSTR(filename))) {
        output["error"] = errors::makeErrorPayload(errors::NE_ST_STKEYRE, key);
        return output;
    }
    
    output["success"] = true;
    output["message"] = "Storage key " + key + " was removed";
    return output;
}

json getData(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"key"})) {
        output["error"] = errors::makeMissingArgErrorPayload("key");
        return output;
    }
    string key = input["key"].get<string>();
    json errorPayload = __validateStorageBucket(key);
    if(!errorPayload.is_null())
        return errorPayload;

    string filename = storagePath + "/" + key + NEU_STORAGE_EXT;

    fs::FileReaderResult fileReaderResult;
    fileReaderResult = fs::readFile(filename);
    if(fileReaderResult.status != errors::NE_ST_OK) {
        output["error"] = errors::makeErrorPayload(errors::NE_ST_NOSTKEX, key);
        return output;
    }
    output["returnValue"] = fileReaderResult.data;
    output["success"] = true;
    return output;
}

json setData(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"key"})) {
        output["error"] = errors::makeMissingArgErrorPayload("key");
        return output;
    }
    string key = input["key"].get<string>();
    json errorPayload = __validateStorageBucket(key);
    if(!errorPayload.is_null())
        return errorPayload;

    filesystem::create_directories(CONVSTR(storagePath));
    #if defined(_WIN32)
    SetFileAttributesA(storagePath.c_str(), FILE_ATTRIBUTE_HIDDEN);
    #endif

    if(!helpers::hasField(input, "data")) {
        return __removeStorageBucket(key);
    }
    else {
        fs::FileWriterOptions fileWriterOptions;
        fileWriterOptions.data = input["data"].get<string>();
        fileWriterOptions.filename = storagePath + "/" + key + NEU_STORAGE_EXT;;

        if(!fs::writeFile(fileWriterOptions)) {
            output["error"] = errors::makeErrorPayload(errors::NE_ST_STKEYWE, key);
            return output;
        }
    }
    output["success"] = true;
    return output;
}

json removeData(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"key"})) {
        output["error"] = errors::makeMissingArgErrorPayload("key");
        return output;
    }
    string key = input["key"].get<string>();
    json errorPayload = __validateStorageBucket(key);
    if(!errorPayload.is_null())
        return errorPayload;

    return __removeStorageBucket(key);
}

json getKeys(const json &input) {
    json output;
    output["returnValue"] = json::array();

    fs::DirReaderResult dirResult;
    dirResult = fs::readDirectory(storagePath);
    if(dirResult.status != errors::NE_ST_OK) {
        output["error"] = errors::makeErrorPayload(errors::NE_ST_NOSTDIR, storagePath);
        return output;
    }

    for(const fs::DirReaderEntry &entry: dirResult.entries) {
        if(entry.type == fs::EntryTypeFile && regex_match(entry.name, regex(NEU_STORAGE_EXT_REGEX))) {
            output["returnValue"].push_back(regex_replace(entry.name, regex(NEU_STORAGE_EXT), ""));
        }
    }
    output["success"] = true;
    return output;
}

json clear(const json &input) {
    json output;

    filesystem::remove_all(CONVSTR(storagePath));
    
    output["success"] = true;
    output["message"] = "Storage was cleared";
    return output;
}


} // namespace controllers

} // namespace storage
