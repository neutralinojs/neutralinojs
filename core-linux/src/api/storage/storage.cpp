// MIT License

// Copyright (c) 2018 Neutralinojs

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <iostream>
#include <fstream>
#include <sys/stat.h>
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
        output["content"] = buffer;
        output["success"] = true;
        return output.dump();
    }

    string putData(json input) {
        json output;
        string bucket = input["bucket"];
        string bucketPath = settings::joinAppPath(STORAGE_DIR);
        mkdir(bucketPath.c_str(), 0700);
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
