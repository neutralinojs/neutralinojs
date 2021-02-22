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
#include "../../../lib/json/json.hpp"
#include "../../settings.h"

using namespace std;
using json = nlohmann::json;

namespace storage {
    string getData(string jso) {
        json input;
        json output;
        try {
            input = json::parse(jso);
        }
        catch(exception e){
            output["error"] = "JSON parse error is occurred!";
            return output.dump();
        }
        string bucket = input["bucket"].get<std::string>();
        string bucketPath = settings::joinAppPath("storage");
        string filename = bucketPath + "/" + bucket + ".json";
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
        return output.dump();
    }

    string putData(string jso) {
        json input;
        json output;
        try {
            input = json::parse(jso);
        }
        catch(exception e){
            output["error"] = "JSON parse error is occurred!";
            return output.dump();
        }

        string bucket = input["bucket"].get<std::string>();
        string bucketPath = settings::joinAppPath("storage");
        mkdir(bucketPath.c_str(), 0700);
        string filename = bucketPath + "/" + bucket + ".json";
        string content = input["content"].dump();
        ofstream t(filename);
        if(!t.is_open()) {
            output["error"] = "Unable to write storage bucket: " + bucket;
            return output.dump();
        }
        t << content;
        t.close();

        return output.dump();
    }

}
