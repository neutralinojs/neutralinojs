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
    json __validateStorageBucket(string bucketName) {
        if(regex_match(bucketName, regex(STORAGE_BUCKET_REGEX)))
            return nullptr;
        json output;
        output["error"] = "Invalid storage bucket identifer";
        return output;
    }
    
    string getData(json input) {
        json output;
        string bucket = input["bucket"];
        json errorPayload = __validateStorageBucket(bucket);
        if(!errorPayload.is_null()) 
            return errorPayload.dump();
        string bucketPath = settings::joinAppPath(STORAGE_DIR);
        string filename = bucketPath + "/" + bucket + STORAGE_EXT;
        
        ifstream t;
        t.open(filename);
        if(!t.is_open()) {
            output["error"] = "Unable to open storage bucket: " + bucket;
            return output.dump();
        }
        string buffer = "";
        string line;
        while(!t.eof()){
            getline(t, line);
            buffer += line + "\n";
        }
        t.close();
        output["data"] = buffer;
        output["success"] = true;
        return output.dump();
    }

    string putData(json input) {
        json output;
        string bucket = input["bucket"];
        json errorPayload = __validateStorageBucket(bucket);
        if(!errorPayload.is_null()) 
            return errorPayload.dump();
        string bucketPath = settings::joinAppPath(STORAGE_DIR);
        
        json createDirParams;
        createDirParams["path"] = bucketPath;
        fs::createDirectory(createDirParams);
        
        string filename = bucketPath + "/" + bucket + STORAGE_EXT;
        if(input["data"].is_null()) {
            json readFileParams;
            readFileParams["fileName"] = filename;
            fs::removeFile(readFileParams);
        }
        else {
            string content = input["data"];
            ofstream t(filename);
            if(!t.is_open()) {
                output["error"] = "Unable to write storage bucket: " + bucket;
                return output.dump();
            }
            t << content;
            t.close();
        }
        output["success"] = true;
        return output.dump();
    }

}
