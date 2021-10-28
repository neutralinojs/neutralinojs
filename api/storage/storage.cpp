#include <string>
#include <iostream>
#include <fstream>
#include <regex>

#include "lib/json/json.hpp"
#include "settings.h"
#include "helpers.h"
#include "api/filesystem/filesystem.h"

#define STORAGE_DIR "/.storage"
#define STORAGE_EXT ".neustorage"
#define STORAGE_KEY_REGEX "^[a-zA-Z-_0-9]{1,50}$"

using namespace std;
using json = nlohmann::json;

namespace storage {
namespace controllers {
    json __validateStorageBucket(const string &key) {
        if(regex_match(key, regex(STORAGE_KEY_REGEX)))
            return nullptr;
        json output;
        output["error"] = helpers::makeErrorPayload("NE_ST_INVSTKY", 
                            "Invalid storage key format. The key should match regex: ^[a-zA-Z-_0-9]{1,50}$");
        return output;
    }
    
    json getData(const json &input) {
        json output;
        if(!input.contains("key")) {
            output["error"] = helpers::makeMissingArgErrorPayload();
            return output;
        }
        string key = input["key"].get<string>();
        json errorPayload = __validateStorageBucket(key);
        if(!errorPayload.is_null()) 
            return errorPayload;
        string bucketPath = settings::joinAppPath(STORAGE_DIR);
        string filename = bucketPath + "/" + key + STORAGE_EXT;
        
        fs::FileReaderResult fileReaderResult;
        fileReaderResult = fs::readFile(filename);
        if(fileReaderResult.hasError) {
            output["error"] = helpers::makeErrorPayload("NE_ST_NOSTKEX", "Unable to find storage key: " + key +
                                 ". " + fileReaderResult.error);
            return output;
        }
        output["returnValue"] = fileReaderResult.data;
        output["success"] = true;
        return output;
    }

    json setData(const json &input) {
        json output;
        if(!input.contains("key")) {
            output["error"] = helpers::makeMissingArgErrorPayload();
            return output;
        }
        string key = input["key"].get<string>();
        json errorPayload = __validateStorageBucket(key);
        if(!errorPayload.is_null()) 
            return errorPayload;
        string bucketPath = settings::joinAppPath(STORAGE_DIR);

        fs::createDirectory(bucketPath);
        
        string filename = bucketPath + "/" + key + STORAGE_EXT;
        if(!input.contains("data")) {
            fs::removeFile(filename);
        }
        else {
            fs::FileWriterOptions fileWriterOptions;
            fileWriterOptions.data = input["data"].get<string>();
            fileWriterOptions.filename = filename;
    
            if(!fs::writeFile(fileWriterOptions)) {
                output["error"] = helpers::makeErrorPayload("NE_ST_STKEYWE", 
                                    "Unable to write data to key: " + key);
                return output;
            }
        }
        output["success"] = true;
        return output;
    }

} // namespace controllers
} // namespace storage
