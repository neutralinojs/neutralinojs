#include <iostream>
#include <fstream>
#include <regex>
#include "lib/json.hpp"
#include "settings.h"
#include "../filesystem/filesystem.h"

#define STORAGE_DIR "/.storage"
#define STORAGE_EXT ".neustorage"
#define STORAGE_BUCKET_REGEX "^[a-zA-Z-_0-9]{2,50}$"

using namespace std;
using json = nlohmann::json;

namespace storage {
namespace controllers {
    json __validateStorageBucket(string bucketName) {
        if(regex_match(bucketName, regex(STORAGE_BUCKET_REGEX)))
            return nullptr;
        json output;
        output["error"] = "Invalid storage bucket identifer";
        return output;
    }
    
    json getData(json input) {
        json output;
        string bucket = input["bucket"];
        json errorPayload = __validateStorageBucket(bucket);
        if(!errorPayload.is_null()) 
            return errorPayload;
        string bucketPath = settings::joinAppPath(STORAGE_DIR);
        string filename = bucketPath + "/" + bucket + STORAGE_EXT;
        
        fs::FileReaderResult fileReaderResult;
        fileReaderResult = fs::readFile(filename);
        ifstream t;
        t.open(filename);
        if(fileReaderResult.hasError) {
            output["error"] = "Unable to open storage bucket: " + bucket +
                                 ". " + fileReaderResult.error;
            return output;
        }
        output["data"] = fileReaderResult.data;
        output["success"] = true;
        return output;
    }

    json putData(json input) {
        json output;
        string bucket = input["bucket"];
        json errorPayload = __validateStorageBucket(bucket);
        if(!errorPayload.is_null()) 
            return errorPayload;
        string bucketPath = settings::joinAppPath(STORAGE_DIR);

        fs::createDirectory(bucketPath);
        
        string filename = bucketPath + "/" + bucket + STORAGE_EXT;
        if(input["data"].is_null()) {
            fs::removeFile(filename);
        }
        else {
            fs::FileWriterOptions fileWriterOptions;
            fileWriterOptions.data = input["data"];
            fileWriterOptions.filename = filename;
    
            if(!fs::writeFile(fileWriterOptions)) {
                output["error"] = "Unable to write storage bucket: " + bucket;
                return output;
            }
        }
        output["success"] = true;
        return output;
    }

} // namespace controllers
} // namespace storage
