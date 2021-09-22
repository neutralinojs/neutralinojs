#include <iostream>
#include <fstream>
#include <regex>
#include "lib/json.hpp"
#include "settings.h"
#include "../filesystem/filesystem.h"

#define STORAGE_DIR "/.storage"
#define STORAGE_EXT ".neustorage"
#define STORAGE_KEY_REGEX "^[a-zA-Z-_0-9]{1,50}$"

using namespace std;
using json = nlohmann::json;

namespace storage {
namespace controllers {
    json __validateStorageBucket(string key) {
        if(regex_match(key, regex(STORAGE_KEY_REGEX)))
            return nullptr;
        json output;
        output["error"] = "Invalid storage key";
        return output;
    }
    
    json getData(json input) {
        json output;
        string key = input["key"];
        json errorPayload = __validateStorageBucket(key);
        if(!errorPayload.is_null()) 
            return errorPayload;
        string bucketPath = settings::joinAppPath(STORAGE_DIR);
        string filename = bucketPath + "/" + key + STORAGE_EXT;
        
        fs::FileReaderResult fileReaderResult;
        fileReaderResult = fs::readFile(filename);
        if(fileReaderResult.hasError) {
            output["error"] = "Unable to find storage key: " + key +
                                 ". " + fileReaderResult.error;
            return output;
        }
        output["returnValue"] = fileReaderResult.data;
        output["success"] = true;
        return output;
    }

    json setData(json input) {
        json output;
        string key = input["key"];
        json errorPayload = __validateStorageBucket(key);
        if(!errorPayload.is_null()) 
            return errorPayload;
        string bucketPath = settings::joinAppPath(STORAGE_DIR);

        fs::createDirectory(bucketPath);
        
        string filename = bucketPath + "/" + key + STORAGE_EXT;
        if(input["data"].is_null()) {
            fs::removeFile(filename);
        }
        else {
            fs::FileWriterOptions fileWriterOptions;
            fileWriterOptions.data = input["data"];
            fileWriterOptions.filename = filename;
    
            if(!fs::writeFile(fileWriterOptions)) {
                output["error"] = "Unable to write data to key: " + key;
                return output;
            }
        }
        output["success"] = true;
        return output;
    }

} // namespace controllers
} // namespace storage
