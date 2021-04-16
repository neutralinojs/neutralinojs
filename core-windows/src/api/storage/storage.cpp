#include <iostream>
#include <fstream>
#include <windows.h>
#include "lib/json.hpp"
#include "settings.h"
#include "../filesystem/filesystem.h"

#define STORAGE_DIR "/.storage"
#define STORAGE_EXT ".neustorage"

using namespace std;
using json = nlohmann::json;

namespace storage {
    string getData(json input) {
        json output;
        string bucket = input["bucket"];
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
        string bucketPath = settings::joinAppPath(STORAGE_DIR);
        CreateDirectory(bucketPath.c_str(), NULL);
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
        output["message"] = "Wrote data to " + bucket;
        return output.dump();
    }

}
